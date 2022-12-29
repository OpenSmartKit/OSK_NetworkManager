#ifndef MODULE_SETTINGS_h
#define MODULE_SETTINGS_h

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
#include <functional>
#include <Arduino.h>
#include <SettingsBase.h>
#include <NetworkManagerSettings.h>

#define START_ADDR 0
#define SIGNATURE 0x123544F0

struct Settings
{
  uint32_t signature;
  char name[32];
  char WiFiName[32];
  char WiFiPass[32];
};

class ModuleSettings : public NetworkManagerSettings, public SettingsBase<Settings>
{
public:
  ModuleSettings();
  virtual char *getModuleName();
  virtual void setModuleName(char *value);
  virtual char *getWifiName();
  virtual void setWifiName(char *value);
  virtual char *getWifiPassword();
  virtual void setWifiPassword(char *value);

protected:
  virtual Settings getDefaultSettings();

private:
  Settings _settings;

  void readSettings();
  void writeSettings();
};

#endif