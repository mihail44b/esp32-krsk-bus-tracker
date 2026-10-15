#pragma once

#include "display_base.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

class St7789Display : public DisplayBase {
public:
    /**
     * @brief Конструктор цветного IPS-дисплея ST7789 (170x320)
     * @param csPin Пин CS (Chip Select, по умолчанию 5)
     * @param dcPin Пин DC (Data/Command, по умолчанию 2)
     * @param rstPin Пин RST (Reset, по умолчанию 4)
     * @param mosiPin Пин MOSI / SDA (по умолчанию 23)
     * @param sclkPin Пин SCLK / SCL (по умолчанию 18)
     * @param pageRotateMs Период автоматической смены страниц (мс)
     */
    St7789Display(
        int csPin = 5,
        int dcPin = 2,
        int rstPin = 4,
        int mosiPin = 23,
        int sclkPin = 18,
        unsigned long pageRotateMs = 5000
    );

    ~St7789Display() override;

    bool init() override;
    void showConnecting(const String& ssid) override;
    void showConnected(const String& ip) override;
    void showLoading() override;
    void showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) override;
    void showEmpty(const String& stopId, const String& systemTime) override;
    void showError(const String& errorMessage) override;

    void update() override;

private:
    int _csPin;
    int _dcPin;
    int _rstPin;
    int _mosiPin;
    int _sclkPin;
    unsigned long _pageRotateMs;

    Adafruit_ST7789* _tft;

    std::vector<ArrivalPrediction> _arrivals;
    String _stopId;
    String _systemTime;
    size_t _currentPageIndex;
    unsigned long _lastPageSwitch;
    bool _hasActiveArrivals;

    void hardReset();
    void renderCurrentPage();
    void renderHeader(const String& stopId, const String& sysTime);
    void renderBusCard(int y, const ArrivalPrediction& pred);
    void renderPageIndicators(size_t activePage, size_t totalPages);
    uint16_t getBadgeColor(VehicleType type) const;
};
