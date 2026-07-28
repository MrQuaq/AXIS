#include <Wire.h>
#include <7Semi_ICM20948.h> 

ICM20948_7Semi imu; // Create the sensor object

float weight = 0.98;
float angle = 0.0;

#define SENSOR A0

// Shared variables matching your global declarations
extern double dt;
extern unsigned long last_time;

// -------------------- SETUP --------------------
void setupIMU() {
  // Use standard ESP32 I2C pins: GPIO 21 (SDA) and GPIO 22 (SCL)
  Wire.begin(21, 22); 

  // Prevent infinite hardware loops if the sensor drops off the bus
  Wire.setTimeOut(50); 

  // Try initializing at 0x69 (or 0x68 depending on hardware address pin status)
  if (!imu.begin(Wire, 0x68)) {
    Serial.println(F("ERROR: ICM-20948 begin() failed. Retrying 0x68..."));
    if (!imu.begin(Wire, 0x68)) {
      Serial.println(F("ERROR: Crucial sensor communication failure. Check wiring."));
      while (1) delay(200);
    }
  }

  Serial.println(F("ICM-20948 ready."));

  uint8_t who = imu.readWhoAmI();
  Serial.print("WHO_AM_I: 0x");
  Serial.println(who, HEX);

  // Enable Accelerometer (true), Gyroscope (true), Magnetometer (false)
  if (!imu.setSensors(true, true, false)) {
      Serial.println(F("setSensors failed."));
  }

  // Configures default hardware scales and filters natively
  imu.applyBasicDefaults();

  delay(10);
}

// -------------------- UPDATE --------------------
void updateIMU() {
  // Local variables to temporarily hold raw readings
  float accelX = 0.0, accelY = 0.0, accelZ = 0.0;
  float gyroX = 0.0, gyroY = 0.0, gyroZ = 0.0;

  // Read data into variables by reference using the 7Semi library structure
  imu.readAccel(accelX, accelY, accelZ);
  imu.readGyro(gyroX, gyroY, gyroZ);

  // Accelerometer angle calculation (degrees)
  float accelAngle = atan2(accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;

  // Gyro integration
  float gyroAngle = angle + gyroX * dt;

  // Complementary filter mixing gyro (high-pass) and accelerometer (low-pass)
  angle = weight * gyroAngle + (1.0 - weight) * accelAngle;
}
