#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Wire.h>
#include <Servo.h>
#include "SparkFun_VL53L1X.h"
#include <utility/imumaths.h>

#define NUM_DIST_SENSORS_SIZE 3
#define BNO055_MULTIPLEXER_PIN 3
#define PIN_ANALOG_IN A0
// TCA9548A multiplexer address (adjust as needed)
#define TCA9548A_ADDRESS 0x70
#define ServoAngle_Compensator -13
#define distance0_Compensator 18 //mm
#define distance1_Compensator 15 //mm

// Create an array to store the VL53L4CD sensor objects
SFEVL53L1X DIST_SENSORS[NUM_DIST_SENSORS_SIZE];
// Create an array of integers distances
int DISTANCE[NUM_DIST_SENSORS_SIZE]; 

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28, &Wire);

Servo servoMotor;
float IMU_Angle = 0.00;
float Working_Angle = 0.00;
float CTWD = 0.00;

void TCA9548A(uint8_t bus){
  if (bus > 7) return;
  Wire.beginTransmission(TCA9548A_ADDRESS);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

void setup() {
  // Start serial communication
  Serial.begin(9600);
  // Start I2C communication
  Wire.begin();
  // Initialize the VL53L4CD sensors
  for (int i = 0; i < NUM_DIST_SENSORS_SIZE; i++) {
    //Serial.print(i);
    TCA9548A(i);
    DISTANCE[i] = 0;
    if (DIST_SENSORS[i].begin() != 0) //Begin returns 0 on a good init
    {
      Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
      while (1)
        ;
    }
    Serial.println("Sensor online!");
    //sensor[i].setI2CAddress(0x29); // Set the sensor's address (0x29, 0x2A, 0x2B)
  }
  
  TCA9548A(BNO055_MULTIPLEXER_PIN);
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  Serial.println("BNO online!");
  servoMotor.attach(9);
}

void loop() {
  for (int i = 0; i < NUM_DIST_SENSORS_SIZE; i++) {
    // Select the current sensor via the multiplexer
    TCA9548A(i);
    if (DIST_SENSORS[i].begin() != 0) //Begin returns 0 on a good init
    {
      Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
      while (1)
        ;
    }
    else {
      // Read the distance measurement
      DISTANCE[i] = DIST_SENSORS[i].getDistance();
      
      // Print the distance for the current sensor
      // Serial.print("Sensor ");
      // Serial.print(i);
      // Serial.print(": ");
      // Serial.print(distance[i]);
      // Serial.println(" mm");
    }
  }

  TCA9548A(BNO055_MULTIPLEXER_PIN);

  sensors_event_t event;
  bno.getEvent(&event);
  
  // Map the orientation data to servo angles (adjust as needed)
  if(event.orientation.z < 0){
    IMU_Angle = 180 + event.orientation.z;
  }
  else {
    IMU_Angle = -180 + event.orientation.z;
  }
  Working_Angle = 90-IMU_Angle;
  float servoAngle = map(IMU_Angle, -90, 90, 0, 180);
  servoAngle = constrain(servoAngle, 0, 180);

  servoMotor.write(servoAngle + ServoAngle_Compensator);

  // Serial.print("Orientation z: ");
  // Serial.print(IMU_Angle);
  // Serial.print("     Servo Angle: ");
  // Serial.println(servoAngle);


  if(DISTANCE[0] > DISTANCE[1]){
    
    CTWD = (DISTANCE[1]+distance1_Compensator)/ sin((90-Working_Angle) * (PI / 180.0) );
    
    Serial.print("Working Angle: ");
    Serial.print(Working_Angle);
    Serial.print("   Distance1: ");
    Serial.print(DISTANCE[1]);
    Serial.print("   CTWD: ");
    Serial.println(CTWD);
  }
  else if(DISTANCE[1] > DISTANCE[0]){
    
    CTWD = (DISTANCE[0]+distance0_Compensator)/ sin(Working_Angle * (PI / 180.0) );
    
    Serial.print("Working Angle: ");
    Serial.print(Working_Angle);
    Serial.print("   Distance0: ");
    Serial.print(DISTANCE[0]);
    Serial.print("   CTWD: ");
    Serial.println(CTWD);
  }
}




