#include <Arduino.h>
#include <memory>

#if __has_include("config.h")
    #include "config.h"
#elif __has_include("config.example.h")
    #warning "Внимание: используется config.example.h! Создайте include/config.h со своими учетными данными Wi-Fi."
    #include "config.example.h"
#else
    #error "Файл конфигурации не найден! Скопируйте include/config.example.h в include/config.h"
#endif

#include "network/wifi_manager.h"
#include "api/kgt_api_client.h"
#include "display/serial_display.h"

// Сервисные компоненты
static WiFiManager wifiManager;
static KgtApiClient apiClient(KGT_API_URL, KGT_API_SID, HTTP_TIMEOUT_MS);
static std::unique_ptr<DisplayBase> display;

// Таймеры
static unsigned long lastUpdateMillis = 0;
static const unsigned long updateInterval = UPDATE_INTERVAL_MS;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    Serial.println("\n========================================================");
    Serial.println("   ESP32 Krasnoyarsk Bus Tracker — Инициализация");
    Serial.println("========================================================");

    // Инициализация интерфейса вывода
    display = std::unique_ptr<DisplayBase>(new SerialDisplay(SERIAL_BAUD_RATE));
    display->init();

    // Подключение к Wi-Fi
    display->showConnecting(WIFI_SSID);
    bool connected = wifiManager.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CONNECT_TIMEOUT);

    if (connected) {
        display->showConnected(wifiManager.getIP());
    } else {
        display->showError("Не удалось подключиться к Wi-Fi при запуске. Переход в фоновый поиск сети...");
    }
}

void loop() {
    // Фоновое поддержание Wi-Fi соединения (авто-реконнект)
    wifiManager.update();

    if (!wifiManager.isConnected()) {
        delay(200);
        return;
    }

    // Проверка интервала обновления данных
    unsigned long now = millis();
    if (lastUpdateMillis == 0 || (now - lastUpdateMillis >= updateInterval)) {
        lastUpdateMillis = now;

        display->showLoading();

        StopArrivalsResult result = apiClient.getStopArrivals(KGT_STOP_ID);

        if (result.success) {
            if (result.predictions.empty()) {
                display->showEmpty(KGT_STOP_ID, result.systemTime);
            } else {
                display->showArrivals(KGT_STOP_ID, result.systemTime, result.predictions);
            }
        } else {
            display->showError(result.errorMessage);
        }
    }

    delay(100); // Даем отработать фоновым задачам FreeRTOS/ESP-IDF
}
