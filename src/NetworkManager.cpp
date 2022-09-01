#include <NetworkManager.h>

NetworkManager *NetworkManager::_instance = nullptr;

NetworkManager *NetworkManager::getInstance()
{
    if (!_instance)
        _instance = new NetworkManager();

    return _instance;
}

void NetworkManager::init(NetworkManagerSettings *settings)
{
    _eg = xEventGroupCreate();
    _settings = settings;

    _cbSaveDeviceName = new SaveDeviceNameCallbacks(_settings);
    _cbGetWifiList = new GetWifiListCallbacks(_settings, _eg);
    _cbSaveWifiName = new SaveWifiNameCallbacks(_settings);
    _cbSaveWifiPassword = new SaveWifiPasswordCallbacks(_settings, this);

    xTaskCreatePinnedToCore(this->taskScanWifi, "sw", INTERNAL_TASK_STACK, this, INTERNAL_TASK_PRIORITY, &_thScanWifi, INTERNAL_TASK_CORE);
    xTaskCreatePinnedToCore(this->taskNotifyIpChange, "ip", INTERNAL_TASK_STACK, this, INTERNAL_TASK_PRIORITY, &_thScanWifi, INTERNAL_TASK_CORE);

    WiFi.onEvent(WiFiEvent);

    initBLE();
    xEventGroupSetBits(_eg, EVENT_IP_CHANGE);
}

NetworkManager::NetworkManager()
{
}

void NetworkManager::initBLE()
{
    char buffer[16];
    sprintf(buffer, "OSK_%02X", ESP.getEfuseMac());
    NimBLEDevice::init(buffer);

    bleServer = NimBLEDevice::createServer();

    NimBLEService *pOSKService = bleServer->createService(BLE_SERVICE_MAIN_ADDR);

    // Get WiFi List Characteristic
    NimBLECharacteristic *pWiFiListCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_SCAN_WIFI_ADDR,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pWiFiListCharacteristic->setCallbacks(_cbGetWifiList);

    // Set WiFi Name
    NimBLECharacteristic *pWiFiNameCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_WIFI_NAME_ADDR,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    std::string wiFiName(_settings->getWifiName());
    pWiFiNameCharacteristic->setValue(wiFiName);
    pWiFiNameCharacteristic->setCallbacks(_cbSaveWifiName);

    // Set WiFi Pass
    NimBLECharacteristic *pWiFiPassCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_WIFI_PASS_ADDR,
        NIMBLE_PROPERTY::WRITE);
    pWiFiPassCharacteristic->setCallbacks(_cbSaveWifiPassword);

    // Set Device Name
    NimBLECharacteristic *pDeviceNameCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_NAME_ADDR,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    std::string deviceName(_settings->getModuleName());
    pDeviceNameCharacteristic->setValue(deviceName);
    pDeviceNameCharacteristic->setCallbacks(_cbSaveDeviceName);

    // Get Device IP
    NimBLECharacteristic *pIPCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_IP_ADDR,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    // Debug info
    NimBLECharacteristic *pDebugCharacteristic = pOSKService->createCharacteristic(
        BLE_CHAR_DEBUG_ADDR,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pOSKService->start();

    /** Add the services to the advertisement **/
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pOSKService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    DEBUG_MSG("Advertising Started");
}

void NetworkManager::connect()
{
    WiFi.mode(WIFI_STA);
    
    if (strcmp(_settings->getWifiName(), "") != 0) {
        DEBUG_MSG("Connect to WiFi...");
        WiFi.begin(_settings->getWifiName(), _settings->getWifiPassword());
    }
}

void NetworkManager::disconnect()
{
    WiFi.disconnect();
    DEBUG_MSG("WiFi Disconnected.");
}

void NetworkManager::taskScanWifi(void *pvParameters)
{
    NetworkManager *self = (NetworkManager *)pvParameters;

    for (;;)
    {
        xEventGroupWaitBits(self->_eg, EVENT_SCAN_WIFI, pdTRUE, pdTRUE, portMAX_DELAY);

        WiFi.mode(WIFI_STA);

        DEBUG_MSG("Start WiFi scanning...");
        
        int n = WiFi.scanNetworks();

        DEBUG_MSG("Found WiFis: ");

        NimBLEService *pSvc = self->bleServer->getServiceByUUID(BLE_SERVICE_MAIN_ADDR);
        NimBLECharacteristic *pChr = pSvc->getCharacteristic(BLE_CHAR_SCAN_WIFI_ADDR);

        for (int i = 0; i < n; ++i)
        {
            std::string wifiName(WiFi.SSID(i).c_str());
            DEBUG_MSG(wifiName.c_str());
            pChr->setValue(wifiName);
            pChr->notify();
            delay(10);
        }

        pChr->setValue('+');
        pChr->notify();

        DEBUG_MSG("Done WiFi scanning.");
    }
}

void NetworkManager::WiFiEvent(WiFiEvent_t event)
{
    NetworkManager *self = NetworkManager::getInstance();

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        xEventGroupSetBits(self->_eg, EVENT_IP_CHANGE);
        break;
    default:
        break;
    }
}

void NetworkManager::taskNotifyIpChange(void *pvParameters)
{
    NetworkManager *self = (NetworkManager *)pvParameters;

    for (;;)
    {
        xEventGroupWaitBits(self->_eg, EVENT_IP_CHANGE, pdTRUE, pdTRUE, portMAX_DELAY);

        NimBLEService *pSvc = self->bleServer->getServiceByUUID(BLE_SERVICE_MAIN_ADDR);
        NimBLECharacteristic *pChr = pSvc->getCharacteristic(BLE_CHAR_IP_ADDR);

        if (WiFi.isConnected())
        {
            std::string ip(WiFi.localIP().toString().c_str());
            pChr->setValue(ip);
            DEBUG_MSG("Connect to WiFi successfully.");
            DEBUG_MSG("IP: ");
            DEBUG_MSG(ip.c_str());
            
            #if DEBUG_USE_TELNET
                telnet.begin(23);
                DEBUG_MSG(WiFi.localIP().toString());
                DEBUG_MSG("Telnet server started.");
            #endif
        }
        else
        {
            pChr->setValue("");
        }

        pChr->notify();
    }
}

void NetworkManager::notifyWifiPaswordReady()
{
    if (strlen(_settings->getWifiName()) == 0) {
        disconnect();
    } else {
        connect();
    }
}

void NetworkManager::debug(const std::string &value)
{
    NimBLEService *pSvc = bleServer->getServiceByUUID(BLE_SERVICE_MAIN_ADDR);
    NimBLECharacteristic *pChr = pSvc->getCharacteristic(BLE_CHAR_DEBUG_ADDR);

    pChr->setValue(value);
    pChr->notify();

    DEBUG_MSG(value.c_str());
}
