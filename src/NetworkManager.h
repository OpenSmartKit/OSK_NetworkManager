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
#include <NimBLEDevice.h>
#include <NetworkManagerSettings.h>
#include <callbacks/SaveDeviceNameCallbacks.hpp>
#include <callbacks/SaveWifiNameCallbacks.hpp>
#include <callbacks/GetWifiListCallbacks.hpp>
#include <callbacks/SaveWifiPasswordCallbacks.hpp>
#include <callbacks/SaveWiFiPasswordNotifier.h>

#define INTERNAL_TASK_STACK 10000
#define INTERNAL_TASK_PRIORITY 3
#define INTERNAL_TASK_CORE 1

#define BLE_SERVICE_MAIN_ADDR "A001"
#define BLE_CHAR_SCAN_WIFI_ADDR "B001"
#define BLE_CHAR_WIFI_NAME_ADDR "B002"
#define BLE_CHAR_WIFI_PASS_ADDR "B003"
#define BLE_CHAR_NAME_ADDR "B004"
#define BLE_CHAR_IP_ADDR "B005"
#define BLE_CHAR_DEBUG_ADDR "D001"

class NetworkManager : public SaveWiFiPasswordNotifier
{
public:
    static NetworkManager *getInstance();
    ~NetworkManager();
    void init(NetworkManagerSettings *settings);
    void initBLE();
    void connect();
    void disconnect();
    void debug(const std::string &value);

private:
    static NetworkManager *_instance;

    static void taskScanWifi(void *pvParameters);
    static void taskNotifyIpChange(void *pvParameters);
    static void WiFiEvent(WiFiEvent_t event);

    NimBLEServer *bleServer;
    NimBLEService *bleService;
    NimBLECharacteristic *bleCharacteristic;

    EventGroupHandle_t _eg = nullptr;
    NetworkManagerSettings *_settings = nullptr;
    SaveDeviceNameCallbacks *_cbSaveDeviceName = nullptr;
    GetWifiListCallbacks *_cbGetWifiList = nullptr;
    SaveWifiNameCallbacks *_cbSaveWifiName = nullptr;
    SaveWifiPasswordCallbacks *_cbSaveWifiPassword = nullptr;

    TaskHandle_t _thScanWifi = nullptr;

    NetworkManager();
    virtual void notifyWifiPaswordReady();
};

#endif