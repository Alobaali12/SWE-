#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

class IMU_DATA {
public:
  IMU_DATA(Adafruit_BNO055& imu); // Constructor
  float GetRoll();
  float GetPitch();
  float GetYaw();
  float GetW();
  float GetX();
  float GetY();
  float GetZ();
  void PrintIMUData();

private:
  Adafruit_BNO055& _imu;
};

IMU_DATA::IMU_DATA(Adafruit_BNO055& imu) : _imu(imu) {}

float IMU_DATA::GetRoll() {
  imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return euler.x();
}

float IMU_DATA::GetPitch() {
  imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return euler.y();
}

float IMU_DATA::GetYaw() {
  imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return euler.z();
}

float IMU_DATA::GetW() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.w();
}

float IMU_DATA::GetX() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.x();
}

float IMU_DATA::GetY() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.y();
}

float IMU_DATA::GetZ() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.z();
}

void IMU_DATA::PrintIMUData() {
  float roll = GetRoll();
  float pitch = GetPitch();
  float yaw = GetYaw();
  float w = GetW();
  float x = GetX();
  float y = GetY();
  float z = GetZ();

  Serial.print("Before transformation: ");
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(", Pitch: ");
  Serial.print(pitch);
  Serial.print(", Yaw: ");
  Serial.println(yaw);

  imu::Quaternion quat = _imu.getQuat();
  imu::Vector<3> euler = quat.toEuler();

  roll = euler.x() * RAD_TO_DEG;
  pitch = euler.y() * RAD_TO_DEG;
  yaw = euler.z() * RAD_TO_DEG;

  Serial.print("After transformation: ");
  Serial.print("Roll: ");
  Serial.print(roll);
  Serial.print(", Pitch: ");
  Serial.print(pitch);
  Serial.print(", Yaw: ");
  Serial.println(yaw);
}

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire); // IMU object
IMU_DATA IMU(bno); // Create an instance of IMUClass and pass the bno object

void setup() {
  Serial.begin(9600);

  if (!bno.begin()) {
    Serial.println("Failed to initialize BNO055! Check your wiring or I2C address.");
    while (1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);
}

void loop() {
  IMU.PrintIMUData(); // Print the IMU data
  delay(500);

  // Other loop code...
}
