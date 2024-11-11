#include "OneWire.h"
#include "DallasTemperature.h"
OneWire oneWire(15);  //шина 1-wire к gpio15
DallasTemperature ds(&oneWire); // инициализация объекта для ds18b20

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello!");
}

// возвращает зачение первого найденного датчика
float getDS18b20() {
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

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(getDS18b20());
  delay(1000); // this speeds up the simulation
}
