#pragma once

/**
 * @file config.example.h
 * @brief Шаблон конфигурации проекта.
 * 
 * Скопируйте этот файл в `include/config.h` и укажите свои параметры:
 *   cp include/config.example.h include/config.h
 * 
 * Файл `include/config.h` добавлен в .gitignore и не попадет в репозиторий.
 */

// =============================================================================
// Wi-Fi Configuration
// =============================================================================
#define WIFI_SSID             "YOUR_WIFI_SSID"
#define WIFI_PASSWORD         "YOUR_WIFI_PASSWORD"
#define WIFI_CONNECT_TIMEOUT  20000   // Таймаут первой попытки подключения (мс)
#define WIFI_RECONNECT_DELAY  10000   // Интервал повторных попыток при обрыве (мс)

// =============================================================================
// МКУ «Красноярскгортранс» API
// =============================================================================
#define KGT_API_URL           "https://mu-kgt.ru/informing/api/rpc.php"
#define KGT_API_SID           "8ED6BFEA-3721-4F24-BDDB-F55EC31D3EE6"
#define KGT_STOP_ID           "646"   // ID остановки (646 — Академгородок)

// =============================================================================
// App Settings
// =============================================================================
#define UPDATE_INTERVAL_MS    30000   // Период опроса API (мс, рекомендуется >= 20-30 сек)
#define HTTP_TIMEOUT_MS       10000   // Таймаут сетевого запроса к API (мс)
#define SERIAL_BAUD_RATE      115200  // Скорость Serial монитора
