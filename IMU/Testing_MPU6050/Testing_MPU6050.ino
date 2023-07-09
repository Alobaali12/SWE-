#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <MadgwickAHRS.h>

Adafruit_MPU6050 mpu;

Madgwick filter;

void setup(void) {
  Serial.begin(115200);
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

  // Set up motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);    // Keep it latched. Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);

  // Initialize the Madgwick filter
  filter.begin(100); // Sample rate in Hz

  Serial.println("");
  delay(100);
}

void loop() {
  if (mpu.getMotionInterruptStatus()) {
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convert raw sensor data to usable units
    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;
    float gx = g.gyro.x;
    float gy = g.gyro.y;
    float gz = g.gyro.z;

    // Update the filter with IMU data
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // Get Euler angles
    float roll, pitch, yaw;
    filter.getEuler(&roll, &pitch, &yaw);

    // Convert angles to degrees
    roll = roll * 180.0 / PI;
    pitch = pitch * 180.0 / PI;
    yaw = yaw * 180.0 / PI;

    // Print Euler angles
    Serial.print("Euler angles: ");
    Serial.print("Roll: ");
    Serial.print(roll);
    Serial.print(", Pitch: ");
    Serial.print(pitch);
    Serial.print(", Yaw: ");
    Serial.println(yaw);

  }

  delay(10);
}
