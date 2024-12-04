#define TINY_GSM_MODEM_SIM800

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

#include <HardwareSerial.h>

HardwareSerial SerialAT(2);  // define a Serial for UART2

long lastMsg_maintimer = 0;

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
//#define TINY_GSM_DEBUG SerialMon

// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "";
const char gprsUser[] = "";
const char gprsPass[] = "";

// MQTT details
//const char* broker = "dev.rightech.io";
const char* broker = "smarthut.online";

/*
const char* topicLed       = "GsmClientTest/led";
const char* topicInit      = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";
*/

#include <TinyGsmClient.h>
#include <PubSubClient.h>


TinyGsm modem(SerialAT);

TinyGsmClient client(modem);
PubSubClient mqtt(client);

uint32_t lastReconnectAttempt = 0;

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  //  boolean status = mqtt.connect("myESP32Client");
  boolean status = mqtt.connect("myESP32Client", "sensor", "sensor");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  //mqtt.publish(topicInit, "GsmClientTest started");
  //mqtt.subscribe(topicLed);
  return mqtt.connected();
}


void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  SerialMon.println("Wait...");
  SerialAT.begin(9600, SERIAL_8N1, 16, 17);
  // Set GSM module baud rate
  // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  // SerialAT.begin(9600);
  // delay(6000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) { modem.simUnlock(GSM_PIN); }

  modem.gprsConnect(apn, gprsUser, gprsPass);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(5000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(5000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) { SerialMon.println("GPRS connected"); }

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  //mqtt.setCallback(mqttCallback);
}

void loop() {
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
    SerialMon.println("Network disconnected");
    if (!modem.waitForNetwork(180000L, true)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected()) {
      SerialMon.println("Network re-connected");
    }

    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      SerialMon.println("GPRS disconnected!");
      SerialMon.print(F("Connecting to "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) { SerialMon.println("GPRS reconnected"); }
    }
  }

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) { lastReconnectAttempt = 0; }
    }
    delay(1000);
    return;
  } else {
    long now = millis();

    if ((now - lastMsg_maintimer > 100 * 1000) or (lastMsg_maintimer == 0)) {
      lastMsg_maintimer = now;
      Serial.println("MAIN TIMER");
      publicTopic("sensor/111111/temp1","12.2");
    }
  }

  mqtt.loop();
}


void publicTopic(String topic, String msg) {
  mqtt.publish(topic.c_str(), msg.c_str());
  Serial.print("SEND TOPIC: ");
  Serial.print(topic);
  Serial.print(" MESSAGE: ");
  Serial.println(msg);
}