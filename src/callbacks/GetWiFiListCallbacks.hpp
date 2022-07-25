#ifndef GET_WIFI_LIST_CALLBACKS_h
#define GET_WIFI_LIST_CALLBACKS_h

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

#define EVENT_SCAN_WIFI 1 << 0
#define EVENT_IP_CHANGE 1 << 1

class GetWifiListCallbacks : public NimBLECharacteristicCallbacks
{
public:
    GetWifiListCallbacks(NetworkManagerSettings *settings, EventGroupHandle_t eg)
    {
        _settings = settings;
        _eg = eg;
    }

    void onSubscribe(NimBLECharacteristic *pCharacteristic, ble_gap_conn_desc *desc, uint16_t subValue)
	{
		if (subValue == 1)
		{
            xEventGroupSetBits(_eg, EVENT_SCAN_WIFI);
		}
	};

private:
    NetworkManagerSettings *_settings = nullptr;
    EventGroupHandle_t _eg = nullptr;
};

#endif