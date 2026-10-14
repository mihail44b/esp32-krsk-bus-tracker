#include "wifi_manager.h"

WiFiManager::WiFiManager()
    : _ssid(nullptr),
      _password(nullptr),
      _lastReconnectAttempt(0),
      _reconnectInterval(10000),
      _wasConnected(false) {}

bool WiFiManager::begin(const char* ssid, const char* password, const char* apSsid, unsigned long timeoutMs) {
    _ssid = ssid;
    _password = password;
    _wasConnected = false;
    _lastReconnectAttempt = 0;

    WiFi.persistent(false); // Не изнашивать flash память сохранением настроек WiFi
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(true);
    delay(100);

    // Запуск собственной точки доступа для мгновенного входа с любого смартфона/ПК
    IPAddress apIP(192, 168, 4, 1);
    IPAddress netMsk(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    const char* actualApSsid = (apSsid && strlen(apSsid) > 0) ? apSsid : "KRSK-Bus-Setup";
    WiFi.softAP(actualApSsid, nullptr, 1);
    Serial.printf("[WiFi] Точка доступа для настройки запущена: '%s' (IP: %s, канал 1)\n",
                  actualApSsid, WiFi.softAPIP().toString().c_str());

    // Если имя сети не задано — сразу переходим в режим ожидания настройки
    if (!_ssid || strlen(_ssid) == 0) {
        Serial.println("[WiFi] Сеть не настроена. Ожидание конфигурации через веб-портал...");
        return false;
    }

    // Подключение к клиентской сети для доступа в интернет к API Гортранса
    Serial.printf("[WiFi] Подключение к Wi-Fi: %s ...\n", _ssid);
    WiFi.begin(_ssid, _password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start < timeoutMs)) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        _wasConnected = true;
        Serial.printf("[WiFi] Успешно подключено к интернету! IP: %s (RSSI: %d dBm)\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }

    Serial.println("[WiFi] Предупреждение: таймаут подключения к интернету! Точка доступа активна для смены сети.");
    return false;
}

String WiFiManager::getApIP() const {
    return WiFi.softAPIP().toString();
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

    // Если сеть настроена, но нет связи, пробуем повторное подключение без блокировки
    if (!currentConnected && _ssid && strlen(_ssid) > 0) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= _reconnectInterval) {
            _lastReconnectAttempt = now;
            Serial.printf("[WiFi] Попытка переподключения к '%s'...\n", _ssid);
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

std::vector<ScannedNetwork> WiFiManager::scanNetworks() {
    std::vector<ScannedNetwork> list;
    int n = WiFi.scanNetworks(false, true); // быстрый поиск
    for (int i = 0; i < n; ++i) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() > 0) {
            bool exists = false;
            for (const auto& existing : list) {
                if (existing.ssid == ssid) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                ScannedNetwork item;
                item.ssid = ssid;
                item.rssi = WiFi.RSSI(i);
                item.isSecure = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
                list.push_back(item);
            }
        }
    }
    WiFi.scanDelete();

    // Сортировка сетей по уровню сигнала (сначала самые сильные)
    std::sort(list.begin(), list.end(), [](const ScannedNetwork& a, const ScannedNetwork& b) {
        return a.rssi > b.rssi;
    });

    return list;
}

