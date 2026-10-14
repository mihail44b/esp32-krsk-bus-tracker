#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

struct ScannedNetwork {
    String ssid;
    int32_t rssi;
    bool isSecure;
};

class WiFiManager {
public:
    WiFiManager();

    /**
     * @brief Инициализация Wi-Fi в режиме AP+STA
     * @param ssid Имя клиентской Wi-Fi сети для выхода в интернет
     * @param password Пароль к клиентской сети
     * @param apSsid Имя собственной точки доступа для Captive Portal
     * @param timeoutMs Максимальное время ожидания подключения (мс)
     * @return true если подключение успешно, false если таймаут
     */
    bool begin(const char* ssid, const char* password, const char* apSsid = "KRSK-Bus-Setup", unsigned long timeoutMs = 10000);

    /**
     * @brief Неблокирующая проверка соединения и авто-переподключение.
     * Вызывается в loop().
     */
    void update();

    /**
     * @brief Проверка текущего статуса подключения
     */
    bool isConnected() const;

    /**
     * @brief Получить текущий IP-адрес клиента (STA)
     */
    String getIP() const;

    /**
     * @brief Получить IP-адрес точки доступа (AP)
     */
    String getApIP() const;

    /**
     * @brief Получить уровень сигнала RSSI в dBm
     */
    int getRSSI() const;

    /**
     * @brief Сканирование доступных 2.4 ГГц сетей вокруг с уровнем сигнала
     */
    std::vector<ScannedNetwork> scanNetworks();

private:
    const char* _ssid;
    const char* _password;
    unsigned long _lastReconnectAttempt;
    unsigned long _reconnectInterval;
    bool _wasConnected;
};
