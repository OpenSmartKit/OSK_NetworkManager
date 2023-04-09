#ifndef NETWORK_MANAGER_h
#define NETWORK_MANAGER_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <functional>
#include <Arduino.h>
#include <WiFi.h>
#include <NetworkManagerSettings.h>
#include <Debug.h>


class NetworkManager
{
public:
    static NetworkManager *getInstance();
    ~NetworkManager();
    void init(NetworkManagerSettings *settings);
    void connect();
    void disconnect();

private:
    static NetworkManager *_instance;
    NetworkManagerSettings *_settings = nullptr;
    NetworkManager();
    void _smartConfigBegin();
    void _smartConfigCheck();
    void _WiFiStationConnected();
    static void _WiFiEvents(WiFiEvent_t event);
    bool _isSmartConfigStarted = false;
    TimerHandle_t *_tSmartConfig = nullptr;
};

#endif