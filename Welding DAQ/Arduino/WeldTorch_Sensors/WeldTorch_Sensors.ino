// ---------------------------------------- IMU ---------------------------------------- //
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (0)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

float Roll = 0.00;
float Pitch = 0.00;
float Yaw = 0.00;

// ---------------------------------- Distance sensor ---------------------------------- //
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

SFEVL53L1X distanceSensor;
int calibration = 19;

// -------------------------------------- Button -------------------------------------- //
const int buttonPin = 2;  // the number of the pushbutton pin
int buttonState = 0;  // variable for reading the pushbutton status
float constRoll = 0.00;
float constPitch = 0.00;
float constYaw = 0.00;

// -------------------------------------- Speed --------------------------------------- // 
double xPos = 0, yPos = 0, headingVel = 0;
//uint16_t BNO055_SAMPLERATE_DELAY_MS = 10; //how often to read data from the board
uint16_t PRINT_DELAY_MS = 0; // how often to print the data
uint16_t printCount = 0; //counter to avoid printing every 10MS sample

//velocity = accel*dt (dt in seconds)
//position = 0.5*accel*dt^2
double ACCEL_VEL_TRANSITION =  (double)(BNO055_SAMPLERATE_DELAY_MS) / 1000.0;
//double ACCEL_POS_TRANSITION = 0.5 * ACCEL_VEL_TRANSITION * ACCEL_VEL_TRANSITION;
double DEG_2_RAD = 0.01745329251; //trig functions require radians, BNO055 outputs degrees

// -------------------------------------- DAQ --------------------------------------- // 
unsigned long timer = 0;    // used to check current time [microseconds]
long loopTime = 0;       // default time between updates, but will be set in python Code [microseconds]
bool initLooptime = false;  // boolean (T/F) to check if loop time has already been set
bool stopProgram = false;

void setup(void)
{
  Serial.begin(115200);
// ---------------------------------------- IMU ---------------------------------------- //
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  bno.setExtCrystalUse(true);

// ---------------------------------- Distance sensor ---------------------------------- //
  Wire.begin();

  if (distanceSensor.begin() != 0) //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1)
      ;
  }

// -------------------------------------- Button -------------------------------------- //
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

// -------------------------------------- DAQ --------------------------------------- // 
  timer = micros();
  
}

void loop() {
// -------------------------------------- Button -------------------------------------- //
    /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
  constRoll = event.orientation.y;
  constPitch = event.orientation.z;
  constYaw = event.orientation.x;
  }
  
  if (Serial.available() > 0) {       // if data is available
    String str = Serial.readStringUntil('\n');
    readFromPC(str);
  }
   
