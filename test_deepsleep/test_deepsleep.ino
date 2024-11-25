#define S_To_uS_Factor 1000000ULL  //Conversion factor for micro seconds to seconds

#define MEASURE_INTERVAL 5 //Time ESP32 will go to sleep (in seconds)
                            //интервал чтения датчиков
#define SEND_WIFI_INTERVAL 30 // интервал отправки данных по wifi, если ничего не происзодит

RTC_DATA_ATTR int bootCount = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("hello setup");
  bootCount++;
}

void loop() {
  
  Serial.print("hello loop");
  //Serial.println(bootCount);
  if (bootCount*MEASURE_INTERVAL>SEND_WIFI_INTERVAL){
     Serial.print("SEND WIFI!!!");
     bootCount=0;
  }
  // put your main code here, to run repeatedly:
  esp_sleep_enable_timer_wakeup(MEASURE_INTERVAL * S_To_uS_Factor);
  Serial.println("Setup ESP32 to sleep for every " + String(MEASURE_INTERVAL) + " Seconds");
  //Go to sleep now
  
  esp_deep_sleep_start();
}
