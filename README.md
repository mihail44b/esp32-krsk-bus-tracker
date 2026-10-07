# ESP32 Krasnoyarsk Bus Tracker 🚎 ⏱️

Физическое табло реального времени на базе **ESP32 DevKit v1**, отображающее онлайн-прогноз прибытия городского транспорта (автобусов, троллейбусов, трамваев) в городе Красноярске по данным бортовых навигаторов ГЛОНАСС.

Проект разработан в редакторе **Zed** с использованием инструментария **PlatformIO Core (CLI)** на базе **Arduino Framework**.

---

## 📑 Оглавление
- [Возможности](#возможности)
- [Архитектура проекта](#архитектура-проекта)
- [Спецификация API](#спецификация-api)
- [Установка и быстрый старт](#установка-и-быстрый-старт)
  - [1. Установка PlatformIO Core (CLI)](#1-установка-platformio-core-cli)
  - [2. Настройка конфигурации](#2-настройка-конфигурации)
  - [3. Сборка и прошивка](#3-сборка-и-прошивка)
  - [4. Интеграция с редактором Zed](#4-интеграция-с-редактором-zed)
- [Подключение дисплеев (Roadmap)](#подключение-дисплеев-roadmap)

---

## ✨ Возможности
- 🛰️ **Прямой запрос к API МКУ «Красноярскгортранс»** без сторонних прокси-серверов и облаков.
- ⚡ **Легковесный парсинг JSON-RPC 2.0** на базе `ArduinoJson v7`.
- 🕒 **Расчет времени ожидания**: автоматический расчет остатка минут (`~12 мин`) с учетом перехода через полночь.
- 📶 **Отказоустойчивый Wi-Fi менеджер**: авто-переподключение в фоновом режиме без зависания основного цикла.
- 🖥️ **Абстрактный интерфейс дисплея (`DisplayBase`)**: вывод в Serial Monitor из коробки, полная готовность к подключению OLED (SSD1306/SH1106), LCD или матричных дисплеев.

---

## 🏗️ Архитектура проекта

```text
esp32-krsk-bus-tracker/
├── .gitignore                      # Исключение сборочных файлов и config.h
├── .zed/
│   └── tasks.json                  # Готовые задачи сборки/прошивки для редактора Zed
├── include/
│   ├── config.example.h            # Шаблон конфигурации (Wi-Fi, ID остановки)
│   ├── config.h                    # Локальный файл конфигурации (в gitignore)
│   └── types.h                     # C++ структуры данных прогнозов и типов ТС
├── src/
│   ├── main.cpp                    # Точка входа, оркестрация сервисов
│   ├── api/
│   │   ├── kgt_api_client.h        # Заголовок клиента API
│   │   └── kgt_api_client.cpp      # HTTP/SSL клиент, формирование JSON-RPC, парсинг
│   ├── display/
│   │   ├── display_base.h          # Абстрактный базовый класс дисплея (IDisplay)
│   │   ├── serial_display.h        # Реализация вывода в Serial Monitor
│   │   └── serial_display.cpp      # Форматирование текстового табло
│   └── network/
│       ├── wifi_manager.h          # Заголовок Wi-Fi менеджера
│       └── wifi_manager.cpp        # Подключение и неблокирующий авто-реконнект
├── platformio.ini                  # Конфигурация сборки PlatformIO
└── README.md
```

### Разделение ответственности:
- **`include/types.h`**: Независимые структуры данных (`ArrivalPrediction`, `VehicleType`, `StopArrivalsResult`).
- **`src/network/`**: Изолированная сетевая подсистема, скрывающая специфику `WiFi.h`.
- **`src/api/`**: Инкапсуляция работы с JSON-RPC протоколом Гортранса через `HTTPClient` и `WiFiClientSecure`.
- **`src/display/`**: Полиморфный интерфейс `DisplayBase`. `main.cpp` работает только с указателем `std::unique_ptr<DisplayBase>`, что позволяет менять дисплей одной строчкой.

---

## 📡 Спецификация API (МКУ «Красноярскгортранс»)
- **URL:** `POST https://mu-kgt.ru/informing/api/rpc.php`
- **Заголовки:**
  - `Content-Type: application/json`
  - `X-Requested-With: XMLHttpRequest`
  - `User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64)`
- **Тело запроса:**
  ```json
  {
    "jsonrpc": "2.0",
    "method": "getStopArrive",
    "params": {
      "sid": "8ED6BFEA-3721-4F24-BDDB-F55EC31D3EE6",
      "st_id": "646"
    },
    "id": 1
  }
  ```
  *(где `st_id` — ID остановки, например `646` — Академгородок)*.

---

## 🚀 Установка и быстрый старт

### 1. Установка PlatformIO Core (CLI)
Если PlatformIO CLI еще не установлен, самый простой способ — через Python:
```bash
pip install platformio
```
Проверьте установку:
```bash
pio --version
```

### 2. Настройка конфигурации
Скопируйте файл шаблона `include/config.example.h` в `include/config.h`:
```bash
# Windows PowerShell:
Copy-Item include/config.example.h include/config.h

# Linux / macOS / bash:
cp include/config.example.h include/config.h
```

Откройте `include/config.h` и укажите данные вашей сети и нужную остановку:
```cpp
#define WIFI_SSID     "Ваш_WiFi"
#define WIFI_PASSWORD "Ваш_Пароль"
#define KGT_STOP_ID   "646" // ID нужной остановки
```

### 3. Сборка и прошивка через терминал
1. **Собрать проект:**
   ```bash
   pio run
   ```
2. **Прошить ESP32:**
   ```bash
   pio run -t upload
   ```
3. **Открыть монитор порта (115200 бод):**
   ```bash
   pio device monitor
   ```
4. **Прошить и сразу запустить монитор (одним действием):**
   ```bash
   pio run -t upload -t monitor
   ```

### 4. Интеграция с редактором Zed
В проекте настроен конфигурационный файл `.zed/tasks.json`.
1. Откройте проект в Zed: `zed .`
2. Нажмите `Ctrl+Shift+P` (или `Cmd+Shift+P` на macOS) и введите `task: spawn` (или сочетание `Ctrl+Alt+T`).
3. Выберите нужную задачу:
   - `PlatformIO: Build`
   - `PlatformIO: Upload`
   - `PlatformIO: Monitor`
   - `PlatformIO: Upload & Monitor`
   - `PlatformIO: Generate compile_commands.json (Clangd LSP)`

> **💡 Подсказка для идеального автодополнения (LSP/clangd) в Zed:**
> Запустите задачу `PlatformIO: Generate compile_commands.json` (команда `pio run -t compiledb`). PlatformIO сгенерирует базу данных компиляции, и языковой сервер Zed (`clangd`) мгновенно подхватит все заголовочные файлы ESP-IDF и Arduino!

---

## 📺 Подключение дисплеев (Roadmap)
Благодаря интерфейсу `DisplayBase`, добавление физического экрана не требует изменения сетевой или API логики:
1. Создать класс, например `OledDisplay : public DisplayBase` (используя библиотеку `U8g2` или `Adafruit_SSD1306`).
2. Реализовать методы `init()`, `showLoading()`, `showArrivals(...)` и `showError(...)`.
3. В `src/main.cpp` заменить инициализацию:
   ```cpp
   display = std::unique_ptr<DisplayBase>(new OledDisplay());
   ```

Лицензия: MIT.
