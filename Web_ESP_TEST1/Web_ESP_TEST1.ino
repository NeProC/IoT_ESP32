#include <WiFi.h>
#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>


const int oneWireBus = 32; // порт термометра
const int An_IN1 = 33; // порт датчика давления

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);


bool ESPControllerWifiClient_HRD = 0;
bool ESPControllerWifiClient_status = 1;
bool ESPControllerWifiClient_isDHCP = 0;
bool ESPControllerWifiClient_IsNeedReconect = 0;
bool ESPControllerWifiClient_workStatus = 1;
char ESPControllerWifiClient_SSID[40] = "ORT_1";
char ESPControllerWifiClient_password[40] = "84953627044";

IPAddress ESPControllerWifiClient_ip(192, 168, 10, 200);
IPAddress  ESPControllerWifiClient_dns (192, 168, 10, 1);
IPAddress  ESPControllerWifiClient_gateway (192, 168, 10, 1);
IPAddress ESPControllerWifiClient_subnet (255, 255, 255, 0);
uint8_t ESPControllerWifiClient_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
WiFiServer ESPControllerWifi_tspWebServer(80);
WiFiClient ESPControllerWifi_tspWebServer_client;

String ESPControllerWifi_tspWebServer_client_buffer = "";
String webServerPageHeader1 = "HTTP/1.1 200 OK\r\n";
String webServerPageHeader2 = "Content-Type: text/html\r\n\r\n<!DOCTYPE HTML PUBLIC ""-//W3C//DTD HTML 4.01 Transitional//EN"">\r\n<META content=""text/html; charset=utf-8"" http-equiv=""Content-Type"">\r\n<html>";
unsigned long _d18x2x1Tti = 0UL;
float _d18x2x1O = 0.00;
float _Presh = 0.00;
float _Presh_old = 0.00;

float _tempVariable_float;
bool fl_WiFI_START_OK = 0;
int Range_P = 850; // уровень срабатывания
int Sum_Rang = 0; // сумматор давления
int Count_imp = 0; // счетчик срабатывания датчика
int Count_steps = 0; // счетчик отчетов при измерениии длительности импульса
float Int_P_sr = 0;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111110, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111,
  0b11111111, 0b11111111
};



void setup()
{

  
  WiFi.mode(WIFI_STA);
  _esp32WifiModuleClientReconnect();
  _parseMacAddressString(WiFi.macAddress(), ESPControllerWifiClient_mac);
  ESPControllerWifi_tspWebServer.begin();

  Serial.begin(115200);
  Serial.println("Start");
  sensors.begin(); //  температура

  //дисплей
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  String IP = "IP:192.168.10.200";

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(String(IP));
  display.display();
  delay(2000);
  display.clearDisplay();

}

void loop()
{

  WiFi_Func();


  //Плата:1
  if (1)
  {

    _Presh = analogRead(An_IN1) * 0.99;

    if (_Presh > Range_P) {
      Sum_Rang = Sum_Rang + _Presh;
      Count_steps++;
      _Presh_old = _Presh;

      Int_P_sr = Sum_Rang / Count_steps;
    }
    else {
      if (_Presh_old >= _Presh) {
        Count_imp ++;
        Count_steps = 0;
        _Presh_old = 0;
        Sum_Rang = 0;
      }
    }



    String P1 = "P=" + String(_Presh) + " IMP=" + String(Count_imp);


    if (_isTimer(_d18x2x1Tti, 1000))
    {
      _d18x2x1Tti = millis();
      sensors.requestTemperatures();
      _tempVariable_float = sensors.getTempCByIndex(0);


      String S1 = "T=" + String(_tempVariable_float) + "C";
      if (_tempVariable_float < 500)
      {
        _d18x2x1O = _tempVariable_float;
        display.clearDisplay();

        if (fl_WiFI_START_OK) {
          display.setCursor(10, 0);
          display.println(String("WiFI-192.168.10.200"));
          display.display();
        }
        else {
          display.setCursor(10, 0);
          display.println(String("WiFI- ERR"));
          display.display();
        }


        display.setCursor(10, 10);
        display.println(String(S1));
        display.setCursor(10, 20);
        display.println(String(P1));
        display.display();

      }
    }




    Serial.println(String(P1));
    Serial.println(String("Int_P_sr" + String(Int_P_sr)));
  }
}

