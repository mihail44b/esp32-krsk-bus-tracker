#pragma once

#include <Arduino.h>
#include <vector>

/**
 * @brief Type of public transit vehicle
 */
enum class VehicleType {
    BUS,        // "A" (Автобус)
    TROLLEYBUS, // "Т" / "T" (Троллейбус)
    TRAM,       // "Тр" (Трамвай)
    UNKNOWN
};

/**
 * @brief Represents a single scheduled arrival of a vehicle at the stop
 */
struct ArrivalPrediction {
    String routeId;           // "mr_id" (внутренний ID маршрута, напр. "24")
    String routeNumber;       // "mr_num" (номер маршрута, напр. "3", "63", "85")
    String raceTypeRaw;       // "rl_racetype" (буквенный код, напр. "A")
    VehicleType vehicleType;  // Распознанный тип транспорта
    String systemTime;        // "tc_systime" (серверное время фиксации, напр. "20:42:54")
    String arrivalTime;       // "tc_arrivetime" (прогноз прибытия, напр. "20:55")
    String firstStation;      // "firststation_title" (начальная остановка маршрута)
    String lastStation;       // "laststation_title" (конечная остановка маршрута)
    int minutesRemaining;     // Расчетное время до прибытия в минутах (-1 если не удалось вычислить)
};

/**
 * @brief Container for API response containing list of predictions
 */
struct StopArrivalsResult {
    bool success;
    int httpStatusCode;
    String errorMessage;
    String systemTime;
    std::vector<ArrivalPrediction> predictions;
};

/**
 * @brief Helper to convert raw API racetype string to VehicleType enum
 */
inline VehicleType parseVehicleType(const String& raw) {
    if (raw == "A" || raw == "a" || raw == "А" || raw == "а") {
        return VehicleType::BUS;
    }
    if (raw == "Т" || raw == "т" || raw == "T" || raw == "t") {
        return VehicleType::TROLLEYBUS;
    }
    if (raw == "Тр" || raw == "тр") {
        return VehicleType::TRAM;
    }
    return VehicleType::UNKNOWN;
}

/**
 * @brief Helper to get human-readable short label for vehicle type
 */
inline const char* getVehicleTypeBadge(VehicleType type) {
    switch (type) {
        case VehicleType::BUS:        return "Авт";
        case VehicleType::TROLLEYBUS: return "Трол";
        case VehicleType::TRAM:       return "Трам";
        default:                      return "ТР";
    }
}

/**
 * @brief Helper to get ASCII label for LCD1602 without Cyrillic ROM
 */
inline const char* getVehicleTypeBadgeAscii(VehicleType type) {
    switch (type) {
        case VehicleType::BUS:        return "A";
        case VehicleType::TROLLEYBUS: return "T";
        case VehicleType::TRAM:       return "Tm";
        default:                      return "?";
    }
}

/**
 * @brief Транслитерация UTF-8 строки (кириллица -> латиница) для дисплеев без русских шрифтов (HD44780 LCD1602)
 */
inline String transliterateRuToEn(const String& utf8Str) {
    String result;
    result.reserve(utf8Str.length());

    size_t i = 0;
    while (i < utf8Str.length()) {
        uint8_t c = (uint8_t)utf8Str[i];

        if (c < 0x80) {
            // Обычный ASCII символ
            result += (char)c;
            i++;
        } else if ((c == 0xD0 || c == 0xD1) && i + 1 < utf8Str.length()) {
            uint8_t c2 = (uint8_t)utf8Str[i + 1];
            uint16_t codepoint = ((c & 0x1F) << 6) | (c2 & 0x3F);
            i += 2;

            switch (codepoint) {
                case 0x0410: result += "A"; break;
                case 0x0411: result += "B"; break;
                case 0x0412: result += "V"; break;
                case 0x0413: result += "G"; break;
                case 0x0414: result += "D"; break;
                case 0x0415: result += "E"; break;
                case 0x0401: result += "Yo"; break;
                case 0x0416: result += "Zh"; break;
                case 0x0417: result += "Z"; break;
                case 0x0418: result += "I"; break;
                case 0x0419: result += "Y"; break;
                case 0x041A: result += "K"; break;
                case 0x041B: result += "L"; break;
                case 0x041C: result += "M"; break;
                case 0x041D: result += "N"; break;
                case 0x041E: result += "O"; break;
                case 0x041F: result += "P"; break;
                case 0x0420: result += "R"; break;
                case 0x0421: result += "S"; break;
                case 0x0422: result += "T"; break;
                case 0x0423: result += "U"; break;
                case 0x0424: result += "F"; break;
                case 0x0425: result += "Kh"; break;
                case 0x0426: result += "Ts"; break;
                case 0x0427: result += "Ch"; break;
                case 0x0428: result += "Sh"; break;
                case 0x0429: result += "Shch"; break;
                case 0x042A: break; // Твердый знак
                case 0x042B: result += "Y"; break;
                case 0x042C: break; // Мягкий знак
                case 0x042D: result += "E"; break;
                case 0x042E: result += "Yu"; break;
                case 0x042F: result += "Ya"; break;

                case 0x0430: result += "a"; break;
                case 0x0431: result += "b"; break;
                case 0x0432: result += "v"; break;
                case 0x0433: result += "g"; break;
                case 0x0434: result += "d"; break;
                case 0x0435: result += "e"; break;
                case 0x0451: result += "yo"; break;
                case 0x0436: result += "zh"; break;
                case 0x0437: result += "z"; break;
                case 0x0438: result += "i"; break;
                case 0x0439: result += "y"; break;
                case 0x043A: result += "k"; break;
                case 0x043B: result += "l"; break;
                case 0x043C: result += "m"; break;
                case 0x043D: result += "n"; break;
                case 0x043E: result += "o"; break;
                case 0x043F: result += "p"; break;
                case 0x0440: result += "r"; break;
                case 0x0441: result += "s"; break;
                case 0x0442: result += "t"; break;
                case 0x0443: result += "u"; break;
                case 0x0444: result += "f"; break;
                case 0x0445: result += "kh"; break;
                case 0x0446: result += "ts"; break;
                case 0x0447: result += "ch"; break;
                case 0x0448: result += "sh"; break;
                case 0x0449: result += "shch"; break;
                case 0x044A: break; // Твердый знак
                case 0x044B: result += "y"; break;
                case 0x044C: break; // Мягкий знак
                case 0x044D: result += "e"; break;
                case 0x044E: result += "yu"; break;
                case 0x044F: result += "ya"; break;
                default: result += " "; break;
            }
        } else {
            // Другие многобайтовые символы (например кавычки-елочки)
            if (c == 0xC2 && i + 1 < utf8Str.length()) {
                uint8_t c2 = (uint8_t)utf8Str[i + 1];
                if (c2 == 0xAB || c2 == 0xBB) { // « или »
                    result += "\"";
                    i += 2;
                    continue;
                }
            }
            i++;
        }
    }
    return result;
}
