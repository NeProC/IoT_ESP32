#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//#include <Wire.h>
//#include <Adafruit_BME280.h>
//#include <Adafruit_Sensor.h>

#define WIFI_SSID "Wp5"                     //
#define WIFI_PASSWORD "12511251"


#define MQTT_SERVER "dev.rightech.io"                 
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "myESP32Client"

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg1 = 0;
long lastMsg2 = 0;

float press1 = 0;
float press2 = 0;
float temp1 = -40;

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

float getTemperature() {
  return 22.3;
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
}
