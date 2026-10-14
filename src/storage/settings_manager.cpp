#include "settings_manager.h"

static const char PREFS_NAMESPACE[] = "bustracker";
static const char KEY_SSID[] = "wifi_ssid";
static const char KEY_PASS[] = "wifi_pass";
static const char KEY_STOP[] = "stop_id";
static const char KEY_CLEARED[] = "cleared";

SettingsManager::SettingsManager() 
    : _ssid(""), _password(""), _stopId("646") {}

bool SettingsManager::begin(const char* defaultSsid, const char* defaultPass, const char* defaultStopId) {
    if (!_prefs.begin(PREFS_NAMESPACE, false)) {
        Serial.println("[STORAGE] Ошибка открытия NVS Preferences!");
        return false;
    }

    bool isCleared = _prefs.getBool(KEY_CLEARED, false);

    _ssid = _prefs.getString(KEY_SSID, "");
    _password = _prefs.getString(KEY_PASS, "");
    _stopId = _prefs.getString(KEY_STOP, "");

    // Если настройки не были явно сброшены и в NVS пусто — используем значения по умолчанию
    if (!isCleared) {
        if (_ssid.length() == 0 && defaultSsid && strlen(defaultSsid) > 0 && strcmp(defaultSsid, "YOUR_WIFI_SSID") != 0) {
            _ssid = String(defaultSsid);
        }
        if (_password.length() == 0 && defaultPass && strlen(defaultPass) > 0 && strcmp(defaultPass, "YOUR_WIFI_PASSWORD") != 0) {
            _password = String(defaultPass);
        }
    }

    if (_stopId.length() == 0) {
        _stopId = (defaultStopId && strlen(defaultStopId) > 0) ? String(defaultStopId) : "646";
    }

    Serial.printf("[STORAGE] Настройки загружены: SSID='%s', StopID='%s' (Сброс: %s)\n",
                  _ssid.c_str(), _stopId.c_str(), isCleared ? "ДА" : "НЕТ");
    return true;
}

String SettingsManager::getSSID() const {
    return _ssid;
}

String SettingsManager::getPassword() const {
    return _password;
}

String SettingsManager::getStopId() const {
    return _stopId;
}

bool SettingsManager::hasConfig() const {
    return _ssid.length() > 0;
}

void SettingsManager::save(const String& ssid, const String& password, const String& stopId) {
    _ssid = ssid;
    _password = password;
    if (stopId.length() > 0) {
        _stopId = stopId;
    }

    _prefs.remove(KEY_CLEARED);
    _prefs.putString(KEY_SSID, _ssid);
    _prefs.putString(KEY_PASS, _password);
    _prefs.putString(KEY_STOP, _stopId);

    Serial.printf("[STORAGE] Настройки сохранены в NVS Flash: SSID='%s', StopID='%s'\n",
                  _ssid.c_str(), _stopId.c_str());
}

void SettingsManager::clear() {
    _prefs.clear();
    _prefs.putBool(KEY_CLEARED, true);
    _ssid = "";
    _password = "";
    _stopId = "646";
    Serial.println("[STORAGE] Настройки очищены (установлен флаг сброса).");
}

