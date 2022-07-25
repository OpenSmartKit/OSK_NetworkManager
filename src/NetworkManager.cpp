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

    NimBLEService *pOSKService = bleServer->createService("A001");

    // Get WiFi List Characteristic
    NimBLECharacteristic *pWiFiListCharacteristic = pOSKService->createCharacteristic(
        "B001",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pWiFiListCharacteristic->setCallbacks(_cbGetWifiList);

    // Set WiFi Name
    NimBLECharacteristic *pWiFiNameCharacteristic = pOSKService->createCharacteristic(
        "B002",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    std::string wiFiName(_settings->getWifiName());
    pWiFiNameCharacteristic->setValue(wiFiName);
    pWiFiNameCharacteristic->setCallbacks(_cbSaveWifiName);

    // Set WiFi Pass
    NimBLECharacteristic *pWiFiPassCharacteristic = pOSKService->createCharacteristic(
        "B003",
        NIMBLE_PROPERTY::WRITE);
    pWiFiPassCharacteristic->setCallbacks(_cbSaveWifiPassword);

    // Set Device Name
    NimBLECharacteristic *pDeviceNameCharacteristic = pOSKService->createCharacteristic(
        "B004",
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
    std::string deviceName(_settings->getModuleName());
    pDeviceNameCharacteristic->setValue(deviceName);
    pDeviceNameCharacteristic->setCallbacks(_cbSaveDeviceName);

    // Get Device IP
    NimBLECharacteristic *pIPCharacteristic = pOSKService->createCharacteristic(
        "B005",
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pOSKService->start();

    /** Add the services to the advertisement **/
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pOSKService->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    Serial.println("Advertising Started");
}

void NetworkManager::connect()
{
    WiFi.mode(WIFI_STA);
    
    if (strcmp(_settings->getWifiName(), "") != 0) {
        Serial.println("Connect to WiFi...");
        WiFi.begin(_settings->getWifiName(), _settings->getWifiPassword());
    }
}

void NetworkManager::disconnect()
{
    WiFi.disconnect();
    Serial.println("WiFi Disconnected.");
}

void NetworkManager::taskScanWifi(void *pvParameters)
{
    NetworkManager *self = (NetworkManager *)pvParameters;

    for (;;)
    {
        xEventGroupWaitBits(self->_eg, EVENT_SCAN_WIFI, pdTRUE, pdTRUE, portMAX_DELAY);

        WiFi.mode(WIFI_STA);

        Serial.println("Start WiFi scanning...");
        int n = WiFi.scanNetworks();
        Serial.print("Found WiFis: ");
        Serial.println(n);

        NimBLEService *pSvc = self->bleServer->getServiceByUUID("A001");
        NimBLECharacteristic *pChr = pSvc->getCharacteristic("B001");

        for (int i = 0; i < n; ++i)
        {
            std::string wifiName(WiFi.SSID(i).c_str());
            Serial.println(wifiName.c_str());
            pChr->setValue(wifiName);
            pChr->notify();
            delay(10);
        }

        pChr->setValue('+');
        pChr->notify();

        Serial.println("Done WiFi scanning.");
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

        NimBLEService *pSvc = self->bleServer->getServiceByUUID("A001");
        NimBLECharacteristic *pChr = pSvc->getCharacteristic("B005");

        if (WiFi.isConnected())
        {
            std::string ip(WiFi.localIP().toString().c_str());
            pChr->setValue(ip);
            Serial.println("Connect to WiFi successfully.");
            Serial.print("IP: ");
            Serial.println(ip.c_str());
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
