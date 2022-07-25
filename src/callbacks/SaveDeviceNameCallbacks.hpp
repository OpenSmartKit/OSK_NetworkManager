#ifndef SAVE_DEVICE_NAME_CALLBACKS_h
#define SAVE_DEVICE_NAME_CALLBACKS_h

#include <Arduino.h>
#include <WiFi.h>
#include <NimBLEDevice.h>
#include <NetworkManagerSettings.h>

class SaveDeviceNameCallbacks : public NimBLECharacteristicCallbacks
{
public:
    SaveDeviceNameCallbacks(NetworkManagerSettings *settings)
    {
        _settings = settings;
    }

    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        char buffer[32];
        strlcpy(buffer, pCharacteristic->getValue().c_str(), sizeof(buffer));
        _settings->setModuleName(buffer);
    };

private:
    NetworkManagerSettings *_settings = nullptr;
};

#endif