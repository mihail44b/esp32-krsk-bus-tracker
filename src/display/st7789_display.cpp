#include "st7789_display.h"

// Цветовая палитра (RGB565)
#define COLOR_BG          0x0842  // Темно-серый / почти черный фон (#0F1117)
#define COLOR_HEADER_BG   0x118D  // Темно-синий (#1A2942)
#define COLOR_CARD_BG     0x18C5  // Графитовый серый (#222630)
#define COLOR_CARD_BORDER 0x2969  // Граница карточки (#3B4254)
#define COLOR_TEXT_MUTED  0x94B2  // Приглушенный текст (#9CA3AF)

#define COLOR_BUS_RED     0xD980  // Красный бейдж автобуса
#define COLOR_TROLLEY_BLU 0x1BDE  // Синий бейдж троллейбуса
#define COLOR_TRAM_GRN    0x2506  // Зеленый бейдж трамвая

#define COLOR_WAIT_FAST   0xFA66  // Ярко-оранжевый / красный (<=3 мин)
#define COLOR_WAIT_MED    0x2668  // Зеленый (<=10 мин)
#define COLOR_WAIT_SLOW   0xFD00  // Желтый / нейтральный (>10 мин)

St7789Display::St7789Display(int csPin, int dcPin, int rstPin, int mosiPin, int sclkPin, unsigned long pageRotateMs)
    : _csPin(csPin),
      _dcPin(dcPin),
      _rstPin(rstPin),
      _mosiPin(mosiPin),
      _sclkPin(sclkPin),
      _pageRotateMs(pageRotateMs),
      _tft(nullptr),
      _currentPageIndex(0),
      _lastPageSwitch(0),
      _hasActiveArrivals(false) {}

St7789Display::~St7789Display() {
    if (_tft) {
        delete _tft;
        _tft = nullptr;
    }
}

void St7789Display::hardReset() {
    pinMode(_rstPin, OUTPUT);
    digitalWrite(_rstPin, HIGH);
    delay(30);
    digitalWrite(_rstPin, LOW);
    delay(80);
    digitalWrite(_rstPin, HIGH);
    delay(150);
}

bool St7789Display::init() {
    hardReset();

    // Явная инициализация SPI
    SPI.begin(_sclkPin, -1, _mosiPin, _csPin);

    _tft = new Adafruit_ST7789(_csPin, _dcPin, _rstPin);
    _tft->init(170, 320);

    // 4 МГц для максимальной устойчивости при макетном подключении
    _tft->setSPISpeed(4000000);
    _tft->setRotation(1); // Альбомная ориентация: 320x170

    _tft->fillScreen(COLOR_BG);

    // Приветственный экран
    _tft->fillRoundRect(10, 10, 300, 150, 8, COLOR_HEADER_BG);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(2);
    _tft->setCursor(30, 45);
    _tft->println("KRSK BUS TRACKER");

    _tft->setTextSize(1);
    _tft->setTextColor(ST77XX_YELLOW);
    _tft->setCursor(30, 80);
    _tft->println("ESP32 DevKit v1 + ST7789 IPS 1.9\"");

    _tft->setTextColor(COLOR_TEXT_MUTED);
    _tft->setCursor(30, 105);
    _tft->println("Starting system...");

    Serial.println("[ST7789] Дисплей ST7789 инициализирован (320x170).");
    return true;
}

void St7789Display::showConnecting(const String& ssid) {
    if (!_tft) return;
    _hasActiveArrivals = false;

    _tft->fillScreen(COLOR_BG);
    _tft->fillRoundRect(10, 20, 300, 130, 8, COLOR_CARD_BG);
    _tft->drawRoundRect(10, 20, 300, 130, 8, COLOR_CARD_BORDER);

    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(2);
    _tft->setCursor(30, 40);
    _tft->print("Wi-Fi Connecting");

    _tft->setTextSize(1);
    _tft->setTextColor(COLOR_TEXT_MUTED);
    _tft->setCursor(30, 75);
    _tft->print("Target Network:");

    _tft->setTextSize(2);
    _tft->setTextColor(ST77XX_CYAN);
    _tft->setCursor(30, 95);
    _tft->print(ssid);

    Serial.printf("[ST7789] Подключение к Wi-Fi: %s\n", ssid.c_str());
}

