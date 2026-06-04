#include "Globals.h"

#include <LiquidCrystal_I2C.h>
#include <Wire.h>


//--------GLOBAL VARIABLES------------
extern double dt;
extern unsigned long last_time;

extern float angle;


LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- ANGLE PID --------
double integral = 0.0;
double previous = 0.0;
double kp = 0.0, ki = 0.0, kd = 0.0;
double setpoint = 0.0;


// -------- SPEED PID --------
double speedIntegral = 0.0;
double lastSpeedError = 0.0;

double kp_speed = 0.003;  // start small
double ki_speed = 0.0;
double kd_speed = 0.0;

bool mode = true;
bool lastAState = false;

unsigned long lastAdjustTimeButtons = 0;
unsigned long lastAdjustTimeDpad = 0;

const unsigned long buttonDelay = 60; // ms
const unsigned long dpadDelay = 60;   // ms




//--------------------------------------------------------------------------

// -------------------- CONTROLLER --------------------
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      Serial.printf("Controller connected, index=%d\n", i);
      return;
    }
  }
  Serial.println("Controller connected, no empty slot");
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.printf("Controller disconnected, index=%d\n", i);
      return;
    }
  }
  Serial.println("Controller disconnected, not found");
}

void processGamepad(ControllerPtr ctl) {

  uint16_t buttons = ctl->buttons();
  uint8_t dpad = ctl->dpad();

  unsigned long now = millis();


  // -------- MODE TOGGLE (press A once) --------
  bool currentAState = (buttons & BUTTON_A);

  if (currentAState && !lastAState) {
    mode = !mode;
  }

  lastAState = currentAState;


  // -------- ANGLE PID MODE --------
  if (mode == true) {

    if ((buttons & BUTTON_X) && (now - lastAdjustTimeButtons > buttonDelay)) {
      kp -= 0.1;
      lastAdjustTimeButtons = now;
    }

    if ((buttons & BUTTON_Y) && (now - lastAdjustTimeButtons > buttonDelay)) {
      kp += 0.1;
      lastAdjustTimeButtons = now;
    }

    if ((dpad & DPAD_UP) && (now - lastAdjustTimeDpad > dpadDelay)) {
      ki += 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_DOWN) && (now - lastAdjustTimeDpad > dpadDelay)) {
      kd -= 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_RIGHT) && (now - lastAdjustTimeDpad > dpadDelay)) {
      kd += 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_LEFT) && (now - lastAdjustTimeDpad > dpadDelay)) {
      ki -= 0.01;
      lastAdjustTimeDpad = now;
    }

  }


  // -------- SPEED PID MODE --------
  else {

    if ((buttons & BUTTON_X) && (now - lastAdjustTimeButtons > buttonDelay)) {
      kp_speed -= 0.01;
      lastAdjustTimeButtons = now;
    }

    if ((buttons & BUTTON_Y) && (now - lastAdjustTimeButtons > buttonDelay)) {
      kp_speed += 0.01;
      lastAdjustTimeButtons = now;
    }

    if ((dpad & DPAD_UP) && (now - lastAdjustTimeDpad > dpadDelay)) {
      ki_speed += 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_DOWN) && (now - lastAdjustTimeDpad > dpadDelay)) {
      kd_speed -= 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_RIGHT) && (now - lastAdjustTimeDpad > dpadDelay)) {
      kd_speed += 0.01;
      lastAdjustTimeDpad = now;
    }

    if ((dpad & DPAD_LEFT) && (now - lastAdjustTimeDpad > dpadDelay)) {
      ki_speed -= 0.01;
      lastAdjustTimeDpad = now;
    }

  }


  // -------- SETPOINT CONTROL --------
  if (ctl->axisY() <= -25) {
    setpoint += 2;
  }

  if (ctl->axisY() >= 25) {
    setpoint -= 2;
  }


  // -------- SAFETY LIMITS --------
  kp = constrain(kp, 0, 100);
  ki = constrain(ki, 0, 50);
  kd = constrain(kd, 0, 50);

  kp_speed = constrain(kp_speed, 0, 5);
  ki_speed = constrain(ki_speed, 0, 5);
  kd_speed = constrain(kd_speed, 0, 5);

}




void processControllers() {
  for (auto ctl : myControllers) {
    if (ctl && ctl->isConnected()) {
      if (ctl->isGamepad()) {
        processGamepad(ctl);
      }
    }
  }
}

// -------------------- PID FUNCTION --------------------
double PID(double error) {
  double proportional = error;
  integral += error * dt;
  // Anti-windup
  integral = constrain(integral, -100, 100);
  double derivative = (error - previous) / dt;
  previous = error;
  return (kp * proportional) + (ki * integral) + (kd * derivative);
}

// -------------------- PID SPEED FUNCTION --------------------
double speedPID() {
  float leftSpeed = getLeftSpeed(dt);
  float rightSpeed = getRightSpeed(dt);
  float avgSpeed = (leftSpeed + rightSpeed) / 2.0;

  double speedError = 0 - avgSpeed;

  speedIntegral += speedError * dt;

  speedIntegral = constrain(speedIntegral, -100, 100);

  double speedDerivative = (speedError - lastSpeedError) / dt;

  lastSpeedError = speedError;
  return (kp_speed * speedError) + (ki_speed * speedIntegral) + (kd_speed * speedDerivative);
}

// -------------------- SETUP --------------------
void setupcontroller() {
  Serial.begin(115200);

  lcd.clear();

  Wire.begin(21, 22);

  // LCD
  lcd.init();
  lcd.backlight();

  kp = 0.0;
  ki = 0.0;
  kd = 0.0;
  last_time = 0;

  // Bluepad32
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();

  last_time = micros();
}

// -------------------- LOOP --------------------
int controller() {
  // Update controller inputs
  BP32.update();
  processControllers();

  // Read angle from IMU (replace this with real filtered value)
  double actual = angle;

  // PID calculation
  double error = setpoint - actual;
  PID_OUTPUT = PID(error);
  SPEEDPID_OUTPUT = speedPID();
  double FINALPID_OUTPUT = PID_OUTPUT + SPEEDPID_OUTPUT;
  FINALPID_OUTPUT = constrain(FINALPID_OUTPUT, -255, 255);
  int pwm = constrain(abs(FINALPID_OUTPUT), 0, 255);


  // LCD display PID values
  lcd.setCursor(0, 0);
  lcd.print("Kp:");
  lcd.print(kp, 2);
  lcd.setCursor(0, 1);
  lcd.print("Ki:");
  lcd.print(ki, 3);
  lcd.print(" Kd:");
  lcd.print(kd, 3);

  if (mode == true){
    lcd.setCursor(0, 0);
    lcd.print("MODE1");
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("MODE2");
  }
  
//
  //Serial.print("kp:");
  //Serial.print(kp);
  //Serial.print(",");
  //Serial.print("ki:");
  //Serial.print(ki);
  //Serial.print(",");
  //Serial.print("kd:");
  //Serial.print(kd);
  //Serial.print(",");
  //Serial.print("kp_speed:");
  //Serial.print(kp_speed);
  //Serial.print(",");
  //Serial.print("ki_speed:");
  //Serial.print(ki_speed);
  //Serial.print(",");
  //Serial.print("kd_speed:");
  //Serial.println(kd_speed);
//
  return (pwm);
}
