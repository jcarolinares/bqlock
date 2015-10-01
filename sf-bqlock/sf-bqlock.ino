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

//Clock variables
RTC_DS1307 RTC;
LiquidCrystal lcd(0);

//Encoder variables
#define encoderSwitchPin  7
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

void setup () {

  Serial.begin(19200); // Establece la velocidad de datos del puerto serie

  //Encoder setup


  Wire.begin();
  //RTC setup
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__)); // Establece la fecha y hora (Comentar una vez establecida la hora)

  //LCD setup
  initializeLcd();
  initializeEncoder();

}

void loop() {
  getTime();
  managePushEncoder();
  Serial.println(encoderValue);
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
    } else {
      //Short click
      if (debug) Serial.println("Short Click");
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

void getTime() {
  lcd.home();
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC

  checkDigits(now.day());
  lcd.print('/');
  checkDigits(now.month()); // Month
  lcd.print('/');
  checkDigits(now.year()); // Year
  lcd.print(' ');
  lcd.setCursor(0, 1);
  checkDigits(now.hour()); // Hour
  lcd.print(':');
  checkDigits(now.minute()); // Minute
  lcd.print(':');
  checkDigits(now.second());
  //lcd.print(now.second(), DEC); // Second

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
}
