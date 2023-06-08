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


// Include the Stepper library:
#include <Stepper.h>

const int PAUSE_Pin = 2; // Pause sysem
const int ReverseDir_Pin = 3; // Reverse stepper motor direction
volatile bool REVERSE_DIR_FLAG = true;
volatile bool PAUSE_FLAG = true;
unsigned long debounceDelay = 400;
unsigned long lastDebounceTime = 0;
// Define number of steps per revolution:
const int stepsPerRevolution = 360/1.8;
const int Steps = 200;
//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3
#define BUFFER_SIZE 1 // Velcoity buffer
// Constants
const unsigned long SAMPLE_INTERVAL = 10;  // Sampling interval in milliseconds
const float VELOCITY_FILTER = 0.7;         // Velocity filtering factor

// Variables
unsigned long previousTime = 0;
float previousDisplacement = 0;
float filteredVelocity = 0;


SFEVL53L1X distanceSensor;
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);


class Velocity_FIFO {
private:
  float Velocity_Buffer[BUFFER_SIZE];
  int currentIndex = 0;
  int itemCount = 0;

public:
  Velocity_FIFO() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      Velocity_Buffer[i] = 0;
    }
  }

  void Insert_Velocities(float leftVelocity) {
    
    if (itemCount < BUFFER_SIZE) {
      Velocity_Buffer[currentIndex] = leftVelocity;
      currentIndex = (currentIndex + 1) % BUFFER_SIZE;
      itemCount++;
    } else {
      // Buffer is full, shift the data
      for (int i = 1; i < BUFFER_SIZE; i++) {
        Velocity_Buffer[i - 1] = Velocity_Buffer[i];
      }
      Velocity_Buffer[BUFFER_SIZE - 1] = leftVelocity;
    }
  }

  float Get_Velocity_Average() {
    float sum = 0;

    for (int i = 0; i < itemCount; i++) {
      sum += Velocity_Buffer[i];
    }
    return sum / itemCount;
  }

// Function to calculate linear velocity
float Smooth_Velocity(float currentDisplacement) {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - previousTime;
  
  if (elapsedTime >= SAMPLE_INTERVAL) {
    // Calculate velocity using differentiation
    float velocity = (currentDisplacement - previousDisplacement) / (elapsedTime / 1000.0);
    
    // Filter the velocity to reduce noise
    filteredVelocity = (VELOCITY_FILTER * velocity) + ((1 - VELOCITY_FILTER) * filteredVelocity);
    
    // Update variables for the next iteration
    previousTime = currentTime;
    previousDisplacement = currentDisplacement;
  }
  
  Insert_Velocities(filteredVelocity); // Add current velocity to buffer 
  return Get_Velocity_Average();
}


};

Velocity_FIFO Sensor_Velocity; // Velocity Object
Stepper myStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);

int counter= 0;
int Speed = 25; //200 RPM <--> 2.5 cm/s

void Drive_Stepper_Motor(void);

void setup(void)
{
  Serial.begin(115200);
  Wire.begin();
  myStepper.setSpeed(Speed);
  pinMode(PAUSE_Pin, INPUT_PULLUP);
  pinMode(ReverseDir_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PAUSE_Pin), Pause_Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(ReverseDir_Pin), Reverse_Interrupt, FALLING);
  Serial.println("Starting ....");
  delay(1000);

  Serial.println("VL53L1X Qwiic Test");

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }                                                      
  Serial.println("Sensor online!");
  distanceSensor.setDistanceModeShort();  //Set to 1.3 meter max range
  Serial.println("Speed: " + String(Speed));
  Serial.println("Time (Sec), Distance (m), Velocity (m/s)");
}

void loop(void)
{
    if(!PAUSE_FLAG)
    {
  //   Serial.println("System resumed ");
      Drive_Stepper_Motor();
      // Perform regular program tasks here
      // Step one revolution in one direction:
     distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
      while (!distanceSensor.checkForDataReady())
      {
        // delay(1);
         Drive_Stepper_Motor();
      }
      
     float distance = static_cast<float>(distanceSensor.getDistance())/1000; //Get the result of the measurement from the sensor
      float velocity = Sensor_Velocity.Smooth_Velocity(distance);
      distanceSensor.clearInterrupt();
      distanceSensor.stopRanging();
      Serial.print(millis());      
      Serial.print(",");
      Serial.print(distance,4);
      Serial.print(",");
      Serial.println(velocity, 4);
      
    }
    else
    {
   //  Serial.println("System Paused ");
    }
      


}

void Pause_Interrupt() {
  if((millis() - lastDebounceTime) > debounceDelay)
  {
    PAUSE_FLAG ^= true; // toggle flag
  //  counter++;
  }
 lastDebounceTime = millis();  

}

void Reverse_Interrupt()
{
    if((millis() - lastDebounceTime) > debounceDelay)
  {
    REVERSE_DIR_FLAG ^= true;
    //  counter++;
  }
  lastDebounceTime = millis();  
}

void Drive_Stepper_Motor(void)
{
  if(!REVERSE_DIR_FLAG)
      {
    //    Serial.println(" Direction up ");
    myStepper.setSpeed(80);
        myStepper.step(-int(Steps));
      }
      else
      {
     //   Serial.println(" Direction Reversed ");
        myStepper.step(-int(Steps));
      }
      
}


