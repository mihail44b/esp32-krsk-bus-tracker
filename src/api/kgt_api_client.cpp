#include "kgt_api_client.h"

KgtApiClient::KgtApiClient(const String& apiUrl, const String& sid, unsigned long timeoutMs)
    : _apiUrl(apiUrl), _sid(sid), _timeoutMs(timeoutMs) {}

String KgtApiClient::buildRequestPayload(const String& stopId) const {
    JsonDocument doc;
    doc["jsonrpc"] = "2.0";
    doc["method"] = "getStopArrive";
    JsonObject params = doc["params"].to<JsonObject>();
    params["sid"] = _sid;
    params["st_id"] = stopId;
    doc["id"] = 1;

    String payload;
    serializeJson(doc, payload);
    return payload;
}

int KgtApiClient::calculateMinutesRemaining(const String& sysTime, const String& arrTime) {
    int sysH = 0, sysM = 0;
    int arrH = 0, arrM = 0;

    if (sscanf(sysTime.c_str(), "%d:%d", &sysH, &sysM) >= 2 &&
        sscanf(arrTime.c_str(), "%d:%d", &arrH, &arrM) >= 2) {
        int sysTotalMinutes = sysH * 60 + sysM;
        int arrTotalMinutes = arrH * 60 + arrM;
        int diff = arrTotalMinutes - sysTotalMinutes;

        // Обработка перехода через полночь (например, системное 23:55, прибытие 00:05)
        if (diff < -720) {
            diff += 1440;
        }

        return (diff >= 0) ? diff : 0;
    }

    return -1;
}

bool KgtApiClient::startSession() {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(_timeoutMs / 1000);

    HTTPClient http;
    http.setTimeout(_timeoutMs);

    if (!http.begin(client, _apiUrl)) {
        Serial.println("[KGT API] Ошибка: не удалось открыть HTTPS соединение для startSession");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Requested-With", "XMLHttpRequest");
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    JsonDocument doc;
    doc["jsonrpc"] = "2.0";
    doc["method"] = "startSession";
    doc["params"].to<JsonObject>();
    doc["id"] = 1;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpCode = http.POST(requestBody);
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[KGT API] startSession HTTP ошибка: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
        http.end();
        return false;
    }

    String responsePayload = http.getString();
    http.end();

    JsonDocument resDoc;
    DeserializationError error = deserializeJson(resDoc, responsePayload);
    if (error) {
        Serial.printf("[KGT API] startSession: ошибка парсинга JSON: %s\n", error.c_str());
        return false;
    }

    if (!resDoc["result"]["sid"].is<const char*>()) {
        Serial.println("[KGT API] startSession: ответ не содержит sid!");
        return false;
    }

    _sid = resDoc["result"]["sid"].as<String>();
    Serial.printf("[KGT API] Успешно получен новый SID сессии: %s\n", _sid.c_str());
    return true;
}

StopArrivalsResult KgtApiClient::getStopArrivals(const String& stopId, bool retryOnSessionError) {
    StopArrivalsResult result;
    result.success = false;
    result.httpStatusCode = -1;

    // Если SID еще не получен, запрашиваем его у сервера
    if (_sid.length() == 0) {
        if (!startSession()) {
            result.errorMessage = "Не удалось инициализировать сессию (startSession)";
            return result;
        }
    }

    WiFiClientSecure client;
    // Отключаем строгую проверку CA сертификата для легковесного подключения к серверу
    client.setInsecure();
    client.setTimeout(_timeoutMs / 1000);

    HTTPClient http;
    http.setTimeout(_timeoutMs);

    if (!http.begin(client, _apiUrl)) {
        result.errorMessage = "Не удалось инициализировать HTTPS соединение";
        return result;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Requested-With", "XMLHttpRequest");
    http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");

    String requestBody = buildRequestPayload(stopId);
    int httpCode = http.POST(requestBody);
    result.httpStatusCode = httpCode;

    if (httpCode != HTTP_CODE_OK) {
        result.errorMessage = "HTTP ошибка: " + String(httpCode) + " (" + http.errorToString(httpCode) + ")";
        http.end();
        return result;
    }

    String responsePayload = http.getString();
    http.end();

    if (responsePayload.length() == 0) {
        result.errorMessage = "Получен пустой ответ от сервера";
        return result;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, responsePayload);

    if (error) {
        result.errorMessage = "Ошибка парсинга JSON: " + String(error.c_str());
        return result;
    }

    // Проверяем наличие ошибки JSON-RPC
    if (doc["error"].is<JsonObject>()) {
        String msg = doc["error"]["message"] | "Неизвестная ошибка";

        // Если сессия протухла, автоматически обновляем SID и повторяем запрос
        if (retryOnSessionError && (msg.indexOf("Session") >= 0 || msg.indexOf("session") >= 0)) {
            Serial.println("[KGT API] Сессия устарела (Session error). Автоматически обновляем SID...");
            if (startSession()) {
                return getStopArrivals(stopId, false);
            }
        }

        result.errorMessage = "RPC ошибка: " + msg;
        return result;
    }

    // Обрабатываем результат
    if (!doc["result"].is<JsonArray>()) {
        result.errorMessage = "Ответ не содержит массива result";
        return result;
    }

    JsonArray arr = doc["result"].as<JsonArray>();
    result.predictions.reserve(arr.size());

    for (JsonObject item : arr) {
        ArrivalPrediction prediction;
        prediction.routeId = item["mr_id"] | "";
        prediction.routeNumber = item["mr_num"] | "";
        prediction.raceTypeRaw = item["rl_racetype"] | "";
        prediction.vehicleType = parseVehicleType(prediction.raceTypeRaw);
        prediction.systemTime = item["tc_systime"] | "";
        prediction.arrivalTime = item["tc_arrivetime"] | "";
        prediction.firstStation = item["firststation_title"] | "";
        prediction.lastStation = item["laststation_title"] | "";
        prediction.minutesRemaining = calculateMinutesRemaining(prediction.systemTime, prediction.arrivalTime);

        if (result.systemTime.length() == 0 && prediction.systemTime.length() > 0) {
            result.systemTime = prediction.systemTime;
        }

        result.predictions.push_back(prediction);
    }

    result.success = true;
    return result;
}
