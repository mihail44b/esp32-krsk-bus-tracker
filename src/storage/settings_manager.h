#pragma once

#include <Arduino.h>
#include <Preferences.h>

class SettingsManager {
public:
    SettingsManager();

    bool begin(const char* defaultSsid = "", const char* defaultPass = "", const char* defaultStopId = "646");

    String getSSID() const;
    String getPassword() const;
    String getStopId() const;

    bool hasConfig() const;

    void save(const String& ssid, const String& password, const String& stopId);
    void clear();

private:
    Preferences _prefs;
    String _ssid;
    String _password;
    String _stopId;
};

