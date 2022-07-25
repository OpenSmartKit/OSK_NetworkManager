#ifndef SAVE_WIFI_PASSWORD_CALLBACKS_h
#define SAVE_WIFI_PASSWORD_CALLBACKS_h

#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <NetworkManagerSettings.h>
#include <callbacks/SaveWiFiPasswordNotifier.h>

class SaveWifiPasswordCallbacks : public NimBLECharacteristicCallbacks
{
public:
    SaveWifiPasswordCallbacks(NetworkManagerSettings *settings, SaveWiFiPasswordNotifier *notifier)
    {
        _settings = settings;
        _notifier = notifier;
    };

    void onWrite(NimBLECharacteristic *pCharacteristic)
	{
        char buffer[32];
		strlcpy(buffer, pCharacteristic->getValue().c_str(), sizeof(buffer));
		_settings->setWifiPassword(buffer);
        _notifier->notifyWifiPaswordReady();
	};

private:
    NetworkManagerSettings *_settings = nullptr;
    SaveWiFiPasswordNotifier *_notifier = nullptr;
};

#endif