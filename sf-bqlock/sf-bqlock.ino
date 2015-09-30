/*
  Título: RTC_Arduino
  Descripción: Uso básico del reloj RTC DS1307
  Autor: FjRamirez
  Fecha: 9/09/2014
  URL: www.tuelectronica.es
  email: email@tuelectronica.es
*/

#include <Wire.h>
#include "RTClib.h"

//Clock variables
RTC_DS1307 RTC;


//Encoder Varibles


void setup () {
  Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora)
  Serial.begin(19200); // Establece la velocidad de datos del puerto serie
}

void loop() {



}

void getTime(){
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC

  Serial.print(now.year(), DEC); // Year
  Serial.print('/');
  Serial.print(now.month(), DEC); // Month
  Serial.print('/');
  Serial.print(now.day(), DEC); // Day
  Serial.print(' ');
  Serial.print(now.hour(), DEC); // Hour
  Serial.print(':');
  Serial.print(now.minute(), DEC); // Minute
  Serial.print(':');
  Serial.print(now.second(), DEC); // Second
  Serial.println();

}
