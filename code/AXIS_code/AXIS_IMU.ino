#include <Wire.h>
#include <ICM_20948.h>


ICM_20948_I2C imu;

float weight = 0.98;
float angle = 0.0;

extern double dt;
extern unsigned long last_time;



// -------------------- SETUP --------------------
void setupIMU() {

  Wire.begin(19, 18);   // ESP32 SDA, SCL
  Wire.setClock(400000);

  if (imu.begin(Wire, 0x68) != ICM_20948_Stat_Ok) {
    Serial.println("ICM20948 not connected!");
    while (1);
  }

  Serial.println("ICM20948 ready");
}



// -------------------- UPDATE --------------------
void updateIMU() {

  imu.getAGMT();   // read accel + gyro

  // Accelerometer (g)
  float accelX = imu.accX();
  float accelY = imu.accY();
  float accelZ = imu.accZ();

  // Gyro is in RAD/S → convert to DEG/S
  float gyroX = imu.gyrX() * 180.0 / PI;

  // Accelerometer tilt angle (deg)
  float accelAngle = atan2(accelY,
                    sqrt(accelX * accelX + accelZ * accelZ))
                    * 180.0 / PI;

  // Gyro integration
  float gyroAngle = angle + gyroX * dt;

  // Complementary filter
  angle = weight * gyroAngle +
          (1.0 - weight) * accelAngle;

  Serial.println(angle);
}
