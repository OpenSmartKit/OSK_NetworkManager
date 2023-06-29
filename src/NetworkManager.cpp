#include <NetworkManager.h>

NetworkManager *NetworkManager::_instance = nullptr;

NetworkManager *NetworkManager::getInstance()
{
    if (!_instance)
        _instance = new NetworkManager();

    return _instance;
}

void NetworkManager::begin(NetworkManagerSettings *settings, bool useSmartConfig)
{
    _settings = settings;
    _isSmartConfigStarted = false;
    _useSmartConfig = _useSmartConfig;

    WiFi.onEvent(_WiFiEvents);

    _connect();
}

NetworkManager::NetworkManager()
{
    DBG("Chip Revision: %d", ESP.getChipRevision());

    char buffer[12];
    uint64_t chipid = ESP.getEfuseMac();
    DBG("ESP32 Chip ID: %04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
    DBG("Flash chip size: %lu", ESP.getFlashChipSize());
    DBG("Flash chip frequency: %lu", ESP.getFlashChipSpeed());
    DBG("SDK Version: %s", ESP.getSdkVersion());
}

void NetworkManager::_connect()
{
    WiFi.mode(WIFI_STA);
    
    if (strcmp(_settings->getWifiName(), "") != 0) {
        DBG("Connect to WiFi...");
        WiFi.begin(_settings->getWifiName(), _settings->getWifiPassword());
    } else if (_useSmartConfig) {
        _smartConfigBegin();
    }
}

void NetworkManager::disconnect()
{
#if OSK_DEBUG_USE_TELNET
    telnet.stop();
#endif
    WiFi.disconnect();
    DBG("WiFi Disconnected.");
}

void NetworkManager::_handleOTA(TimerHandle_t handle)
{
    ArduinoOTA.handle();
}

void NetworkManager::_blinkStatusLed(TimerHandle_t handle)
{
    IO *io = IO::getInstance();
    io->set(OSK_STATUS_LED, !io->get(OSK_STATUS_LED));
}

void NetworkManager::beginOTA(std::string hostname)
{
    if (hostname.length() > 0) {
        ArduinoOTA.setHostname(hostname.c_str());
    }
    ArduinoOTA.begin();

    _tOTA = xTimerCreate("ota", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(_handleOTA));
	xTimerStart(_tOTA, 0);
}

void NetworkManager::useStatusLedForWiFi()
{
    IO *io = IO::getInstance();
	io->mode(OSK_STATUS_LED, OUTPUT);
	io->set(OSK_STATUS_LED, LOW);

    _tStatus = xTimerCreate("statusBlink", pdMS_TO_TICKS(STATUS_BLINK_WIFI_DISABLED), pdTRUE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(_blinkStatusLed));
	xTimerStart(_tStatus, 0);
}

void NetworkManager::_WiFiStationConnected()
{
    if (_tStatus) {
        xTimerChangePeriod(_tStatus, pdMS_TO_TICKS(STATUS_BLINK_WIFI_CONNECTED), portMAX_DELAY);
    }

    if (_isSmartConfigStarted && WiFi.status() == WL_CONNECTED) {
        char buffer[32];
        
		strlcpy(buffer, WiFi.SSID().c_str(), sizeof(buffer));
        DBG("Save WfFi: %s", buffer);
        _settings->setWifiName(buffer);

		strlcpy(buffer, WiFi.psk().c_str(), sizeof(buffer));
        DBG("Save PWD: %s", buffer);
        _settings->setWifiPassword(buffer);

        _isSmartConfigStarted = false;
    }

    DBG("WiFi is connected!");
    DBG("IP: %s", WiFi.localIP().toString().c_str());

    WiFi.setAutoReconnect(true);
}

void NetworkManager::_WiFiEvents(WiFiEvent_t event)
{
    NetworkManager *self = NetworkManager::getInstance();

    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        self->_WiFiStationConnected();
        break;
    default:
        if (self->_tStatus) {
            xTimerChangePeriod(self->_tStatus, pdMS_TO_TICKS(STATUS_BLINK_WIFI_DISCONNECTED), portMAX_DELAY);
        }
        break;
    }
}

void NetworkManager::_smartConfigBegin()
{
    DBG("Start SmartConfig.");
    //Init WiFi as Station
    WiFi.mode(WIFI_AP_STA);

    //Start SmartConfig
    WiFi.beginSmartConfig();
    _isSmartConfigStarted = true;
}
