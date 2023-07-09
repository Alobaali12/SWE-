/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example prints the distance to an object.

  Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
*/

#include <Wire.h>
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

const float alpha = 0.1;  // EMA filter coefficient

unsigned long filteredDist= 0;

#define PIN_CNTRL 2

SFEVL53L1X distanceSensor1;
SFEVL53L1X distanceSensor2;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{
  pinMode(PIN_CNTRL, OUTPUT);
  digitalWrite(PIN_CNTRL, LOW);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Setting Sensor 1 address: ");
  distanceSensor1.setI2CAddress(0x54);
  Serial.println("Address Management Done");
  delay(3000);
  Serial.println("VL53L1X Qwiic Test");
  if (distanceSensor1.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor 1 failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor 1 address: " + String(distanceSensor1.getI2CAddress()) + "  Intialized  --- Ready to go");
  delay(1000);
  digitalWrite(PIN_CNTRL, HIGH);
  Serial.println("Sensor 2 address: " + String(distanceSensor2.getI2CAddress()));
  if (distanceSensor2.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor 2 failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }
  Serial.println("Sensor online!");
}
int distance2 = 0;
void loop(void)
{ 
  distanceSensor1.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor1.checkForDataReady())
  {
    delay(1);
  }
  int distance1 = distanceSensor1.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor1.clearInterrupt();
  distanceSensor1.stopRanging();

  distanceSensor2.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor2.checkForDataReady())
  {
    delay(1);
  }
  distance2 = alpha*distanceSensor2.getDistance()   + (1-alpha)*distance2; //Get the result of the measurement from the sensor
  distanceSensor2.clearInterrupt();
  distanceSensor2.stopRanging();

  Serial.print("Distance 1: ");
  Serial.println(distance1);


  Serial.print("      Distance 2: ");
  Serial.print(distance2);

  Serial.println();
}
