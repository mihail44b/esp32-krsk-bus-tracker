#pragma once

#include <Arduino.h>
#include <vector>
#include "types.h"

/**
 * @brief Абстрактный интерфейс для устройств отображения.
 * Позволяет прозрачно подменять устройство вывода (Serial, SSD1306 OLED, LCD2004, MAX7219)
 * без изменения бизнес-логики приложения.
 */
class DisplayBase {
public:
    virtual ~DisplayBase() = default;

    /**
     * @brief Инициализация дисплея
     */
    virtual bool init() = 0;

    /**
     * @brief Индикация процесса подключения к Wi-Fi
     */
    virtual void showConnecting(const String& ssid) = 0;

    /**
     * @brief Индикация успешного подключения к сети
     */
    virtual void showConnected(const String& ip) = 0;

    /**
     * @brief Индикация запроса данных из сети
     */
    virtual void showLoading() = 0;

    /**
     * @brief Отображение полученных прогнозов транспорта
     * @param stopId ID остановки
     * @param systemTime Системное время сервера
     * @param arrivals Список прогнозов прибытия
     */
    virtual void showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) = 0;

    /**
     * @brief Индикация отсутствия приближающихся маршрутов
     */
    virtual void showEmpty(const String& stopId, const String& systemTime) = 0;

    /**
     * @brief Индикация ошибки (сетевой или синтаксической)
     */
    virtual void showError(const String& errorMessage) = 0;
};
