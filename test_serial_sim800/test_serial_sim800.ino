#include <HardwareSerial.h>


HardwareSerial sim800(2);  // define a Serial for UART2
//const int MySerialRX = 16;
//const int MySerialTX = 17;


void setup() {
  Serial.begin(9600);  // Скорость обмена данными с компьютером
  Serial.println("Start!");
  //  sim800.begin(9600);  // Скорость обмена данными с GSM модулем
  sim800.begin(9600, SERIAL_8N1, 16, 17);
  sim800.println("AT");
}

void loop() {
  if (sim800.available())
    Serial.write(sim800.read());
  if (Serial.available())
    sim800.write(Serial.read());
}