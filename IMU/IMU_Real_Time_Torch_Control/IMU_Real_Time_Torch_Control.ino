#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>

#define DEG (180/M_PI)

Adafruit_BNO055 bno = Adafruit_BNO055(55);




void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);

  if (!bno.begin()) {
    Serial.println("No BNO055 Detected!");
    while(1);
  }
  delay(1000);
  bno.setMode(OPERATION_MODE_NDOF_FMC_OFF);
  bno.setExtCrystalUse(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  imu::Quaternion quat = bno.getQuat();
  imu::Vector<3> euler = quat.toEuler();

 // Serial.print("W: ");
    Serial.print(quat.w());
    Serial.print(",");
//  
//  Serial.print("x: ");
    Serial.print(quat.x());
    Serial.print(",");
//  
//  Serial.print("y: ");
    Serial.print(quat.y());
    Serial.print(",");
//  
//  Serial.print("z: ");
    Serial.println(quat.z());
//  Serial.print(F(" "));

/*
  Serial.print("Yaw: ");
  Serial.print(euler[0] * DEG);
  Serial.print(F(" "));
  Serial.print("Pitch: ");
  Serial.print(euler[1] * DEG);
  Serial.print(F(" "));
  Serial.print("Roll: ");
  Serial.print(euler[2] * DEG);
  Serial.print(F(" "));
  Serial.println("");
  delay(200);
  */
}