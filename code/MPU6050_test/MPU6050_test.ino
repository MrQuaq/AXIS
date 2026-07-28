#include <Wire.h>

float RateRoll, RatePitch, RateYaw;

void gyro_signals(void){
  Wire.beginTransmission(0x68);
  
  Wire.wire(0x1A);
  Wire.write(0x05);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.wire(0x1);
  Wire.write(0x8);
  Wire.endTransmission();

  Wire.beginTransmission(0x68);
  Wire.wire(0x43);
  Wire.endTransmission();

  Wire.requestFrom(0x68,6);
  int16_t GryroX=Wire.read()<<8 | Wire.read();
  int16_t GryroY=Wire.read()<<8 | Wire.read();
  int16_t GryroZ=Wire.read()<<8 | Wire.read();

  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  pinMode(13,OUTPUT);
  digitalWrite(13, HIGH);
  Wire.setClock(400000)
  Wire.begin();
  delay(250);
}

void loop() {
  // put your main code here, to run repeatedly:

}
