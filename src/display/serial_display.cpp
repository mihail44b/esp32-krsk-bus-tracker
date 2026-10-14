#include "serial_display.h"

SerialDisplay::SerialDisplay(unsigned long baudRate) : _baudRate(baudRate) {}

bool SerialDisplay::init() {
    if (!Serial) {
        Serial.begin(_baudRate);
        delay(100);
    }
    Serial.println("\n[DISPLAY] SerialDisplay инициализирован.");
    return true;
}

void SerialDisplay::showConnecting(const String& ssid) {
    Serial.printf("[DISPLAY] Подключение к Wi-Fi сети '%s'...\n", ssid.c_str());
}

void SerialDisplay::showConnected(const String& ip) {
    Serial.printf("[DISPLAY] Подключено к сети! IP: %s\n", ip.c_str());
}

void SerialDisplay::showLoading() {
    Serial.println("[DISPLAY] Запрос актуального расписания с сервера...");
}

void SerialDisplay::showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) {
    Serial.println();
    Serial.println("==========================================================================================");
    Serial.printf("  ТАБЛО ТРАНСПОРТА КРАСНОЯРСКА | Остановка ID: %s | Сервер: %s\n",
                  stopId.c_str(),
                  systemTime.length() > 0 ? systemTime.c_str() : "--:--:--");
    Serial.println("==========================================================================================");
    Serial.println(" Маршрут  |  Тип   | Прибытие |  Ожидание  | Направление (Конечная)");
    Serial.println("----------+--------+----------+------------+----------------------------------------------");

    for (const auto& item : arrivals) {
        char waitBuf[32];
        if (item.minutesRemaining >= 0) {
            snprintf(waitBuf, sizeof(waitBuf), "~%d мин", item.minutesRemaining);
        } else {
            snprintf(waitBuf, sizeof(waitBuf), "??");
        }

        Serial.printf(" %-8s | %-6s | %-8s | %-10s | %s\n",
                      item.routeNumber.c_str(),
                      getVehicleTypeBadge(item.vehicleType),
                      item.arrivalTime.c_str(),
                      waitBuf,
                      item.lastStation.c_str());
    }

    Serial.println("==========================================================================================");
    Serial.printf(" Всего единиц транспорта на подходе: %u\n", (unsigned int)arrivals.size());
    Serial.println();
}

void SerialDisplay::showEmpty(const String& stopId, const String& systemTime) {
    Serial.println();
    Serial.println("==========================================================================================");
    Serial.printf("  ТАБЛО ТРАНСПОРТА КРАСНОЯРСКА | Остановка ID: %s | Сервер: %s\n",
                  stopId.c_str(),
                  systemTime.length() > 0 ? systemTime.c_str() : "--:--:--");
    Serial.println("==========================================================================================");
    Serial.println(" В ближайшее время подходящего транспорта нет или смена завершена.");
    Serial.println("==========================================================================================");
    Serial.println();
}

void SerialDisplay::showError(const String& errorMessage) {
    Serial.println();
    Serial.println("******************************************************************************************");
    Serial.printf(" [ОШИБКА ДИСПЛЕЯ / API]: %s\n", errorMessage.c_str());
    Serial.println("******************************************************************************************");
    Serial.println();
}
