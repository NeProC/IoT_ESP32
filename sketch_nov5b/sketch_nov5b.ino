#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include <Wire.h>
//#include <Adafruit_BME280.h>
//#include <Adafruit_Sensor.h>

//================================================= Библиотеки для ds18b20
#include <OneWire.h> //---------------
#include <DallasTemperature.h> //----------
#define ONE_WIRE_BUS 15//------------ Data wire is connected to GPIO15
//=================================================

//#define WIFI_SSID "Wp5"                     //
//#define WIFI_PASSWORD "12511251"
#define WIFI_SSID "zelhome"                     //Домашняя сеть
#define WIFI_PASSWORD "ZelenevBaeva"


#define MQTT_SERVER "dev.rightech.io"                 
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "myESP32Client"

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg1 = 0;
long lastMsg2 = 0;

float press1 = 0.0;
float press2 = 0.0;
float temp1  = 0.0;

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//-----------------------Setup a oneWire instance to communicate with a OneWire device
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor 

void setup() {
  Serial.begin(115200);
  //дисплей
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
//DEBUG    for (;;)
//DEBUG      ;  // Don't proceed, loop forever
  }
  display.display();

  delay(2000);  // Pause for 2 seconds
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void printDisplay(String text, byte x, byte y, byte textsize, byte color) {
  display.clearDisplay();
  display.setTextSize(textsize);
  display.setTextColor(color);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  printDisplay(String("WiFi connected\n IP: ") + WiFi.localIP(), 10, 0, 1, SSD1306_WHITE);
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageString;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageString += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
      // Subscribe
      //client.subscribe("myESP32Client/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float getPressure(byte gpio) {
  return 1.5;
}

// возвращает зачение первого найденного датчика
float getTemperature() {
  float res = -55;  // в этой переменной будет результат
  ds.begin();
  byte ds18b20_count = ds.getDeviceCount();

  ds.requestTemperatures(); // запросили обновить температуру у датчиков
  for (int i = 0; i < ds18b20_count; i++) { // цикл по найденным датчикам. У нас один.
    DeviceAddress tempDeviceAddress;
    if (ds.getAddress(tempDeviceAddress, i)) // если определили его адрес, значит живой датчик
    {
      res = ds.getTempC(tempDeviceAddress); // по найденному адресу запрашиваем его температуру
      break; // цикл можно закончить, т.к. у нас один датчик
    }
  }

  // убедились, что значение с датчика в нужно диаппазоне от 125 до -55.
  if ((res > 125) or (res < -55)) {
    res = -55; // если не в диаппазоне, то -55 выдаем, ака ошибка.
  }
  return res;

}

void publicTopic(String topic, String msg) {
  client.publish(topic.c_str(), msg.c_str());
  Serial.print("SEND TOPIC: ");
  Serial.print(topic);
  Serial.print(" MESSAGE: ");
  Serial.println(msg);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg1 > 10000) {  // шлем топики в mqtt раз в 10 секунд
    lastMsg1 = now;

    // Temperature in Celsius
    publicTopic("myESP32Client/sensors/temp1", String(temp1, 2));
    publicTopic("myESP32Client/sensors/press1", String(press1, 2));
    publicTopic("myESP32Client/sensors/press2", String(press2, 2));
  }

  if (now - lastMsg2 > 1000) {  // обновляем датчики раз в секунду
    lastMsg2 = now;
    temp1 = getTemperature();
    press1 = getPressure(35);
    press2 = getPressure(34);
    Serial.print("Temp: ");
    Serial.println(temp1);
    Serial.print("Pressure1: ");
    Serial.println(press1);
    Serial.print("Pressure2: ");
    Serial.println(press2);
  }
//-----------------------Определение адреса DS18B20
// ROM = 28 D0 E E0 F 0 0 6E        1ый датчик (зеленый)
// ROM = 28 3E 8A DF F 0 0 52       2ой датчик (синий)
/*  byte i;
  byte addr[8];
  
  if (!ds.search(addr)) {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }*/
//----------------------  
}
