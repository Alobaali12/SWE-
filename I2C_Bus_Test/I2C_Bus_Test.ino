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
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define PIN_GATE_IN 8
#define IRQ_GATE_IN  0
#define PIN_LED_OUT 13
#define PIN_ANALOG_IN A0

const float alpha = 1;  // EMA filter coefficient

unsigned long filteredDist= 0;

#define PIN_CNTRL 2
#define BNO055_SAMPLERATE_DELAY_MS (100)

Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28, &Wire);

SFEVL53L1X distanceSensor1;
SFEVL53L1X distanceSensor2;

int distance2 = 0;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{

  pinMode(PIN_LED_OUT, OUTPUT);

  // configure input to interrupt
  pinMode(PIN_GATE_IN, INPUT);
  attachInterrupt(IRQ_GATE_IN, soundISR, CHANGE);

  // Display status
  
  pinMode(PIN_CNTRL, OUTPUT);
  digitalWrite(PIN_CNTRL, LOW);
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Initialized Sound sensor");
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

   while (!Serial) delay(10);  // wait for serial port to open!

  Serial.println("Orientation Sensor Raw Data Test"); Serial.println("");

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  delay(1000);

  /* Display the current temperature */
  int8_t temp = bno.getTemp();
  Serial.print("Current Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.println("");

  bno.setExtCrystalUse(true);

  Serial.println("Calibration status values: 0=uncalibrated, 3=fully calibrated");

}



void loop(void)
{ 
  int value;

  // Check the envelope input
  value = analogRead(PIN_ANALOG_IN);

  // Convert envelope value into a message
  Serial.print("Status: ");
  if(value <= 10)
  {
    Serial.println("Quiet.");
  }
  else if( (value > 10) && ( value <= 30) )
  {
    Serial.println("Moderate.");
  }
  else if(value > 30)
  {
    Serial.println("Loud.");
  }

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
  Serial.print(distance1);


  Serial.print("      Distance 2: ");
  Serial.println(distance2);

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(euler.x());
  Serial.print(" Y: ");
  Serial.print(euler.y());
  Serial.print(" Z: ");
  Serial.println(euler.z());
  //Serial.println("\t\t");
  delay(BNO055_SAMPLERATE_DELAY_MS);
}

// pin's state.
void soundISR()
{
  int pin_val;

  pin_val = digitalRead(PIN_GATE_IN);
  digitalWrite(PIN_LED_OUT, pin_val);   
}
