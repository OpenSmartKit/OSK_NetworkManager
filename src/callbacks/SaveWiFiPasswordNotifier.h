#ifndef SAVE_WIFI_PASSWORD_NOTIFIER_h
#define SAVE_WIFI_PASSWORD_NOTIFIER_h

#include <Arduino.h>

class SaveWiFiPasswordNotifier
{
public:
    virtual void notifyWifiPaswordReady() = 0;
};

#endif