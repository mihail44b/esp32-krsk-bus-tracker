#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "types.h"

class SettingsManager;
class WiFiManager;

class WebServerManager {
public:
    WebServerManager();

    bool begin(SettingsManager* settings, WiFiManager* wifi, uint16_t port = 80);
    void update();
    void setArrivals(const StopArrivalsResult& result);

private:
    WebServer _server;
    DNSServer _dnsServer;
    SettingsManager* _settings;
    WiFiManager* _wifi;
    StopArrivalsResult _latestResult;
    unsigned long _lastUpdateTimestamp;

    void handleRoot();
    void handleSettings();
    void handleApiArrivals();
    void handleApiScan();
    void handleApiSaveSettings();
    void handleApiReset();
    void handleNotFound();
};

