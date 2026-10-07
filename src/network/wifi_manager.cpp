#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : _ssid(nullptr),
      _password(nullptr),
      _lastReconnectAttempt(0),
      _reconnectInterval(10000),
      _wasConnected(false) {}

bool WiFiManager::begin(const char* ssid, const char* password, unsigned long timeoutMs) {
    _ssid = ssid;
    _password = password;
    _wasConnected = false;
    _lastReconnectAttempt = 0;

    WiFi.persistent(false); // Не изнашивать flash память сохранением настроек WiFi
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    Serial.printf("[WiFi] Подключение к SSID: %s ...\n", _ssid);
    WiFi.begin(_ssid, _password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start < timeoutMs)) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        _wasConnected = true;
        Serial.printf("[WiFi] Успешно подключено! IP: %s (RSSI: %d dBm)\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }

    Serial.println("[WiFi] Ошибка: таймаут подключения к Wi-Fi!");
    return false;
}

void WiFiManager::update() {
    bool currentConnected = (WiFi.status() == WL_CONNECTED);

    if (currentConnected && !_wasConnected) {
        Serial.printf("[WiFi] Связь восстановлена! IP: %s (RSSI: %d dBm)\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        _wasConnected = true;
    } else if (!currentConnected && _wasConnected) {
        Serial.println("[WiFi] Потеряно соединение с Wi-Fi!");
        _wasConnected = false;
        _lastReconnectAttempt = millis();
    }

    // Если нет связи, пробуем повторное подключение без блокировки основного цикла
    if (!currentConnected) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= _reconnectInterval) {
            _lastReconnectAttempt = now;
            Serial.printf("[WiFi] Попытка переподключения к %s...\n", _ssid ? _ssid : "N/A");
            WiFi.disconnect();
            WiFi.reconnect();
        }
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::getIP() const {
    return WiFi.localIP().toString();
}

int WiFiManager::getRSSI() const {
    return WiFi.RSSI();
}
