// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

float weight = 0.80;
float angle = 0;
float dt; // time delta
unsigned long previousTime = 0;

void setup(void) {
  Serial.begin(115200);

  previousTime = millis();

  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

void loop() {
  //tijd sinds vorige loop
  unsigned long currentTime = millis();
  dt = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime;

  /* Get new sensor events */
sensors_event_t a, g, temp;
mpu.getEvent(&a, &g, &temp);

// Subtract offsets
 float ax = a.acceleration.x - ACCEL_X_OFFSET;
float ay = a.acceleration.y - ACCEL_Y_OFFSET;
float az = a.acceleration.z - ACCEL_Z_OFFSET;

float gx = g.gyro.x - GYRO_X_OFFSET;
// You can also do gy, gz if you need them later

// Calculate pitch angle from accelerometer
float accelAngle = atan2(ax, sqrt(ay * ay + az * az)) * 180.0 / PI;

// Calculate gyro angle delta
float gyroRate = gx * 180.0 / PI; // convert rad/s to deg/s
float gyroAngle = gyroAngle + gyroRate * dt;

// Complementary filter
angle = (1 - weight) * gyroAngle + weight * accelAngle;

// Serial output for plotting
Serial.print(accelAngle); Serial.print(",");
Serial.print(gyroRate); Serial.print(",");
Serial.println(angle);

  ///* Print out the values */
  //Serial.print("Acceleration X: ");
  //Serial.print(a.acceleration.x);
  //Serial.print(", Y: ");
  //Serial.print(a.acceleration.y);
  //Serial.print(", Z: ");
  //Serial.print(a.acceleration.z);
  //Serial.println(" m/s^2");
//
  //Serial.print("Rotation X: ");
  //Serial.print(g.gyro.x);
  //Serial.print(", Y: ");
  //Serial.print(g.gyro.y);
  //Serial.print(", Z: ");
  //Serial.print(g.gyro.z);
  //Serial.println(" rad/s");
//
  //Serial.print("Temperature: ");
  //Serial.print(temp.temperature);
  //Serial.println(" degC");

  //Serial.println("");
  //delay(500);
}