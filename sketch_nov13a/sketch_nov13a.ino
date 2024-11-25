#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OneWire.h"
#include "DallasTemperature.h"

#define N_CONSTANT 2           // разница давлений для выполнения условия

#define S_To_uS_Factor 1000000ULL  //Conversion factor for micro seconds to seconds
#define MEASURE_INTERVAL 10     //Time ESP32 will go to sleep (in seconds) \
                               //интервал чтения датчиков
#define SEND_WIFI_INTERVAL 600  // интервал отправки данных по wifi, если ничего не происзодит

RTC_DATA_ATTR int bootCount = 0;


#define GERKON_PIN 4         // пин геркона
#define GERKON_LOGIC_ON LOW  // логический уровень для включения экрана

#define PRESS_PIN1 35
#define PRESS_PIN2 36

//#define WIFI_SSID "Wp5"                                           //
//#define WIFI_PASSWORD "12511251"
//#define WIFI_SSID "zelhome"                                       //Домашняя сеть
//#define WIFI_PASSWORD "ZelenevBaeva"
//#define WIFI_SSID "ORT1"                                          //МЭИ
//#define WIFI_PASSWORD "84953627044"
#define WIFI_SSID "MF1440_GUEST"  //БЮРО
#define WIFI_PASSWORD "Mf144020997479"

#define MQTT_SERVER "dev.rightech.io"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "myESP32Client"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(15);             //шина 1-wire к gpio15
DallasTemperature ds(&oneWire);  // инициализация объекта для ds18b20

float press1 = 0.0;
float press2 = 0.0;
float temp1 = 0.0;


void setup() {
  Serial.begin(115200);
  pinMode(GERKON_PIN, INPUT_PULLUP);
  bootCount++;
}

void loop() {

  temp1 = getTemperature();
  press1 = getPressure(PRESS_PIN1);
  press2 = getPressure(PRESS_PIN2);

  Serial.print("Temp: ");
  Serial.println(temp1);
  Serial.print("Pressure1: ");
  Serial.println(press1);
  Serial.print("Pressure2: ");
  Serial.println(press2);

  // проверка давления P1>=N*P2
  if (press1 >= press2 * N_CONSTANT) {
    // если условие выполнилось, что подключаемся к wifi и шлем данные на сервер
    Serial.print("SEND WIFI BY PRESS !!!");
    bootCount = 0;
    sendWiFi();
  } else {
    if (bootCount * MEASURE_INTERVAL > SEND_WIFI_INTERVAL) {
      Serial.print("SEND WIFI BY TIME!!!");
      bootCount = 0;
      sendWiFi();
    }
  }

  if (digitalRead(GERKON_PIN) == GERKON_LOGIC_ON) {
    //oled ON
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.ssd1306_command(SSD1306_DISPLAYON);
    String todisplay = "Temp: " + String(temp1) + "\n";
    todisplay = todisplay + "Press1: " + String(press1) + "\n";
    todisplay = todisplay + "Press2: " + String(press2);
    printDisplay(todisplay, 10, 0, 1, SSD1306_WHITE);
  } else {
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS,false);
    display.ssd1306_command(SSD1306_DISPLAYOFF);
  }
  esp_sleep_enable_timer_wakeup(MEASURE_INTERVAL * S_To_uS_Factor);
  Serial.println("Setup ESP32 to sleep for every " + String(MEASURE_INTERVAL) + " Seconds");
  //Go to sleep now
  esp_deep_sleep_start();
}


void sendGPRS() {
  return;
}

void sendWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //TODO выход с цикла, если не удалось соединиться
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //printDisplay(String("WiFi connected\n IP: ") + WiFi.localIP(), 10, 0, 1, SSD1306_WHITE);

  Serial.print("Connecting to ");
  Serial.print(MQTT_SERVER);
  //TODO выход с цикла, если не удалось соединиться
  client.setServer(MQTT_SERVER, MQTT_PORT);

  while (!client.connected()) {
    client.connect(MQTT_CLIENT_ID);
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("MQTT connected");

  // Temperature in Celsius
  publicTopic("myESP32Client/sensors/temp1", String(temp1, 2));
  publicTopic("myESP32Client/sensors/press1", String(press1, 2));
  publicTopic("myESP32Client/sensors/press2", String(press2, 2));
  WiFi.disconnect();
}

void printDisplay(String text, byte x, byte y, byte textsize, byte color) {
  display.clearDisplay();
  display.setTextSize(textsize);
  display.setTextColor(color);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}

float getPressure(byte gpio) {
  uint16_t adc = 0;
  for (int i = 0; i < 20; i++) {
    adc += analogRead(gpio);
    delay(5);
  }
  adc = adc / 20;

  float etalon_bar = 10.34214;   // значение бар при etalon_volt
  float etalon_volt = 4.5;       // значение вольт при etalon_bar
  float etalon_zero_volt = 0.5;  // значение вольт при 0 бар
                                 // в adc значение от 0 до 4095, что соответствует 0 вольт до 3.3в
  float current_volt = map(adc, 0, 4095, 0, 3.3);

  //DEBUG float current_volt = 3.3;
  float bar = etalon_bar * (current_volt - etalon_zero_volt) / (etalon_volt - etalon_zero_volt);
  Serial.print("gpio=");
  Serial.print(gpio);
  Serial.print(" adc=");
  Serial.print(adc);
  Serial.print(" volt=");
  Serial.print(current_volt);
  Serial.print(" bar=");
  Serial.println(bar);

  if (bar < 0) return 0;  // что б минуса не было, например когда датчик не подключен

  return bar;
}

float getTemperature()  // возвращает зачение первого найденного датчика
{
  ds.begin();
  byte ds18b20_count = ds.getDeviceCount();
  ds.requestTemperatures();                  // запросили обновить температуру у датчиков
  for (int i = 0; i < ds18b20_count; i++) {  // цикл по найденным датчикам. У нас один.
    DeviceAddress tempDeviceAddress;
    if (ds.getAddress(tempDeviceAddress, i))  // если определили его адрес, значит живой датчик
    {
      temp1 = ds.getTempC(tempDeviceAddress);  // по найденному адресу запрашиваем его температуру
      break;                                   // цикл можно закончить, т.к. у нас один датчик
    }
  }
  return constrain(temp1, -55, 125);
}

void publicTopic(String topic, String msg) {
  client.publish(topic.c_str(), msg.c_str());
  Serial.print("SEND TOPIC: ");
  Serial.print(topic);
  Serial.print(" MESSAGE: ");
  Serial.println(msg);
}
