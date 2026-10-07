#include "lcd1602_display.h"
#include <Wire.h>

Lcd1602Display::Lcd1602Display(uint8_t i2cAddr, uint8_t cols, uint8_t rows, int sdaPin, int sclPin, unsigned long pageRotateMs)
    : _i2cAddr(i2cAddr),
      _cols(cols),
      _rows(rows),
      _sdaPin(sdaPin),
      _sclPin(sclPin),
      _pageRotateMs(pageRotateMs),
      _lcd(nullptr),
      _currentPageIndex(0),
      _lastPageSwitch(0),
      _hasActiveArrivals(false) {}

Lcd1602Display::~Lcd1602Display() {
    if (_lcd) {
        delete _lcd;
        _lcd = nullptr;
    }
}

uint8_t Lcd1602Display::detectI2CAddress(uint8_t preferredAddr) {
    Wire.beginTransmission(preferredAddr);
    if (Wire.endTransmission() == 0) {
        Serial.printf("[LCD1602] Найден дисплей по адресу: 0x%02X\n", preferredAddr);
        return preferredAddr;
    }

    // Если по указанному адресу нет ответа, сканируем типичные адреса PCF8574
    const uint8_t commonAddrs[] = {0x27, 0x3F, 0x20, 0x26, 0x38};
    for (uint8_t addr : commonAddrs) {
        if (addr == preferredAddr) continue;
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[LCD1602] Дисплей обнаружен по альтернативному адресу: 0x%02X (вместо 0x%02X)\n",
                          addr, preferredAddr);
            return addr;
        }
    }

    Serial.printf("[LCD1602] Внимание: I2C дисплей не отвечает! Используем адрес по умолчанию 0x%02X\n", preferredAddr);
    return preferredAddr;
}

bool Lcd1602Display::init() {
    Wire.begin(_sdaPin, _sclPin);
    delay(50);

    uint8_t activeAddr = detectI2CAddress(_i2cAddr);
    _i2cAddr = activeAddr;

    _lcd = new LiquidCrystal_I2C(_i2cAddr, _cols, _rows);
    _lcd->init();
    _lcd->backlight();
    _lcd->clear();

    printPadded(0, "KRSK BUS TRACK");
    printPadded(1, "Starting up...");
    delay(1000);

    Serial.println("[LCD1602] LCD 1602 инициализирован.");
    return true;
}

void Lcd1602Display::printPadded(uint8_t row, const String& text) {
    if (!_lcd || row >= _rows) return;

    _lcd->setCursor(0, row);
    String line = text;
    if (line.length() > _cols) {
        line = line.substring(0, _cols);
    }
    _lcd->print(line);

    // Добиваем пробелами до конца строки для очистки предыдущего текста
    for (size_t i = line.length(); i < _cols; i++) {
        _lcd->print(' ');
    }
}

void Lcd1602Display::showConnecting(const String& ssid) {
    _hasActiveArrivals = false;
    printPadded(0, "WiFi Connect...");
    printPadded(1, ssid);
    Serial.printf("[LCD1602] Подключение к Wi-Fi: %s\n", ssid.c_str());
}

void Lcd1602Display::showConnected(const String& ip) {
    _hasActiveArrivals = false;
    printPadded(0, "WiFi Connected!");
    printPadded(1, ip);
    Serial.printf("[LCD1602] Wi-Fi подключен: %s\n", ip.c_str());
    delay(1500);
}

void Lcd1602Display::showLoading() {
    if (!_hasActiveArrivals) {
        printPadded(0, "KRSK Bus Tracker");
        printPadded(1, "Updating data...");
    }
}

String Lcd1602Display::formatBusRow(const ArrivalPrediction& pred) {
    // Получаем краткий префикс типа транспорта: "A", "T", "Tm"
    const char* typeBadge = getVehicleTypeBadgeAscii(pred.vehicleType);
    String routeBadge = String(typeBadge) + pred.routeNumber; // например "A3", "T15"

    // Формируем время ожидания
    char waitBuf[8];
    if (pred.minutesRemaining >= 0) {
        snprintf(waitBuf, sizeof(waitBuf), "%2dm", pred.minutesRemaining);
    } else {
        snprintf(waitBuf, sizeof(waitBuf), "??m");
    }

    // Транслитерируем конечную станцию и убираем кавычки
    String dest = transliterateRuToEn(pred.lastStation);
    dest.replace("\"", "");
    dest.trim();

    // Форматируем ровно 16 символов:
    // Позиция 0..4: Тип и номер (например "A3   " или "A123 ")
    // Позиция 5..8: Время (например " 12m")
    // Позиция 9..15: Конечная (например " Akadem")
    char rowBuf[20];
    snprintf(rowBuf, sizeof(rowBuf), "%-4.4s %-4.4s %-6.6s",
             routeBadge.c_str(),
             waitBuf,
             dest.c_str());

    return String(rowBuf);
}

void Lcd1602Display::renderCurrentPage() {
    if (!_lcd || _arrivals.empty()) return;

    // Первая строка: текущий автобус
    if (_currentPageIndex < _arrivals.size()) {
        printPadded(0, formatBusRow(_arrivals[_currentPageIndex]));
    } else {
        printPadded(0, "");
    }

    // Вторая строка: следующий автобус (если есть) или системное время/номер остановки
    if (_currentPageIndex + 1 < _arrivals.size()) {
        printPadded(1, formatBusRow(_arrivals[_currentPageIndex + 1]));
    } else {
        // Если остался только 1 автобус на странице, пишем системное время
        char timeBuf[17];
        snprintf(timeBuf, sizeof(timeBuf), "Time: %-10.10s", _systemTime.c_str());
        printPadded(1, timeBuf);
    }
}

void Lcd1602Display::showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) {
    _stopId = stopId;
    _systemTime = systemTime;
    _arrivals = arrivals;
    _currentPageIndex = 0;
    _lastPageSwitch = millis();
    _hasActiveArrivals = true;

    renderCurrentPage();

    // Дублируем в Serial красивый лог
    Serial.printf("\n[LCD1602] Обновлены данные (всего %u ед. транспорта):\n", (unsigned int)arrivals.size());
    for (size_t i = 0; i < arrivals.size(); i++) {
        Serial.printf("  %s\n", formatBusRow(arrivals[i]).c_str());
    }
}

void Lcd1602Display::showEmpty(const String& stopId, const String& systemTime) {
    _hasActiveArrivals = false;
    _arrivals.clear();

    char line0[17];
    snprintf(line0, sizeof(line0), "Stop %-4.4s  %s", stopId.c_str(), systemTime.substring(0, 5).c_str());
    printPadded(0, line0);
    printPadded(1, "No buses now");

    Serial.println("[LCD1602] Нет данных о приближающемся транспорте.");
}

void Lcd1602Display::showError(const String& errorMessage) {
    _hasActiveArrivals = false;
    printPadded(0, "Error!");
    printPadded(1, errorMessage.substring(0, _cols));
    Serial.printf("[LCD1602] Ошибка: %s\n", errorMessage.c_str());
}

void Lcd1602Display::update() {
    if (!_hasActiveArrivals || _arrivals.size() <= 2) {
        return;
    }

    unsigned long now = millis();
    if (now - _lastPageSwitch >= _pageRotateMs) {
        _lastPageSwitch = now;
        _currentPageIndex += 2;
        if (_currentPageIndex >= _arrivals.size()) {
            _currentPageIndex = 0;
        }
        renderCurrentPage();
    }
}
