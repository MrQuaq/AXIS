#include <Wire.h>
#include <7Semi_ICM20948.h> 

ICM20948_7Semi imu; // Create the sensor object

float weight = 0.98;
float angle = 0.0;

#define SENSOR A0

// Define placeholder variables if not declared in another tab
// If you already have these in your main tab, you can comment these out.
double dt = 0.01; // Assuming 10ms loop time sample placeholder
unsigned long last_time = 0;

// -------------------- SETUP --------------------
void setupIMU() {
  Wire.begin(21, 22);

  if (!imu.begin(Wire, 0x68)) {
    Serial.println(F("ERROR: ICM-20948 begin() failed."));
    while (1) delay(200);
  }

  Serial.println(F("ICM-20948 ready."));

  uint8_t who = imu.readWhoAmI();
  Serial.print("WHO_AM_I: 0x");
  Serial.println(who, HEX);

  if (!imu.setSensors(true, true, false)) {
      Serial.println(F("setSensors failed."));
  }

  // This library handles full-scale range configurations via defaults natively
  imu.applyBasicDefaults();

  delay(10);
}

// -------------------- UPDATE --------------------
void updateIMU() {
  // Create variables to hold the raw readings passed by reference
  float accelX = 0.0, accelY = 0.0, accelZ = 0.0;
  float gyroX = 0.0, gyroY = 0.0, gyroZ = 0.0;

  // Read data into the variables using the correct 7Semi library reference parameters
  imu.readAccel(accelX, accelY, accelZ);
  imu.readGyro(gyroX, gyroY, gyroZ);

  // Accelerometer angle (degrees)
  float accelAngle = atan2(accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;

  // Gyro integration
  float gyroAngle = angle + gyroX * dt;

  // Complementary filter
  angle = weight * gyroAngle + (1.0 - weight) * accelAngle;
  Serial.println(angle);
}

void setup() {
  Serial.begin(9600);
  setupIMU();
  last_time = millis();
}

void loop() {
  // Simple dt calculation loop for standalone verification
  unsigned long current_time = millis();
  dt = (current_time - last_time) / 1000.0;
  last_time = current_time;

  updateIMU();
  delay(10); // Run at ~100Hz
}