// -------------------------------------- DAQ --------------------------------------- //  
  if (initLooptime && !stopProgram)      // once loop time has been initialized
  {
  
    timeSync(loopTime);   // sync up time to match data rate
    unsigned long currT = micros();  // get current time
   // ---------------------------------------- IMU ---------------------------------------- //  
  /* Get a new sensor event */
//  sensors_event_t event;
//  bno.getEvent(&event);

  Roll = event.orientation.y-constRoll; //Travel Angle
  Pitch = event.orientation.z-constPitch; //Working Angle
  Yaw = event.orientation.x-constYaw;

// ---------------------------------- Distance sensor ---------------------------------- //
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
//  while (!distanceSensor.checkForDataReady())
//  {
//    delay(1);
//  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  distance = distance-calibration;

// -------------------------------------- Speed --------------------------------------- //

  unsigned long tStart = micros();
  sensors_event_t orientationData , linearAccelData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  //  bno.getEvent(&angVelData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);

  // velocity of sensor in the direction it's facing
  headingVel = ACCEL_VEL_TRANSITION * linearAccelData.acceleration.x * 1000/ cos(DEG_2_RAD * orientationData.orientation.x);  // mm/s

//  if (printCount * BNO055_SAMPLERATE_DELAY_MS >= PRINT_DELAY_MS) {
//
//    printCount = 0;
//  }
//  else {
//    printCount = printCount + 1;
//  }
//
//  while ((micros() - tStart) < (BNO055_SAMPLERATE_DELAY_MS * 1000))
//  {
//    //poll until the next sample is ready
//  }  
  
    sendToPC(&currT);
    sendToPC(&distance);
    sendToPC(&Pitch);
    sendToPC(&Roll);
    sendToPC(&headingVel);
  }


  else if (initLooptime && stopProgram)
  {
  // ---------------------------------------- IMU ---------------------------------------- //  
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  Roll = event.orientation.y-constRoll; //Travel Angle
  Pitch = event.orientation.z-constPitch; //Working Angle
  Yaw = event.orientation.x-constYaw;
  /* Display the floating point data */
  Serial.print("Yaw: ");
  Serial.print(event.orientation.x-constYaw, 4);
  //Serial.print("\tRoll: ");
  Serial.print("\tTravel Angle: ");
  Serial.print(event.orientation.y-constRoll, 4);
  //Serial.print("\tPitch: ");
  Serial.print("\tWorking Angle: ");
  Serial.print(event.orientation.z-constPitch, 4);

  /* New line for the next sample */
  Serial.println("");

  /* Wait the specified delay before requesting nex data */
  //delay(BNO055_SAMPLERATE_DELAY_MS);

// ---------------------------------- Distance sensor ---------------------------------- //
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
//  while (!distanceSensor.checkForDataReady())
//  {
//    delay(1);
//  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  distance = distance-calibration;
  Serial.print("Distance(mm): ");
  Serial.print(distance);
  Serial.println();
  
  // -------------------------------------- Speed --------------------------------------- //

  unsigned long tStart = micros();
  sensors_event_t orientationData , linearAccelData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  //  bno.getEvent(&angVelData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);

//  xPos = xPos + ACCEL_POS_TRANSITION * linearAccelData.acceleration.x;
//  yPos = yPos + ACCEL_POS_TRANSITION * linearAccelData.acceleration.y;

  // velocity of sensor in the direction it's facing
  headingVel = ACCEL_VEL_TRANSITION * linearAccelData.acceleration.x * 1000/ cos(DEG_2_RAD * orientationData.orientation.x);  // mm/s

//  if (printCount * BNO055_SAMPLERATE_DELAY_MS >= PRINT_DELAY_MS) {
    //enough iterations have passed that we can print the latest data
//    Serial.print("Heading: ");
//    Serial.println(orientationData.orientation.x);
////    Serial.print("Position: ");
////    Serial.print(xPos);
////    Serial.print(" , ");
////    Serial.println(yPos);
    Serial.print("Speed: ");
    Serial.println(headingVel);
    Serial.println("-------");

//    printCount = 0;
//  }
//  else {
//    printCount = printCount + 1;
//  }
//
//  while ((micros() - tStart) < (BNO055_SAMPLERATE_DELAY_MS * 1000))
//  {
//    //poll until the next sample is ready
////  }  
//
//  }

}  
}










// -------------------------------------- DAQ --------------------------------------- //
void timeSync(unsigned long deltaT)
{
  unsigned long currTime = micros();  // get current time
  long timeToDelay = deltaT - (currTime - timer); // calculate how much time to delay for [us]
  
  if (timeToDelay > 5000) // if time to delay is large 
  {
    // Split up delay commands into delay(milliseconds)
    delay(timeToDelay / 1000);

    // and delayMicroseconds(microseconds)
    delayMicroseconds(timeToDelay % 1000);
  }
  else if (timeToDelay > 0) // If time to delay is positive and small
  {
    // Use delayMicroseconds command
    delayMicroseconds(timeToDelay);
  }
  else
  {
      // timeToDelay is negative or zero so don't delay at all
  }
  timer = currTime + timeToDelay;
}


void readFromPC(const String input)
{
  int commaIndex = input.indexOf(',');
  char command = input.charAt(commaIndex - 1);
  String data = input.substring(commaIndex + 1);    
  int rate = 0;
  switch(command)
  {
    case 'r':
      // rate command
      rate = data.toInt();
      loopTime = 1000000/rate;         // set loop time in microseconds to 1/frequency sent
      initLooptime = true;             // no longer check for data
      timer = micros();
      break;
    case 's':
      // stop command
      stopProgram = true;
      break;
    case 'f':
      // free run
      initLooptime = true;
      stopProgram = true;
      break;
    default:
    // Otherwise, do nothing
      break;
  
  }

}

// ------------------------------------------------------------------------------------------------------------
// Send Data to PC: Methods to send different types of data to PC
// ------------------------------------------------------------------------------------------------------------

void sendToPC(int* data)
{
  byte* byteData = (byte*)(data);
  Serial.write(byteData, 2);
}

void sendToPC(float* data)
{
  byte* byteData = (byte*)(data);
  Serial.write(byteData, 4);
}
 
void sendToPC(double* data)
{
  byte* byteData = (byte*)(data);
  Serial.write(byteData, 4);
}

void sendToPC(unsigned long* data)
{
  byte* byteData = (byte*)(data);
  Serial.write(byteData, 4);
}
