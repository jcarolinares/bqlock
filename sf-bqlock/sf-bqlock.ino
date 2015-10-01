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
#include <Encoder.h>

/*
  I2C_ADDR_LCD    0x20
  I2C_ADDR_RTC    0x68
*/


LiquidCrystal lcd(0);

//Encoder variables
#define encoderSwitchPin  4
#define encoderPin1 2
#define encoderPin2 3
//Push buttons-encoder variables
int pinledLong = 13;

bool clicked = false;

int userDelay = 250;
unsigned long nextTime;
int intervale = 1000;
//Encoder rotation variables
int sum;

volatile int lastEncoded = 3; //You must initialize the encoders pins on 11 (3) !!!
volatile long encoderValue = 0;

long lastencoderValue = 0;

int lastMSB = 0;
int lastLSB = 0;

//Other variables
bool debug = true;

//clock & alarm variables
RTC_DS1307 RTC;

bool isAlarmOn = false;
int alarmHour=13, alarmMinute=48;
DateTime now;

bool playedOnce = false;
int oldMinute;

int pinBuz = 6;

int mode = 0;
int maxMode = 2;

void setup () {

  Serial.begin(19200); // Establece la velocidad de datos del puerto serie

  Wire.begin();
  //LCD setup
  initializeLcd();
  //Encoder setup
  initializeEncoder();
  //RTC setup
  initializeRTC();

  pinMode(pinBuz, OUTPUT);

}

void loop() {
  managePushEncoder();
  manageMode();
  checkAlarm();
  if (debug) Serial.println(encoderValue);
}
void manageMode(){
  switch (mode){
    case 0:
      getTime(false);
      printAlarmStatus();
      break;
    case 1:
      //cambio hora alarma
      setAlarmHour();
      delay(userDelay);
      setAlarmMin();
      isAlarmOn=true;
      delay(userDelay);
      mode = 0;
      getTime(true);
      break;
  }
}
void setAlarmHour(){
  lastEncoded = 3; //You must initialize the encoders pins on 11 (3) !!!
  encoderValue = 0;
  alarmHour = 0;
  lcd.clear();
  lcd.home();
  lcd.print("Set hour: ");
  while(!digitalRead(encoderSwitchPin)){
    lcd.setCursor(10,0);
    alarmHour = constrain(encoderValue,0,96)/4;
    Serial.println(alarmHour);
    checkDigits(alarmHour);
  }
  Serial.print("Set Hour: ");
  Serial.println(alarmHour);
  lcd.clear();
  lcd.print("HOUR SET");
  delay(userDelay*1);
}
void setAlarmMin(){
  lastEncoded = 3; //You must initialize the encoders pins on 11 (3) !!!
  encoderValue = 0;
  alarmMinute = 0;
  lcd.clear();
  lcd.home();
  lcd.print("Set min: ");
  while(!digitalRead(encoderSwitchPin)){
    lcd.setCursor(10,0);
    checkDigits(alarmMinute);
    alarmMinute = constrain(encoderValue,0,240)/4;
  }
  lcd.clear();
  lcd.print("minute SET");
  delay(userDelay*1);

}
void checkAlarm(){
  int alarmDuration=250; //Real alarm duration will be alarmDuration * 2 * times
  int times = 15;
  if (isAlarmOn && now.hour() == alarmHour && now.minute() == alarmMinute && !playedOnce){
    for (int i=0;i<times;i++){
      Serial.println("Alarm ON");
      tone(pinBuz, 500, alarmDuration);
      delay(alarmDuration);
      noTone(pinBuz);
      delay(alarmDuration);
      playedOnce = true;
    }
  }else if(now.hour() != alarmHour || now.minute() != alarmMinute){
    playedOnce = false;
  }
}
void printAlarmStatus(){
  lcd.setCursor(11,0);
  lcd.print("Alarm");
  lcd.setCursor(11,1);
  if (isAlarmOn){
    checkDigits(alarmHour);
    lcd.print(":");
    checkDigits(alarmMinute);
  }else{
    lcd.print("Off  ");
  }
}
void managePushEncoder() {

  nextTime = millis() + intervale;
  while (digitalRead(encoderSwitchPin)) {
    clicked = true;
    Serial.println("Inside while");
    if (millis() > nextTime) {
      digitalWrite(pinledLong, HIGH);
    }
  }
  digitalWrite(pinledLong, LOW);

  if (clicked) {
    if (millis() > nextTime) {
      //Long click
      if (debug) Serial.println("Long Click");
      isAlarmOn = !isAlarmOn;
    } else {
      //Short click
      if (debug) Serial.println("Short Click");
      mode++;
      if(mode == maxMode){
        mode = 0;
      }

      delay(userDelay);

    }
    delay(userDelay);//used for debouncing
    clicked = false;
  }

}
//Function to update encoder value
void updateEncoder() {

  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit


  int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number

  sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value


  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;



  lastEncoded = encoded; //store this value for next time
}
void getTime(bool forced) {
  lcd.home();
  now = RTC.now(); // Obtiene la fecha y hora del RTC
  if(oldMinute != now.minute() || forced){
    checkDigits(now.day());
    lcd.print('/');
    checkDigits(now.month());
    lcd.print('/');
    checkDigits(now.year());
    lcd.print(' ');
    lcd.setCursor(0, 1);
    checkDigits(now.hour());
    lcd.print(':');
    checkDigits(now.minute());
    //lcd.print(':');
    //checkDigits(now.second());
    oldMinute = now.minute();
    }
}
void checkDigits(int value) {
  //this function adds a 0 before the digits if is
  //between 0 and 9
  if (value < 10) {
    lcd.print(0);
    lcd.print(value);
  } else {
    lcd.print(value);
  }
}
void initializeEncoder(){
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);

  pinMode(encoderSwitchPin, INPUT);

  digitalWrite(encoderPin1, HIGH); //turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); //turn pullup resistor on

  digitalWrite(encoderSwitchPin, HIGH); //turn pullup resistor on

  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);
}
void initializeLcd(){

  lcd.begin(16, 2);
  lcd.display();
  lcd.setBacklight(HIGH);
  lcd.blink();
  lcd.print("Bqlock");
  lcd.setCursor(0,1);
  lcd.print("diwo.bq.com");
  delay(1000);
  lcd.home();
  lcd.clear();
  lcd.noBlink();
}
void initializeRTC(){
  RTC.begin();
  getTime(true);
  if(checkTime()){
    lcd.clear();
    oldMinute = now.minute();
    Serial.println("Trying to reload time...");
    RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora)
    if(checkTime){
      Serial.println("Impossible to reload");
    }
    while(true){
      lcd.home();
      lcd.print("RELOAD CLOCK!!");
    }
  }
}
boolean checkTime(){
  //returns true when failure time and date it set.
  Serial.println("Checking time");
  return (now.day()==1 && now.month()==1 && now.year()== 2000 && now.hour()==0 && now.minute()==0 && now.second()==0);
}
