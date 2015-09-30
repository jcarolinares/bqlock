/*
  bqlock
  Julian Caro
  Luis Díaz

  diwo.bq.com
*/

//libraries
#include <Wire.h>
#include "RTClib.h"
#include "bqLiquidCrystal.h"

/*#define I2C_ADDR_LCD    0x20
#define I2C_ADDR_RTC    0x68*/

//Clock variables
RTC_DS1307 RTC;
LiquidCrystal lcd(0);

void setup () {
  Serial.begin(19200); // Establece la velocidad de datos del puerto serie

  Wire.begin();
  RTC.begin();

  lcd.begin(16,2);
  lcd.display();
  lcd.setBacklight(HIGH);
  lcd.blink();
  lcd.print("Hola Mundo I2C!");
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora)
  delay(1000);
  lcd.clear();
  lcd.home();
}

void loop() {
  getTime();
}

void getTime(){
  //lcd.clear();
  lcd.home();
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC

  lcd.print(now.day(), DEC); // Day
  lcd.print('/');
  lcd.print(now.month(), DEC); // Month
  lcd.print('/');
  lcd.print(now.year(), DEC); // Year
  lcd.print(' ');
  lcd.setCursor(0,1);
  lcd.print(now.hour(), DEC); // Hour
  lcd.print(':');
  lcd.print(now.minute(), DEC); // Minute
  lcd.print(':');
  checkDigits(now.second());
  //lcd.print(now.second(), DEC); // Second

}
void checkDigits(int value){
  if(value<10){
    lcd.print(0);
    lcd.print(value);
  }else{
    lcd.print(value);
  }
}
