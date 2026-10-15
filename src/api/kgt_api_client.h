#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "types.h"

class KgtApiClient {
public:
    /**
     * @brief Конструктор API клиента
     * @param apiUrl URL эндпоинта RPC API
     * @param sid Сессионный идентификатор
     * @param timeoutMs Сетевой таймаут запроса в миллисекундах
     */
    explicit KgtApiClient(
        const String& apiUrl = "https://mu-kgt.ru/informing/api/rpc.php",
        const String& sid = "",
        unsigned long timeoutMs = 10000
    );

    /**
     * @brief Запросить прогноз прибытия транспорта для заданной остановки
     * @param stopId Идентификатор остановки (например, "646")
     * @param retryOnSessionError Автоматически запросить новый SID при ошибке сессии
     * @return StopArrivalsResult структура с прогнозами и метаданными
     */
    StopArrivalsResult getStopArrivals(const String& stopId, bool retryOnSessionError = true);

    /**
     * @brief Запросить у сервера новый идентификатор сессии (SID) через метод startSession
     * @return true если сессия успешно создана и SID получен
     */
    bool startSession();

    String getSid() const { return _sid; }

    void setApiUrl(const String& url) { _apiUrl = url; }
    void setSid(const String& sid) { _sid = sid; }
    void setTimeout(unsigned long timeoutMs) { _timeoutMs = timeoutMs; }

private:
    String _apiUrl;
    String _sid;
    unsigned long _timeoutMs;

    String buildRequestPayload(const String& stopId) const;
    static int calculateMinutesRemaining(const String& sysTime, const String& arrTime);
};
