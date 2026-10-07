#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();

    /**
     * @brief Инициализация Wi-Fi и первичное подключение с таймаутом
     * @param ssid Имя Wi-Fi сети
     * @param password Пароль к сети
     * @param timeoutMs Максимальное время ожидания подключения (мс)
     * @return true если подключение успешно, false если таймаут
     */
    bool begin(const char* ssid, const char* password, unsigned long timeoutMs = 20000);

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
     * @brief Получить текущий IP-адрес
     */
    String getIP() const;

    /**
     * @brief Получить уровень сигнала RSSI в dBm
     */
    int getRSSI() const;

private:
    const char* _ssid;
    const char* _password;
    unsigned long _lastReconnectAttempt;
    unsigned long _reconnectInterval;
    bool _wasConnected;
};