bool _isTimer(unsigned long startTime, unsigned long period)
{
  unsigned long currentTime;
  currentTime = millis();
  if (currentTime >= startTime)
  {
    return (currentTime >= (startTime + period));
  }
  else
  {
    return (currentTime >= (4294967295 - startTime + period));
  }
}

void _sendWebServerPage(int sendPageNumber)
{
  if (sendPageNumber == -1)
  {
    return;
  }
  if (sendPageNumber == 1)
  {
    _sendWebServerPage1();
    return;
  }
  _sendWebServerSend404Page();
}
void _sendWebServerSend404Page(void)
{
  ESPControllerWifi_tspWebServer_client.print("HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML><html><body><p>404 - Page not fond</p></body></html>\r\n");
  delay(1);
  ESPControllerWifi_tspWebServer_client.flush();
  ESPControllerWifi_tspWebServer_client.stop();
}
int _parseWebServerReqest(String reqestAddres)
{
  int index;
  int result = 0;
  index = reqestAddres.indexOf("/");
  reqestAddres = reqestAddres.substring(index + 1);
  index = reqestAddres.indexOf(" ");
  reqestAddres = reqestAddres.substring(0, index);
  if (reqestAddres == "")
  {
    if (1)
    {
      result = 1;
    }
  }
  return result;
}
void _sendWebServerPage1(void)
{
  printToClient(webServerPageHeader1, &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("Refresh: 1\r\n", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(webServerPageHeader2, &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("Датчик расхода газа №1<br>", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("Температура -", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(String((_d18x2x1O)), &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("<br> ", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("Давление  мгновенное -", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(String((_Presh)), &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("<br> ", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);

  printToClient("Интеграл давления  -", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(String((Int_P_sr)), &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("<br> ", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);

  printToClient("Длительность импульса, отсчетов  -", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(String(Count_steps), &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("<br> ", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);


  printToClient("Число импульсов  -", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient(String((Count_imp)), &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  printToClient("<br> ", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);




  printToClient("</body></html>\r\n", &ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  commitClient(&ESPControllerWifi_tspWebServer_client, &ESPControllerWifi_tspWebServer_client_buffer);
  Serial.println("Page-Send");
}
void printToClient(String value, WiFiClient* sendClient, String* sendBuffer)
{
  for (int i = 0; i < value.length(); i++)
  {
    if (sendBuffer->length() > 800)
    {
      sendClient->print(*sendBuffer);
      *sendBuffer = "";
    }
    sendBuffer->concat(value.charAt(i));
  }
}
void commitClient(WiFiClient* sendClient, String* sendBuffer)
{
  if ((sendBuffer->length()) > 0)
  {
    sendClient->print(*sendBuffer);
  }
  *sendBuffer = "";
  sendClient->flush();
  sendClient->stop();
}
int hexStrToInt(String instring)
{
  byte len = instring.length();
  if  (len == 0) return 0;
  int result = 0;
  for (byte i = 0; i < 8; i++)    // только первые 8 цыфар влезуть в uint32
  {
    char ch = instring[i];
    if (ch == 0) break;
    result <<= 4;
    if (isdigit(ch))
      result = result | (ch - '0');
    else result = result | (ch - 'A' + 10);
  }
  return result;
}
void _esp32WifiModuleClientReconnect()
{
  if (ESPControllerWifiClient_isDHCP)
  {
    WiFi.config(0U, 0U, 0U, 0U, 0U);
  }
  else
  {
    WiFi.config(ESPControllerWifiClient_ip, ESPControllerWifiClient_gateway, ESPControllerWifiClient_subnet, ESPControllerWifiClient_dns , ESPControllerWifiClient_dns);
  }
  WiFi.begin(ESPControllerWifiClient_SSID, ESPControllerWifiClient_password);
}
bool _checkMacAddres(byte array[])
{
  bool result = 0;
  for (byte i = 0; i < 6; i++)
  {
    if (array[i] == 255)
    {
      return 0;
    }
    if (array[i] > 0)
    {
      result = 1;
    }
  }
  return result;
}
void _parseMacAddressString(String value, byte array[])
{
  int index;
  byte buffer[6] = {255, 255, 255, 255, 255, 255};
  byte raz = 0;
  String tempString;
  while ((value.length() > 0) && (raz <= 6))
  {
    index = value.indexOf(":");
    if (index == -1)
    {
      tempString = value;
      value = "";
    }
    else
    {
      tempString = value.substring(0, index);
      value = value.substring(index + 1);
    }
    buffer[raz] = byte(hexStrToInt(tempString));
    raz++;
  }
  if (_checkMacAddres(buffer))
  {
    for (byte i = 0; i < 6; i++)
    {
      array[i] = buffer[i];
    }
  }
}
bool _compareMacAddreses(byte array1[], byte array2[])
{
  for (byte i = 0; i < 6; i++)
  {
    if (array1[i] != array2[i])
    {
      return 0;
    }
  }
  return 1;
}
bool _compareMacAddresWithString(byte array[], String value)
{
  byte buffer[6] = {255, 255, 255, 255, 255, 255};
  _parseMacAddressString(value,  buffer);
  return _compareMacAddreses(array, buffer);
}
bool _checkMacAddresString(String value)
{
  byte buffer[6] = {255, 255, 255, 255, 255, 255};
  _parseMacAddressString(value,  buffer);
  return _checkMacAddres(buffer);
}
String _macAddresToString(byte array[])
{
  String result = "";
  String  temp = "";
  for (byte i = 0; i < 6; i++)
  {
    temp = String(array[i], HEX);
    if (temp.length()  < 2)
    {
      temp = String("0") + temp;
    }
    result = result + temp;
    if (i < 5)
    {
      result = result + String(":");
    }
  }
  result.toUpperCase();
  return result;
}

void  WiFi_Func() {
  ESPControllerWifi_tspWebServer_client = ESPControllerWifi_tspWebServer.available();

  if (ESPControllerWifi_tspWebServer_client)
  {
    String _WSCReqest = "";
    int _WSCPageNumber = 0;
    delay(5);
    while (ESPControllerWifi_tspWebServer_client.available())
    {
      _WSCReqest.concat(char(ESPControllerWifi_tspWebServer_client.read()));
    }
    ESPControllerWifi_tspWebServer_client.flush();
    _WSCPageNumber  = _parseWebServerReqest(_WSCReqest);
    _sendWebServerPage(_WSCPageNumber);
  }
  if (ESPControllerWifiClient_IsNeedReconect)
  {
    _esp32WifiModuleClientReconnect();
    ESPControllerWifiClient_IsNeedReconect = 0;
  }
  ESPControllerWifiClient_status = WiFi.status() == WL_CONNECTED;
  //Serial.println("WiFI-connect IP:192.168.10.200");
  if (ESPControllerWifiClient_status)
  {
    if (! ESPControllerWifiClient_HRD)
    {
      ESPControllerWifiClient_ip =  WiFi.localIP();
      ESPControllerWifiClient_subnet =  WiFi.subnetMask();
      ESPControllerWifiClient_gateway =  WiFi.gatewayIP();
      ESPControllerWifiClient_dns =  WiFi.dnsIP();
      ESPControllerWifiClient_HRD = 1;
    }
    fl_WiFI_START_OK = 1;



  }
  else
  {

    ESPControllerWifiClient_HRD = 0;
    Serial.println("WiFI- ERR");
    fl_WiFI_START_OK = 0;
  }
}
