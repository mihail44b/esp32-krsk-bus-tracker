#pragma once

#include "display_base.h"
#include <LiquidCrystal_I2C.h>

class Lcd1602Display : public DisplayBase {
public:
    /**
     * @brief Конструктор дисплея LCD 1602 (I2C)
     * @param i2cAddr Адрес на шине I2C (обычно 0x27 или 0x3F)
     * @param cols Количество колонок (16)
     * @param rows Количество строк (2)
     * @param sdaPin GPIO пин SDA (21)
     * @param sclPin GPIO пин SCL (22)
     * @param pageRotateMs Период автоматической смены страниц прогнозов в мс
     */
    Lcd1602Display(
        uint8_t i2cAddr = 0x27,
        uint8_t cols = 16,
        uint8_t rows = 2,
        int sdaPin = 21,
        int sclPin = 22,
        unsigned long pageRotateMs = 4000
    );

    ~Lcd1602Display() override;

    bool init() override;
    void showConnecting(const String& ssid) override;
    void showConnected(const String& ip) override;
    void showLoading() override;
    void showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) override;
    void showEmpty(const String& stopId, const String& systemTime) override;
    void showError(const String& errorMessage) override;

    /**
     * @brief Циклическое перелистывание страниц с прогнозами
     */
    void update() override;

private:
    uint8_t _i2cAddr;
    uint8_t _cols;
    uint8_t _rows;
    int _sdaPin;
    int _sclPin;
    unsigned long _pageRotateMs;

    LiquidCrystal_I2C* _lcd;

    std::vector<ArrivalPrediction> _arrivals;
    String _stopId;
    String _systemTime;
    size_t _currentPageIndex;
    unsigned long _lastPageSwitch;
    bool _hasActiveArrivals;

    uint8_t detectI2CAddress(uint8_t preferredAddr);
    void printPadded(uint8_t row, const String& text);
    void renderCurrentPage();
    String formatBusRow(const ArrivalPrediction& pred);
};
