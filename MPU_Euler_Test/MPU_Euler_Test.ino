#include <Wire.h>
#include <MPU6050.h>
#include <I2Cdev.h>
#include <helper_3dmath.h>

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  
  Wire.begin();
  mpu.initialize();
  
  // Verify connection
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed. Please check wiring.");
    while (1);
  }
  
  // Configure MPU6050
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  mpu.setDLPFMode(MPU6050_DLPF_BW_42);
}

void loop() {
  // Read raw sensor data
  int16_t accelX, accelY, accelZ;
  int16_t gyroX, gyroY, gyroZ;
  
  mpu.getMotion6(&accelX, &accelY, &accelZ, &gyroX, &gyroY, &gyroZ);
  
  // Convert raw sensor data to Euler angles
  double roll, pitch, yaw;
  
  roll = atan2(accelY, accelZ) * RAD_TO_DEG;
  pitch = atan(-accelX / sqrt(accelY * accelY + accelZ * accelZ)) * RAD_TO_DEG;
  yaw = atan2(gyroY, gyroZ) * RAD_TO_DEG;
  
  // Output Euler angles
  Serial.print("Euler Angles: ");
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print("° Pitch: ");
  Serial.print(pitch);
  Serial.print("° Yaw: ");
  Serial.print(yaw);
  Serial.println("°");
  
  delay(500);
}