void St7789Display::showConnected(const String& ip) {
    if (!_tft) return;

    _tft->fillRoundRect(10, 20, 300, 130, 8, COLOR_CARD_BG);
    _tft->drawRoundRect(10, 20, 300, 130, 8, COLOR_CARD_BORDER);

    _tft->setTextColor(ST77XX_GREEN);
    _tft->setTextSize(2);
    _tft->setCursor(30, 40);
    _tft->print("Wi-Fi Connected!");

    _tft->setTextSize(1);
    _tft->setTextColor(COLOR_TEXT_MUTED);
    _tft->setCursor(30, 75);
    _tft->print("Assigned IP Address:");

    _tft->setTextSize(2);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setCursor(30, 95);
    _tft->print(ip);

    delay(1200);
}

void St7789Display::showLoading() {
    if (!_tft) return;

    if (!_hasActiveArrivals) {
        _tft->fillScreen(COLOR_BG);
        _tft->fillRoundRect(10, 30, 300, 110, 8, COLOR_CARD_BG);

        _tft->setTextColor(ST77XX_WHITE);
        _tft->setTextSize(2);
        _tft->setCursor(35, 55);
        _tft->print("Krasnoyarskgortrans");

        _tft->setTextSize(1);
        _tft->setTextColor(ST77XX_YELLOW);
        _tft->setCursor(35, 88);
        _tft->print("Downloading live timetable...");
    } else {
        // Если данные уже есть, показываем маленькую точку обновления в правом верхнем углу
        _tft->fillCircle(310, 13, 3, ST77XX_YELLOW);
    }
}

uint16_t St7789Display::getBadgeColor(VehicleType type) const {
    switch (type) {
        case VehicleType::BUS:        return COLOR_BUS_RED;
        case VehicleType::TROLLEYBUS: return COLOR_TROLLEY_BLU;
        case VehicleType::TRAM:       return COLOR_TRAM_GRN;
        default:                      return COLOR_CARD_BORDER;
    }
}

void St7789Display::renderHeader(const String& stopId, const String& sysTime) {
    // Темная плашка шапки
    _tft->fillRect(0, 0, 320, 26, COLOR_HEADER_BG);
    _tft->drawFastHLine(0, 26, 320, COLOR_CARD_BORDER);

    // Левый бейдж
    _tft->fillRoundRect(6, 4, 46, 18, 3, COLOR_TROLLEY_BLU);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(1);
    _tft->setCursor(14, 9);
    _tft->print("KRSK");

    // Номер остановки
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(1);
    _tft->setCursor(60, 9);
    _tft->printf("Stop #%s", stopId.c_str());

    // Системное время (крупно справа)
    String timeStr = sysTime.length() >= 5 ? sysTime.substring(0, 5) : "--:--";
    _tft->setTextColor(ST77XX_CYAN);
    _tft->setTextSize(2);
    _tft->setCursor(245, 6);
    _tft->print(timeStr);
}

void St7789Display::renderBusCard(int y, const ArrivalPrediction& pred) {
    // Фоновая карточка
    _tft->fillRoundRect(4, y, 312, 42, 6, COLOR_CARD_BG);
    _tft->drawRoundRect(4, y, 312, 42, 6, COLOR_CARD_BORDER);

    // 1. Левый цветной бейдж с номером маршрута (ширина 50, высота 34)
    uint16_t badgeColor = getBadgeColor(pred.vehicleType);
    _tft->fillRoundRect(8, y + 4, 48, 34, 4, badgeColor);

    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(2);

    // Центрирование номера маршрута в бейдже
    int textX = 14;
    if (pred.routeNumber.length() == 1) {
        textX = 26;
    } else if (pred.routeNumber.length() == 2) {
        textX = 20;
    } else if (pred.routeNumber.length() >= 3) {
        textX = 12;
    }
    _tft->setCursor(textX, y + 13);
    _tft->print(pred.routeNumber);

    // 2. Средняя часть: Направление (конечная) и время прибытия
    String dest = transliterateRuToEn(pred.lastStation);
    dest.replace("\"", "");
    dest.trim();
    if (dest.length() > 20) {
        dest = dest.substring(0, 20);
    }

    _tft->setTextSize(1);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setCursor(64, y + 8);
    _tft->print(dest);

    // Подстрочник (тип транспорта + плановое время прибытия)
    _tft->setTextColor(COLOR_TEXT_MUTED);
    _tft->setCursor(64, y + 24);
    const char* typeName = (pred.vehicleType == VehicleType::TROLLEYBUS) ? "Trolley" :
                           (pred.vehicleType == VehicleType::TRAM) ? "Tram" : "Bus";
    _tft->printf("%s | Arr: %s", typeName, pred.arrivalTime.c_str());

    // 3. Правая часть: Оставшиеся минуты (бейджик)
    int waitMin = pred.minutesRemaining;
    char waitBuf[12];
    uint16_t waitBgColor = COLOR_WAIT_SLOW;
    uint16_t waitTextColor = ST77XX_YELLOW;

    if (waitMin >= 0) {
        snprintf(waitBuf, sizeof(waitBuf), "~%dm", waitMin);
        if (waitMin <= 3) {
            waitBgColor = COLOR_BUS_RED;
            waitTextColor = ST77XX_WHITE;
        } else if (waitMin <= 10) {
            waitBgColor = COLOR_TRAM_GRN;
            waitTextColor = ST77XX_WHITE;
        }
    } else {
        snprintf(waitBuf, sizeof(waitBuf), "??");
    }

    // Плашка ожидания справа
    _tft->fillRoundRect(244, y + 6, 66, 30, 4, waitBgColor);
    _tft->setTextColor(waitTextColor);
    _tft->setTextSize(2);

    int waitX = (strlen(waitBuf) <= 3) ? 260 : 250;
    _tft->setCursor(waitX, y + 13);
    _tft->print(waitBuf);
}

