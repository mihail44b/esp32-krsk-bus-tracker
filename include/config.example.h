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

// =============================================================================
// Hardware Display Settings (ST7789 1.9" IPS 170x320 SPI)
// =============================================================================
#define USE_ST7789            1       // 1 = Использовать цветной IPS ST7789, 0 = отключено
#define TFT_CS_PIN            5       // CS
#define TFT_DC_PIN            2       // DC
#define TFT_RST_PIN           4       // RST
#define TFT_MOSI_PIN          23      // SDA (MOSI)
#define TFT_SCLK_PIN          18      // SCL (SCK)
#define TFT_PAGE_ROTATE_MS    5000    // Период смены страниц прибывающего транспорта (мс)

// =============================================================================
// Hardware Display Settings (LCD 1602 I2C)
// =============================================================================
#define USE_LCD1602           0       // 1 = Использовать LCD 1602, 0 = только Serial
#define LCD_I2C_ADDR          0x27    // Обычно 0x27 или 0x3F (поддерживается авто-сканирование)
#define LCD_COLS              16      // Количество символов в строке
#define LCD_ROWS              2       // Количество строк
#define LCD_SDA_PIN           21      // SDA пин (GPIO 21 для ESP32 DevKit v1)
#define LCD_SCL_PIN           22      // SCL пин (GPIO 22 для ESP32 DevKit v1)
#define LCD_PAGE_ROTATE_MS    4000    // Период смены страниц прибывающих автобусов (мс)

