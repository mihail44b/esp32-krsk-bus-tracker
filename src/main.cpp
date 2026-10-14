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

#include "storage/settings_manager.h"
#include "network/wifi_manager.h"
#include "api/kgt_api_client.h"
#include "display/serial_display.h"
#include "web/web_server_manager.h"

// Сервисные компоненты
static SettingsManager settingsManager;
static WiFiManager wifiManager;
static KgtApiClient apiClient(KGT_API_URL, KGT_API_SID, HTTP_TIMEOUT_MS);
static std::unique_ptr<DisplayBase> display;
static WebServerManager webServer;

// Таймеры
static unsigned long lastUpdateMillis = 0;
static const unsigned long updateInterval = UPDATE_INTERVAL_MS;

// Настройки аппаратной кнопки сброса BOOT (GPIO 0)
#ifndef BUTTON_BOOT_PIN
#define BUTTON_BOOT_PIN 0
#endif

static unsigned long bootButtonPressStart = 0;
static bool bootButtonHeld = false;

static void checkBootButton() {
    if (digitalRead(BUTTON_BOOT_PIN) == LOW) {
        if (!bootButtonHeld) {
            bootButtonHeld = true;
            bootButtonPressStart = millis();
        } else if (millis() - bootButtonPressStart >= 3000) {
            Serial.println("\n[RESET] ⚠️ Кнопка BOOT удерживалась 3 секунды!");
            Serial.println("[RESET] Сброс настроек Wi-Fi и переход в режим первоначальной настройки...");
            if (display) {
                display->showError("Сброс Wi-Fi... Подключитесь к 'KRSK-Bus-Setup'");
            }
            settingsManager.clear();
            delay(1500);
            ESP.restart();
        }
    } else {
        bootButtonHeld = false;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    // Настройка кнопки BOOT (GPIO 0 на devkit плате)
    pinMode(BUTTON_BOOT_PIN, INPUT_PULLUP);

    Serial.println("\n========================================================");
    Serial.println("   ESP32 Krasnoyarsk Bus Tracker — Инициализация");
    Serial.println("========================================================");

    // Инициализация интерфейса вывода
    display = std::unique_ptr<DisplayBase>(new SerialDisplay(SERIAL_BAUD_RATE));
    display->init();

    // Загрузка настроек из Flash памяти NVS (с откатом на config.h при первом запуске)
    settingsManager.begin(WIFI_SSID, WIFI_PASSWORD, KGT_STOP_ID);
    String activeSsid = settingsManager.getSSID();
    String activePass = settingsManager.getPassword();
    String activeStop = settingsManager.getStopId();

    bool hasConfig = settingsManager.hasConfig();
    if (hasConfig) {
        display->showConnecting(activeSsid);
    } else {
        display->showError("Wi-Fi не настроен. Подключитесь к 'KRSK-Bus-Setup'");
    }

    // Подключение к Wi-Fi и запуск точки доступа Captive Portal
    bool connected = wifiManager.begin(activeSsid.c_str(), activePass.c_str(), WIFI_AP_SSID, WIFI_CONNECT_TIMEOUT);

    // Веб-сервер и портал настроек доступны сразу (на http://bus.local и http://192.168.4.1)
    webServer.begin(&settingsManager, &wifiManager);

    if (connected) {
        display->showConnected(wifiManager.getIP());
        Serial.println("\n========================================================");
        Serial.println("   📱 ОНЛАЙН-ТАБЛО И ПОРТАЛ НАСТРОЕК ГОТОВЫ К РАБОТЕ!");
        Serial.println("========================================================");
        Serial.printf(" 1. В браузере (iPhone, Android, PC, Mac):\n");
        Serial.printf("    👉 http://bus.local\n");
        Serial.printf("    👉 http://%s\n", wifiManager.getIP().c_str());
        Serial.println("\n 2. Для смены Wi-Fi или остановки в браузере:");
        Serial.println("    👉 http://bus.local/settings\n");
        Serial.println(" 3. Аппаратный сброс Wi-Fi в любой момент:");
        Serial.println("    👉 Удерживайте кнопку BOOT на плате 3 секунды");
        Serial.println("========================================================\n");
    } else {
        Serial.println("\n========================================================");
        Serial.println("   ⚙️ ПОРТАЛ НАСТРОЙКИ WI-FI АКТИВЕН (CAPTIVE PORTAL)");
        Serial.println("========================================================");
        Serial.printf(" 1. Подключитесь с телефона к Wi-Fi: '%s'\n", WIFI_AP_SSID);
        Serial.printf(" 2. Браузер откроется автоматически или перейдите на:\n");
        Serial.printf("    👉 http://192.168.4.1/settings\n");
        Serial.println(" 3. Выберите домашнюю сеть, введите пароль и сохраните!");
        Serial.println("========================================================\n");
        display->showError("Подключитесь к Wi-Fi 'KRSK-Bus-Setup' -> 192.168.4.1");
    }
}

void loop() {
    // Проверка удержания кнопки BOOT для сброса Wi-Fi
    checkBootButton();

    // Фоновое поддержание Wi-Fi соединения (авто-реконнект)
    wifiManager.update();

    // Обслуживание веб-сервера и портала Captive Portal (работает ВСЕГДА)
    webServer.update();

    if (!wifiManager.isConnected()) {
        delay(20);
        return;
    }

    // Проверка интервала обновления данных расписания
    unsigned long now = millis();
    if (lastUpdateMillis == 0 || (now - lastUpdateMillis >= updateInterval)) {
        lastUpdateMillis = now;

        display->showLoading();

        String currentStop = settingsManager.getStopId();
        StopArrivalsResult result = apiClient.getStopArrivals(currentStop);
        webServer.setArrivals(result);

        if (result.success) {
            if (result.predictions.empty()) {
                display->showEmpty(currentStop, result.systemTime);
            } else {
                display->showArrivals(currentStop, result.systemTime, result.predictions);
            }
        } else {
            display->showError(result.errorMessage);
        }
    }

    delay(20); // Небольшая пауза для отзывчивости веб-сервера
}