void St7789Display::renderPageIndicators(size_t activePage, size_t totalPages) {
    if (totalPages <= 1) return;

    int totalW = totalPages * 12;
    int startX = (320 - totalW) / 2;
    int y = 165;

    for (size_t i = 0; i < totalPages; i++) {
        if (i == activePage) {
            _tft->fillCircle(startX + i * 12 + 4, y, 3, ST77XX_WHITE);
        } else {
            _tft->drawCircle(startX + i * 12 + 4, y, 2, COLOR_TEXT_MUTED);
        }
    }
}

void St7789Display::renderCurrentPage() {
    if (!_tft) return;

    _tft->fillScreen(COLOR_BG);
    renderHeader(_stopId, _systemTime);

    if (_arrivals.empty()) {
        showEmpty(_stopId, _systemTime);
        return;
    }

    // Отображаем до 3 автобусов на экран
    const int startY = 30;
    const int cardHeight = 44;

    for (size_t i = 0; i < 3; i++) {
        size_t index = _currentPageIndex + i;
        if (index < _arrivals.size()) {
            renderBusCard(startY + i * cardHeight, _arrivals[index]);
        }
    }

    size_t totalPages = (_arrivals.size() + 2) / 3;
    size_t activePage = _currentPageIndex / 3;
    renderPageIndicators(activePage, totalPages);
}

void St7789Display::showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) {
    _stopId = stopId;
    _systemTime = systemTime;
    _arrivals = arrivals;
    _currentPageIndex = 0;
    _lastPageSwitch = millis();
    _hasActiveArrivals = true;

    renderCurrentPage();

    Serial.printf("\n[ST7789] Отображено %u ед. транспорта (Стр. 1 из %u)\n",
                  (unsigned int)arrivals.size(),
                  (unsigned int)((arrivals.size() + 2) / 3));
}

void St7789Display::showEmpty(const String& stopId, const String& systemTime) {
    if (!_tft) return;
    _hasActiveArrivals = false;
    _arrivals.clear();

    _tft->fillScreen(COLOR_BG);
    renderHeader(stopId, systemTime);

    _tft->fillRoundRect(10, 40, 300, 100, 8, COLOR_CARD_BG);
    _tft->drawRoundRect(10, 40, 300, 100, 8, COLOR_CARD_BORDER);

    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(2);
    _tft->setCursor(30, 65);
    _tft->print("No vehicles approaching");

    _tft->setTextSize(1);
    _tft->setTextColor(COLOR_TEXT_MUTED);
    _tft->setCursor(30, 100);
    _tft->print("No active routes or shift ended for today.");
}

void St7789Display::showError(const String& errorMessage) {
    if (!_tft) return;
    _hasActiveArrivals = false;

    _tft->fillScreen(COLOR_BG);
    renderHeader("ERR", "--:--");

    _tft->fillRoundRect(10, 40, 300, 110, 8, COLOR_BUS_RED);

    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(2);
    _tft->setCursor(25, 55);
    _tft->print("Network / API Error");

    _tft->setTextSize(1);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setCursor(25, 88);
    _tft->print(errorMessage.substring(0, 45));

    _tft->setTextColor(ST77XX_YELLOW);
    _tft->setCursor(25, 115);
    _tft->print("Retrying in 10-30s...");
}

void St7789Display::update() {
    if (!_hasActiveArrivals || _arrivals.size() <= 3) {
        return;
    }

    unsigned long now = millis();
    if (now - _lastPageSwitch >= _pageRotateMs) {
        _lastPageSwitch = now;
        _currentPageIndex += 3;
        if (_currentPageIndex >= _arrivals.size()) {
            _currentPageIndex = 0;
        }
        renderCurrentPage();
    }
}
