#include <ModuleSettings.h>

ModuleSettings::ModuleSettings()
    : SettingsBase(START_ADDR, SIGNATURE)
{
}

char *ModuleSettings::getModuleName()
{
    readSettings();
    return _settings.name;
}

void ModuleSettings::setModuleName(char *value)
{
    strlcpy(_settings.name, value, sizeof(_settings.name));
    writeSettings();
}

char *ModuleSettings::getWifiName()
{
    readSettings();
    return _settings.WiFiName;
}

void ModuleSettings::setWifiName(char *value)
{
    strlcpy(_settings.WiFiName, value, sizeof(_settings.WiFiName));
    writeSettings();
}

char *ModuleSettings::getWifiPassword()
{
    readSettings();
    return _settings.WiFiPass;
}

void ModuleSettings::setWifiPassword(char *value)
{
    strlcpy(_settings.WiFiPass, value, sizeof(_settings.WiFiPass));
    writeSettings();
}

void ModuleSettings::readSettings()
{
    _settings = getSettings();
}

void ModuleSettings::writeSettings()
{
    saveSettings(_settings);
}

Settings ModuleSettings::getDefaultSettings()
{
    Settings settings = {SIGNATURE, "", "", ""};
    return settings;
}