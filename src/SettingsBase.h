#ifndef SETTINGS_BASE_h
#define SETTINGS_BASE_h

#include <Arduino.h>
#include <EEPROM.h>

template <typename T>
class SettingsBase
{
public:
  SettingsBase(int startAddr, int signature)
  {
    _startAddr = startAddr;
    _signature = signature;
  }
  T getSettings()
  {
    T settings;
    EEPROM.readBytes(_startAddr, &settings, sizeof(T));

    if (settings.signature != _signature)
    {
      settings = getDefaultSettings();
      saveSettings(settings);
    }

    return settings;
  }
  void saveSettings(T newSettings)
  {
    newSettings.signature = _signature;
    EEPROM.writeBytes(_startAddr, &newSettings, sizeof(T));
    EEPROM.commit();
  }

protected:
  virtual T getDefaultSettings()
  {
    T settings = {_signature};
    return settings;
  }

private:
  uint32_t _startAddr = 0;
  uint32_t _signature = 0;
};

#endif