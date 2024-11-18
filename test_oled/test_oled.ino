#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define MOSFET_PIN 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void enableOled(bool st) {
  digitalWrite(MOSFET_PIN, st);
  //  digitalWrite(MOSFET_PIN, !st); //либо такое, если будет обратный эффект
}

void setup() {
  pinMode(MOSFET_PIN, OUTPUT);
  enableOled(true);

  Serial.begin(115200);
  //дисплей
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
    ;
    // Don't proceed, loop forever
  }
  display.display();

  delay(2000);  // Pause for 2 seconds
}

void printDisplay(String text, byte x, byte y, byte textsize, byte color) {
  display.clearDisplay();
  display.setTextSize(textsize);
  display.setTextColor(color);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}




void loop() {
  enableOled(true);
  printDisplay(String("helllooo"), 10, 0, 1, SSD1306_WHITE);
  delay(5000);
  enableOled(false);
  delay(5000);

}
