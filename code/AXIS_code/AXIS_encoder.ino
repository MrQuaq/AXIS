#ifndef ENCODERS_H
#define ENCODERS_H

#define LEFT_ENC_A 34
#define LEFT_ENC_B 35

#define RIGHT_ENC_A 32
#define RIGHT_ENC_B 33

volatile long leftCount = 0;
volatile long rightCount = 0;

void IRAM_ATTR leftEncoderISR(){
  if (digitalRead(LEFT_ENC_A) == digitalRead(LEFT_ENC_B))
      leftCount++;
  else
      leftCount--;
}

void IRAM_ATTR rightEncoderISR(){
  if (digitalRead(RIGHT_ENC_A) == digitalRead(RIGHT_ENC_B))
      rightCount++;
  else
      rightCount--;
}

void setupEncoders(){
  pinMode(LEFT_ENC_A, INPUT_PULLUP);
  pinMode(LEFT_ENC_B, INPUT_PULLUP);

  pinMode(RIGHT_ENC_A, INPUT_PULLUP);
  pinMode(RIGHT_ENC_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_ENC_A), leftEncoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENC_A), rightEncoderISR, CHANGE);
}

float getLeftSpeed(float dt){
  static long lastCount = 0;
  long delta = leftCount - lastCount;
  lastCount = leftCount;
  return delta / dt;
}

float getRightSpeed(float dt){
  static long lastCount = 0;
  long delta = rightCount - lastCount;
  lastCount = rightCount;
  return delta / dt;
}

#endif

