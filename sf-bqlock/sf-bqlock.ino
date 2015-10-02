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
#include <HTS221.h>
#include <HTS221_Registers.h>

/*
  Just for the record.
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

//Temperature and humidity variables
HTS221 hts221;
float h = 0.0, t = 0.0;

//Other variables
bool debug = true;
int mode = 0;
int maxMode = 2;

//clock & alarm variables
RTC_DS1307 RTC;

bool isAlarmOn = false;
int alarmHour=13, alarmMinute=48;
DateTime now;

bool playedOnce = false;
int oldMinute;


int pinBuz = 6;

int pinLDR = A3;
int threshold = 200;

void cleaaaar(){
  delay(500);
  lcd.clear();
}
void setup () {

  Serial.begin(19200); // Establece la velocidad de datos del puerto serie

  Wire.begin();
  //LCD setup
  initializeLcd();
  //Encoder setup
  cleaaaar();
  initializeEncoder();
  //RTC setup
  initializeTH();
  initializeRTC();

  pinMode(pinBuz, OUTPUT);

}
void loop() {
  checkDayNight();
  managePushEncoder();
  manageMode();
  checkAlarm();
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
  lcd.clear();
  lcd.print("HOUR SET: ");
  checkDigits(alarmHour);
  delay(userDelay*1);
  lcd.clear();
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
  lcd.print("MINUTE SET: ");
  checkDigits(alarmMinute);
  delay(userDelay*1);
  lcd.clear();

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
    //turn led on after clicked
    digitalWrite(pinledLong, HIGH);
    clicked = true;
    if (millis() > nextTime) {
      //blink led after clicked
      digitalWrite(pinledLong, HIGH);
      delay(100);
      digitalWrite(pinledLong, LOW);
      delay(100);
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
  //If forced == true, then update time and date
  lcd.home();
  now = RTC.now(); // Obtiene la fecha y hora del RTC
  if(oldMinute != now.minute() || forced){
    checkDigits(now.day());
    lcd.print('/');
    checkDigits(now.month());

    lcd.print(' ');
    lcd.setCursor(0, 1);
    checkDigits(now.hour());
    lcd.print(':');
    checkDigits(now.minute());

    oldMinute = now.minute();

    //use this function to update the temp and hum that is shown in lcd screen
    //workarround :D
    getTempHum();
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
    lcd.home();
    oldMinute = now.minute();
    lcd.print("Trying to reload time...");
    RTC.adjust(DateTime(__DATE__, __TIME__));
    if(checkTime){
      lcd.clear();
      lcd.home();
      lcd.print("Impossible to reload");
    }
    while(true){
      lcd.setCursor(0,1);
      lcd.print("RELOAD CLOCK!!");
    }// stops here!
  }
}
void initializeTH(){
  hts221.begin();
  if (hts221.checkConnection()) {
    if (debug){
      Serial.println("Error checking HTS221 connection");
    }
  } else {
    if(debug){
      Serial.println("HTS221 connected");
    }
  }
  getTempHum();
}
boolean checkTime(){
  //returns true when failure time and date it set.

  if ((now.day()==165 && now.month()==165 && now.year()== 2165 && now.hour()==165 && now.minute()==165 && now.second()==85)){
    lcd.clear();
    lcd.home();
    lcd.print("Check Wiring");
    while(true); // stops here!
  }
  boolean value  = (now.day()==1 && now.month()==1 && now.year()== 2000 && now.hour()==0 && now.minute()==0 && now.second()==0);
  Serial.println(value);
  return value;
}
void checkDayNight(){
  int ldrValue = analogRead(pinLDR);
  if(ldrValue > threshold ){
    lcd.setBacklight(HIGH);
  }else{
    lcd.setBacklight(LOW);
  }
}
void getTempHum(){
  h = int(hts221.getHumidity());
  t = hts221.getTemperature();

  lcd.setCursor(6,0);
  lcd.print(t,1);
  lcd.setCursor(10,0);
  //lcd.print(" ");

  lcd.setCursor(6,1);
  lcd.print(h,1);
  lcd.setCursor(8,1);
  lcd.print("% ");
}
