#ifndef NETWORK_MANAGER_h
#define NETWORK_MANAGER_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <functional>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <NetworkManagerSettings.h>
#include <Debug.h>

#ifndef STATUS_BLINK_WIFI_CONNECTED
#define STATUS_BLINK_WIFI_CONNECTED 800
#endif
#ifndef STATUS_BLINK_WIFI_DISCONNECTED
#define STATUS_BLINK_WIFI_DISCONNECTED 200
#endif
#ifndef STATUS_BLINK_WIFI_DISABLED
#define STATUS_BLINK_WIFI_DISABLED 3000
#endif

class NetworkManager
{
public:
    static NetworkManager *getInstance();
    ~NetworkManager();
    void begin(NetworkManagerSettings *settings, bool useSmartConfig = false);
    void disconnect();
    void beginOTA(std::string hostname = "");
    void useStatusLedForWiFi(uint8_t statusPin = 2);
    uint8_t _statusPin;

private:
    static NetworkManager *_instance;
    NetworkManagerSettings *_settings = nullptr;
    NetworkManager();
    void _connect();
    void _smartConfigBegin();
    void _smartConfigCheck();
    void _WiFiStationConnected();
    static void _handleOTA(TimerHandle_t handle);
    static void _blinkStatusLed(TimerHandle_t handle);
    static void _WiFiEvents(WiFiEvent_t event);
    bool _isSmartConfigStarted = false;
    bool _useSmartConfig = false;
    TimerHandle_t _tOTA = nullptr;
    TimerHandle_t _tStatus = nullptr;
};

#endif