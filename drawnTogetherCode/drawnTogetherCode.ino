#include <Wire.h> //I2C Arduino Library
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"

#define BUTTON      6
#define POTI        A0
#define LDR         A1

// not every Pin is able to write analog (PWM) values!
#define LED_YELLOW  10
#define LED_INTERN  13
#define BEEPER      3

// for bluetooth
#define SERIAL_SPEED  115200

// better on PC
//#define SERIAL_SPEED  9600


// HARDWARE I2C: A4 -> SDA, A5 -> SCL
#define PIN_RESET  4 // dummy

// the SSD1306 I2C ADDRESS 0x3C is set in the Adafruit_SSD1306 header file
Adafruit_SSD1306 * oled = new Adafruit_SSD1306(PIN_RESET);


const int MPU=0x68;  // I2C address of the MPU-6050

char incoming[21] = {};
byte incomeId = 0;
int16_t AcX,AcY,AcZ;
float xx,yy,zz;
float val;

int16_t potiVal, ldrVal;

byte loops = 0;
byte storage[64]; // store 64 LDR values

void setup() {
  pinMode(BUTTON,     INPUT_PULLUP);
  pinMode(POTI,       INPUT);
  pinMode(LDR,        INPUT);
  pinMode(LED_INTERN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BEEPER,     OUTPUT);

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); // a start config for the MPU-6050 Gyroscope Chip
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(SERIAL_SPEED);

  oled->begin();
  oled->clearDisplay();
  oled->display();
  oled->setTextColor(WHITE);
  oled->setTextSize(2);
}

void loop() {
  loops++; // it is a byte and starts with 0 after more than 256x +1
  
  while (Serial.available() > 0) {
    incoming[incomeId] = (char) Serial.read();
    if (incoming[incomeId] == '\n') {
      incoming[incomeId] = '\0';
      incomeId = 0;
    } else {
      incomeId = (incomeId+1) % 21; // it is a 21 char buffer
    }
  }
  
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // The start register, we want to read
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // we need to read all 6 registers, really!

  AcX=Wire.read()<<8|Wire.read();  // 0x43 (ACCEL_XOUT_H) & 0x44 (ACCEL_XOUT_L)
  AcY=Wire.read()<<8|Wire.read();  // 0x45 (ACCEL_YOUT_H) & 0x46 (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x47 (ACCEL_ZOUT_H) & 0x48 (ACCEL_ZOUT_L)
  xx = AcX;
  yy = AcY;
  zz = AcZ;
  val = sqrt(xx*xx + yy*yy + zz*zz);

  potiVal = analogRead(POTI);
  ldrVal  = analogRead(LDR);
  analogWrite(BEEPER,     ldrVal>>5); // we are not able to write 11Bit values
  analogWrite(LED_YELLOW, potiVal>>4);
  storage[loops%64] = ldrVal>>3;

  if (loops==0) { // We do not Update display all the time
    Serial.println(ldrVal);   
    oled->clearDisplay();
    oled->setCursor(0, 0);
    oled->setTextSize(2);
    oled->print(val);
    oled->setTextSize(1);
    oled->setCursor(0, 55);
    oled->print(incoming);
    if (digitalRead(BUTTON) == LOW) {
      oled->setCursor(0, 44);
      oled->print(F("pressed"));
      digitalWrite(LED_INTERN, HIGH);
    } else {
      digitalWrite(LED_INTERN, LOW);
    }
    for (int i=0; i<64; ++i) {
      // improvement to display 64 values on 128 pixels
      oled->drawPixel(2*i-1, storage[i], WHITE);
      oled->drawPixel(2*i, storage[i], WHITE);
    }
    oled->display();
  }
}

