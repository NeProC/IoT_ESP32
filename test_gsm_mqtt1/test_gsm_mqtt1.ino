#define TINY_GSM_MODEM_SIM800
#include <HardwareSerial.h>

HardwareSerial SIM800(2);  // define a Serial for UART2
//#define TINY_GSM_DEBUG Serial
#define SIM800_RX_PIN 16
#define SIM800_TX_PIN 17
// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[] = "";
const char gprsUser[] = "";
const char gprsPass[] = "";


// MQTT details
//const char* broker = "dev.rightech.io";
const char* broker = "smarthut.online";


#include <TinyGsmClient.h>
#include <PubSubClient.h>


TinyGsm modem(SIM800);

TinyGsmClient client_gsm(modem);
PubSubClient mqtt_gsm(client_gsm);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Wait...");
  SIM800.begin(9600, SERIAL_8N1, SIM800_RX_PIN, SIM800_TX_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("loop");
  sendGPRS();
  delay(20000);
}


void sendGPRS() {
  Serial.println("Initializing modem...");
  modem.restart();
  // modem.init();
  /*
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);
  */
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) { modem.simUnlock(GSM_PIN); }

  //modem.gprsConnect(apn, gprsUser, gprsPass);

  Serial.print("Waiting for network...");
  int i = 0;
  while (!modem.waitForNetwork()) {
    Serial.print(".");
    delay(500);
    i++;
    if (i > 10) return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) { Serial.println("Network connected"); }

  // GPRS connection parameters are usually set after network registration
  Serial.print("Connecting to gprs ");
  Serial.print(apn);
  Serial.print("...");
  i = 0;
  while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.print(".");
    delay(500);
    i++;
    if (i > 10) return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) { Serial.println("GPRS connected"); }



  // MQTT Broker setup
   mqtt_gsm.setServer(broker, 1883);
  boolean status =  mqtt_gsm.connect("myESP32Client", "sensor", "sensor");

  if (status == false) {
    Serial.println(" fail");
    //return false;
  } else {
    publicTopic("sensor/111111/temp1", "12.5");
  }


  if (modem.gprsDisconnect()) { Serial.println("GPRS disconnected"); };
  modem.sendAT(GF("+CNETLIGHT=0"));
  modem.setPhoneFunctionality(0);
  //mqttConnect();

  return;
}

void publicTopic(String topic, String msg) {
   mqtt_gsm.publish(topic.c_str(), msg.c_str());
  Serial.print("SEND TOPIC: ");
  Serial.print(topic);
  Serial.print(" MESSAGE: ");
  Serial.println(msg);
}
