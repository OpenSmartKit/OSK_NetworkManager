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
    _settings = settings;
    _isSmartConfigStarted = false;

    WiFi.setAutoReconnect(true);
    WiFi.onEvent(_WiFiEvents);
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

void NetworkManager::connect()
{
    WiFi.mode(WIFI_STA);
    
    if (strcmp(_settings->getWifiName(), "") != 0) {
        DBG("Connect to WiFi...");
        WiFi.begin(_settings->getWifiName(), _settings->getWifiPassword());
    } else {
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

void NetworkManager::_WiFiStationConnected()
{
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
