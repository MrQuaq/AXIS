#include "Globals.h"
#include <LiquidCrystal_I2C.h>

//--------GLOBAL VARIABLES------------
extern double dt;
extern unsigned long last_time;
extern float angle;

// Ensure these are defined globally in Globals.h or your main tab
extern double PID_OUTPUT;
extern double SPEEDPID_OUTPUT;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- ANGLE PID --------
double integral = 0.0;
double kp = 3.5;    // Start with a safe default instead of 0.0
double ki = 0.05;   
double kd = 0.1;    
double setpoint = 0.0;

// -------- SPEED PID --------
double speedIntegral = 0.0;
double lastSpeedError = 0.0;

double kp_speed = 0.05;   // Adjusted for cascade control scaling
double ki_speed = 0.001;
double kd_speed = 0.0;

// Track button press states to prevent rapid firing adjustments
unsigned long lastButtonTime = 0;

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

  // Rate limit button presses to once every 150ms so numbers change controllably
  if (millis() - lastButtonTime > 150) {
    // X button -> decrease kp
    if (ctl->x()) { kp -= 0.1; lastButtonTime = millis(); }
    // Y button -> increase kp
    if (ctl->y()) { kp += 0.1; lastButtonTime = millis(); }
    // Dpad UP -> increase ki
    if (buttons & 0x0001) { ki += 0.01; lastButtonTime = millis(); }
    // Dpad LEFT -> decrease ki
    if (buttons & 0x0008) { ki -= 0.01; lastButtonTime = millis(); }
    // Dpad RIGHT -> increase kd
    if (buttons & 0x0004) { kd += 0.01; lastButtonTime = millis(); }
    // Dpad DOWN -> decrease kd
    if (buttons & 0x0002) { kd -= 0.01; lastButtonTime = millis(); }
  }

  // Left joystick Y -> change setpoint
  if (ctl->axisY() <= -25) setpoint += 0.1; // Smaller increments for gentler trim tuning
  if (ctl->axisY() >= 25) setpoint -= 0.1;
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
  static double previous = 0.0;
  double proportional = error;
  
  integral += error * dt;
  integral = constrain(integral, -100, 100); // Anti-windup
  
  double derivative = 0.0;
  if (dt > 0.0) { // Protect against divide-by-zero crash
    derivative = (error - previous) / dt;
  }
  previous = error;
  
  return (kp * proportional) + (ki * integral) + (kd * derivative);
}

// -------------------- PID SPEED FUNCTION --------------------
double speedPID() {
  float leftSpeed = getLeftSpeed(dt);
  float rightSpeed = getRightSpeed(dt);
  float avgSpeed = (leftSpeed + rightSpeed) / 2.0;

  double speedError = 0.0 - avgSpeed;

  speedIntegral += speedError * dt;
  speedIntegral = constrain(speedIntegral, -50, 50);

  double speedDerivative = 0.0;
  if (dt > 0.0) {
    speedDerivative = (speedError - lastSpeedError) / dt;
  }

  lastSpeedError = speedError;
  return (kp_speed * speedError) + (ki_speed * speedIntegral) + (kd_speed * speedDerivative);
}

// -------------------- SETUP --------------------
void setupcontroller() {
  Serial.begin(115200);
  Wire.begin();

  // LCD
  lcd.init();
  lcd.backlight();

  // Removed the lines that reset kp, ki, kd to 0.0 instantly

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

  double actual = angle;

  // 1. CASCADING APPROACH: Run the Speed PID first to find the angle offset adjustment
  SPEEDPID_OUTPUT = speedPID();
  
  // 2. Adjust target angle setpoint dynamically based on wheel velocity
  double adjustedSetpoint = setpoint + SPEEDPID_OUTPUT;

  // 3. Run Angle Control loop based on the new adjusted setpoint target
  double error = adjustedSetpoint - actual;
  PID_OUTPUT = PID(error);
  
  double FINALPID_OUTPUT = PID_OUTPUT;
  
  // Constrain to physical PWM limits
  FINALPID_OUTPUT = constrain(FINALPID_OUTPUT, -255, 255);
  int pwm = constrain(abs(FINALPID_OUTPUT), 0, 255);

  // LCD display PID values
  lcd.setCursor(0, 0);
  lcd.print("Kp:");
  lcd.print(kp, 1);
  lcd.print(" SP:");
  lcd.print(adjustedSetpoint, 1);
  
  lcd.setCursor(0, 1);
  lcd.print("P:");
  lcd.print(pwm);
  lcd.print(" Ang:");
  lcd.print(actual, 1);

  return(pwm);
}