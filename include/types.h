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
