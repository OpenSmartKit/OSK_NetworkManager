#ifndef NETWORK_MANAGER_SETTINGS_h
#define NETWORK_MANAGER_SETTINGS_h

#include <Arduino.h>

class NetworkManagerSettings
{
public:
    virtual char *getModuleName() = 0;
    virtual void setModuleName(char *value) = 0;
    virtual char *getWifiName() = 0;
    virtual void setWifiName(char *value) = 0;
    virtual char *getWifiPassword() = 0;
    virtual void setWifiPassword(char *value) = 0;
};

#endif