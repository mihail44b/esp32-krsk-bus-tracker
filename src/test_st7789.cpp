#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// =============================================================================
// Назначение пинов дисплея ST7789 (VSPI для ESP32)
// =============================================================================
#define TFT_CS    5    // Chip Select
#define TFT_DC    2    // Data / Command
#define TFT_RST   4    // Reset
#define TFT_MOSI  23   // SPI Data (SDA на дисплее)
#define TFT_SCLK  18   // SPI Clock (SCL на дисплее)

#define SCREEN_WIDTH   170
#define SCREEN_HEIGHT  320

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void hardResetDisplay() {
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(50);
    digitalWrite(TFT_RST, LOW);
    delay(100);
    digitalWrite(TFT_RST, HIGH);
    delay(200);
}

void drawTestPattern() {
    // 1. Очистка экрана
    tft.fillScreen(ST77XX_BLACK);

    // 2. Рамка по периметру
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
    tft.drawRect(1, 1, tft.width() - 2, tft.height() - 2, ST77XX_CYAN);

    // 3. Шапка
    tft.fillRect(4, 4, tft.width() - 8, 30, tft.color565(30, 100, 220));
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(12, 11);
    tft.println("KRSK BUS TRACKER");

    // 4. Информация
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 42);
    tft.printf("ST7789 IPS 1.9\" | %dx%d px", tft.width(), tft.height());

    // 5. Цветные плашки
    int boxW = 50;
    int boxH = 20;
    int startY = 58;

    tft.fillRect(10, startY, boxW, boxH, ST77XX_RED);
    tft.setCursor(18, startY + 6);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("RED");

    tft.fillRect(66, startY, boxW, boxH, ST77XX_GREEN);
    tft.setCursor(72, startY + 6);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("GREEN");

    tft.fillRect(122, startY, boxW, boxH, ST77XX_BLUE);
    tft.setCursor(132, startY + 6);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("BLUE");

    tft.fillRect(178, startY, boxW, boxH, ST77XX_MAGENTA);
    tft.setCursor(184, startY + 6);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("MAGENTA");

    tft.fillRect(234, startY, boxW, boxH, ST77XX_ORANGE);
    tft.setCursor(240, startY + 6);
    tft.setTextColor(ST77XX_BLACK);
    tft.print("ORANGE");

    // 6. Карточки автобусов
    int cardY = 86;
    tft.fillRoundRect(10, cardY, tft.width() - 20, 32, 6, tft.color565(35, 38, 48));
    tft.fillRoundRect(12, cardY + 2, 45, 28, 4, tft.color565(220, 60, 40));
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(22, cardY + 8);
    tft.print("63");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(65, cardY + 6);
    tft.print("Akademgorodok");
    tft.setTextColor(tft.color565(160, 160, 160));
    tft.setCursor(65, cardY + 18);
    tft.print("Solnechnyj");

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(tft.width() - 75, cardY + 8);
    tft.print("~4m");

    cardY = 124;
    tft.fillRoundRect(10, cardY, tft.width() - 20, 32, 6, tft.color565(35, 38, 48));
    tft.fillRoundRect(12, cardY + 2, 45, 28, 4, tft.color565(40, 140, 220));
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(22, cardY + 8);
    tft.print("85");

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(65, cardY + 6);
    tft.print("Verhnie Cheremushki");
    tft.setTextColor(tft.color565(160, 160, 160));
    tft.setCursor(65, cardY + 18);
    tft.print("Vetchinka");

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(tft.width() - 85, cardY + 8);
    tft.print("~12m");
}

void initScreen() {
    hardResetDisplay();

    // Явная инициализация шины SPI
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

    // Запуск ST7789 с пониженной частотой SPI (4 МГц) для устойчивости к неидеальным контактам
    tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);
    tft.setSPISpeed(4000000); // 4 MHz вместо 27-40 MHz
    tft.setRotation(1);

    drawTestPattern();
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n[ST7789] Инициализация с защитой от помех (4 MHz SPI)...");
    initScreen();
}

static unsigned long lastRefresh = 0;
static int loopCount = 0;

void loop() {
    unsigned long now = millis();

    // Переинициализируем экран каждые 2.5 секунды!
    // Это ключевой момент для неприпаянной гребенки:
    // если при шевелении питание на секунду пропало и контроллер ушел в сон,
    // он автоматически перезапустится при следующем цикле, как только вы прижмете контакты!
    if (now - lastRefresh >= 2500) {
        lastRefresh = now;
        loopCount++;

        Serial.printf("[ST7789] Цикл авто-переинициализации #%d...\n", loopCount);
        initScreen();
    }

    delay(50);
}
