#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include "SparkFun_VL53L1X.h"

#define BUTTON_PIN 3
#define PIN_CNTRL 2 // Controls N-Channel MOSFETs Gate Switching
#define BAUD_RATE 115200 // Serial Baud rate
/* Set the delay between fresh samples */
#define BUFFER_SIZE 5       // nth Order velocity buffer
#define BNO055_SAMPLERATE_DELAY_MS (10) // Ask Carlos ???
#define NUM_DIST_SENSORS_SIZE 2
#define PIN_ANALOG_IN A0
#define FILTER_READINGS 1 // Applies a Low pass filter for displacement data, and a FIFO running average smoothing for velocity data.
#define DEBUG_MODE 0 // 1 to debug, 0 to run normal system operations
#define DEBUG_TIME_DELAY 0 // in millis 
#define SENSOR_ADDRESS_CHANGE 0 // change sensor zero address
#define SENS_I2C_ADDRESS 0x54


typedef struct
{
  volatile float constRoll = 0;
  volatile float constPitch = 0;
  volatile float constYaw = 0;
}Calibration_Data;

class Velocity_FIFO {
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
      float velocity = (currentDisplacement - previousDisplacement) / (elapsedTime/1000.00); // in mm/sec
      
      // Filter the velocity to reduce noise
      filteredVelocity = (VELOCITY_FILTER * velocity) + ((1 - VELOCITY_FILTER) * filteredVelocity);
      
      // Update variables for the next iteration
      previousTime = currentTime;
      previousDisplacement = currentDisplacement;
    }
    
    Insert_Velocities(filteredVelocity); // Add current velocity to buffer 
    return Get_Velocity_Average();
}

private:
    float Velocity_Buffer[BUFFER_SIZE];
    int currentIndex = 0;
    int itemCount = 0;
    const float VELOCITY_FILTER = 0.6;         // Velocity filtering factor
    unsigned long previousTime = 0;
    float previousDisplacement = 0;
    float filteredVelocity = 0;
    const unsigned long SAMPLE_INTERVAL = 10;  // Sampling interval in milliseconds
};

class EMA_Filter // Exponential Moving Average filter, eliminates high frequency noise 
{
  public:
  unsigned long Current_Data = 0;

  int Get_Raw_Data(void)
  {
    return Current_Data;
  }
  float Get_Filtered_Data(void)
  {
    Current_Data = alpha * Current_Data + (1-alpha) * Prev_data;
    Prev_data = Current_Data;
    return Current_Data;
  }

  private:
  const float alpha = 0.2;  // EMA filter coefficient, controls signal time constant.
  unsigned long Prev_data = 0;
};

// -------------------------------------- System Objects --------------------------------------- //  
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire); /// IMU

/*  1: [CTWD: Distance between nozel tip and weldbead "Along the nozel direction !!!" ]
    2: [HD: Horizontal distance for velocity measurements]
*/
SFEVL53L1X DIST_SENSORS[NUM_DIST_SENSORS_SIZE];

Calibration_Data Calibration; // Calibration data structure
Velocity_FIFO Sensor_Velocity; // Velocity Object, horizontal velcoity "Differentiation of displacement signal"

// -------------------------------------- Button Debouncing --------------------------------------- //  
unsigned long debounceDelay = 200;
unsigned long lastDebounceTime = 0;

/*
// Carlos variables
float Roll = 0;
float Pitch = 0;
float Yaw = 0;
int calibration = 19;  // Ask Carlos ???
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
*/
// -------------------------------------- DAQ --------------------------------------- // 
unsigned long timer = 0;    // used to check current time [microseconds]
long loopTime = 0;       // default time between updates, but will be set in python Code [microseconds]
bool initLooptime = false;  // boolean (T/F) to check if loop time has already been set
bool stopProgram = false;


void System_Initialization(void);
void Serial_Start(int baud); // Start serial communication
void Init_BNO055(Adafruit_BNO055& bno); // IMU Sensor initialization 
void Init_I2C(void); // Using default pins A4 (SDA) amd A5 (SCL)
void Init_Distance_Sensor(SFEVL53L1X* distanceSensor, int Sens_num); // Intialize Distance Sensors via default I2C
void Init_All_Dist_Sensors(void);
void Init_Button(int buttonPin); // Sets Digital pin as Input held at Vcc rail.
void Init_timer(unsigned long timer); // Carlos timer intialization
void Init_Sound_Detector(void);
void Init_PIN_CNTRL(void); // N-Channel MOSFETS gate contrl
void SET_I2C_ADDRESS(SFEVL53L1X* distanceSensor,int Address);
void ENABLE_PIN_CNTRL(void);

int Get_Sound(void); // Currently configured to return analog data from pin A0.
imu::Vector<3> Get_Euler_Vector(void); //BNO055: A vector containing orientation data, pitch, yaw and roll
float Get_Pitch(void); // BNO055 Pitch angle in degrees 
float Get_Roll(void); // Roll angle degrees   
float Get_Yaw(void); // Yaw angle degrees
float Get_Displacement(SFEVL53L1X& distanceSensor);
float Get_CTWD(void); // Returns "filtered" CTWD distance defined earlier, in mm.
float Get_HD(void); // Returns "filtered" Horizontal Displacement, in mm.
float Get_H_Velocity(float displacement); // Horizontal velocity 
void Auto_Calibrate(void);
void Read_Serial(void);
void Print_Excel_Columns(void);
void CNTRL_DIST_SEN_I2C_BUS_COLLISION(void); // In house circuit design, intializes distance sensors in a specific sequence
void Send_Data_To_Port(void);
void Debug_System(void);
void Run(void);

void setup(void)
{
  Serial.begin(BAUD_RATE);  // begin serial communcation
  System_Initialization();
}


void loop()
{
    Run(); // Runs sensor box system.
}


/* // Raw Data System Debugging
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

  DIST_SENSORS[0].startRanging(); //Write configuration bytes to initiate measurement
  while (! DIST_SENSORS[0].checkForDataReady())
  {
    delay(1);
  }
  int distance1 =  DIST_SENSORS[0].getDistance(); //Get the result of the measurement from the sensor
   DIST_SENSORS[0].clearInterrupt();
   DIST_SENSORS[0].stopRanging();

   DIST_SENSORS[1].startRanging(); //Write configuration bytes to initiate measurement
  while (! DIST_SENSORS[1].checkForDataReady())
  {
    delay(1);
  }
  int distance2 = 1* DIST_SENSORS[1].getDistance()   + (0)*distance2; //Get the result of the measurement from the sensor
   DIST_SENSORS[1].clearInterrupt();
   DIST_SENSORS[1].stopRanging();

  Serial.print("Distance 1: ");
  Serial.print(distance1);


  Serial.print("      Distance 2: ");
  Serial.println(distance2);

  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

 
  Serial.print("X: ");
  Serial.print(euler.x());
  Serial.print(" Y: ");
  Serial.print(euler.y());
  Serial.print(" Z: ");
  Serial.println(euler.z());
  //Serial.println("\t\t");
  delay(BNO055_SAMPLERATE_DELAY_MS);
}
*/

void Run(void)
{
 (DEBUG_MODE) ? Debug_System() : Send_Data_To_Port();
}

void Init_PIN_CNTRL(void)
{
  pinMode(PIN_CNTRL, OUTPUT);
  digitalWrite(PIN_CNTRL, LOW);
}

void ENABLE_PIN_CNTRL(void)
{
  delay(10);
  digitalWrite(PIN_CNTRL, HIGH);
}

void SET_I2C_ADDRESS(SFEVL53L1X* distanceSensor,int Address)
{
  (DEBUG_MODE) ? Serial.println("Setting Sensor 1 address: ") : 0;
  distanceSensor[SENSOR_ADDRESS_CHANGE].setI2CAddress(Address);
}

void Send_Data_To_Port(void)
{
  Auto_Calibrate();
  Serial.print((float) millis()/1000, 3); // time in seconds
  Serial.print(", ");
  Serial.print(Get_Pitch());
  Serial.print(", ");
  Serial.print(Get_Roll());
  Serial.print(", ");
  Serial.print(Get_Yaw());
  Serial.print(", ");
  Serial.print(Get_CTWD());
  Serial.print(", ");
  Serial.print(Get_HD());
  Serial.print(", ");
  Serial.print(Get_H_Velocity(Get_HD()),4);
  Serial.print(", ");
  Serial.println(Get_Sound());
  delay(BNO055_SAMPLERATE_DELAY_MS);
}

void Debug_System(void)
{
  Auto_Calibrate();
  Serial.print("Time: ");
  Serial.println(millis()); // Time in milli seconds
  Serial.print("Pitch: ");
  Serial.println(Get_Pitch());
  Serial.print("Roll: ");
  Serial.println(Get_Roll());
  Serial.print("Yaw: ");
  Serial.println(Get_Yaw());
  Serial.print("CTWD: ");
  Serial.println(Get_CTWD());
  Serial.print("HD: ");
  Serial.println(Get_HD());
  Serial.print("Velocity: ");
  Serial.println(Get_H_Velocity(Get_HD()));
  Serial.print("Calibration Data (Pitch, Yaw , Roll): ");
  Serial.print(Calibration.constPitch);
  Serial.print(", ");
  Serial.print(Calibration.constYaw);
  Serial.print(", ");
  Serial.println(Calibration.constRoll);
  Serial.print("Button Status: ");
  Serial.println(digitalRead(BUTTON_PIN));
  Serial.print("Microphone: ");
  Serial.println(Get_Sound());
  delay(DEBUG_TIME_DELAY);
}

void System_Initialization(void)
{
  (DEBUG_MODE) ? Serial.println("Starting System Initalization ...") : 0;
//  Serial_Start(BAUD_RATE); // BUG !!!
  Init_I2C();
  Init_BNO055(bno);
  Init_All_Dist_Sensors();
  Init_Button(BUTTON_PIN);
  Init_timer(timer);
  (DEBUG_MODE) ?  (void) Serial.println("ALL SENSORS INITIALIZED ENTERING DEBUGGING MODE") : Print_Excel_Columns();
}

void Init_All_Dist_Sensors(void)
{
  CNTRL_DIST_SEN_I2C_BUS_COLLISION();
}

void CNTRL_DIST_SEN_I2C_BUS_COLLISION(void)
{
  Init_PIN_CNTRL();
  SET_I2C_ADDRESS(DIST_SENSORS, SENS_I2C_ADDRESS);
  Init_Distance_Sensor(DIST_SENSORS, 0); // Initalize sensor 1
  (DEBUG_MODE) ? Serial.println("Sensor 1 initialized.") : 0;
  ENABLE_PIN_CNTRL();
  Init_Distance_Sensor(DIST_SENSORS, 1); // Initalize sensor 2
  (DEBUG_MODE) ?  Serial.println("Sensor 2 initialized.") : 0;
  
}

// ---------------------------------- Distance sensor ---------------------------------- //
 /* Initialise Distance Sensors */
void Init_Distance_Sensor(SFEVL53L1X* distanceSensor, int Sens_num)
{
      if (distanceSensor[Sens_num].begin() != 0) //Begin returns 0 on a good init
     {
      Serial.println("Sensor " + String(Sens_num + 1) + " failed to begin. Please check wiring. Freezing...");
    while (1);
     }
}


void Print_Excel_Columns(void)
{
  Serial.println("Time,Pitch,Roll,Yaw,CTWD,HD,HV,Mic");
}
// Carlos function
void Read_Serial(void)
{
  if (Serial.available() > 0) {       // if data is available
    String str = Serial.readStringUntil('\n');
    readFromPC(str);
  }
}

float Get_H_Velocity(float displacement)
{
  return Sensor_Velocity.Smooth_Velocity(displacement); // filtered velocity
}

float Get_HD(void)
{
  return Get_Displacement(DIST_SENSORS[1]);
}

float Get_CTWD(void)
{
   return Get_Displacement(DIST_SENSORS[0]);
}

float Get_Displacement(SFEVL53L1X& distanceSensor)
{
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  EMA_Filter distance;
  distance.Current_Data = distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  return (FILTER_READINGS) ? distance.Get_Filtered_Data() : distance.Get_Raw_Data();
}

// Check vectors will carlos, pitch, roll and yaw definition !!!
float Get_Pitch(void) 
{
    return bno.getVector(Adafruit_BNO055::VECTOR_EULER).x() - Calibration.constPitch;
}

float Get_Roll(void)
{
   return bno.getVector(Adafruit_BNO055::VECTOR_EULER).y() - Calibration.constRoll;
}

float Get_Yaw(void)
{
   return bno.getVector(Adafruit_BNO055::VECTOR_EULER).z() - Calibration.constYaw; 
}

imu::Vector<3> Get_Euler_Vector(void) 
{
  return bno.getVector(Adafruit_BNO055::VECTOR_EULER); // Return Euler Vector
} 

int Get_Sound(void) {return analogRead(PIN_ANALOG_IN);}
// -------------------------------------- timer --------------------------------------- // 
void Init_timer(unsigned long timer)
{
    timer = micros();
}
// -------------------------------------- Button -------------------------------------- //
  // initialize the pushbutton pin as an input:
void Init_Button(int buttonPin)
{
  pinMode(buttonPin, INPUT_PULLUP);
 // digitalWrite(buttonPin, HIGH);
 // attachInterrupt(digitalPinToInterrupt(buttonPin), Auto_Calibrate, FALLING);
}

void Auto_Calibrate(void)
{
 // (DEBUG_MODE) ? Serial.println("Button Interrupt Handler Accessed.") : 0;
  if(!digitalRead(BUTTON_PIN))
  {
      if((millis() - lastDebounceTime) > debounceDelay)
    {
    Calibration.constPitch = bno.getVector(Adafruit_BNO055::VECTOR_EULER).x();
    Calibration.constRoll = bno.getVector(Adafruit_BNO055::VECTOR_EULER).y();
    Calibration.constYaw = bno.getVector(Adafruit_BNO055::VECTOR_EULER).z();
    (DEBUG_MODE) ? Serial.println("Button is pressed") : 0;
    }
  lastDebounceTime = millis();  
  }

}

// ---------------------------------------- I2C ---------------------------------------- //
  /* Initialise I2C */
void Init_I2C(void)
{
   Wire.begin();
}
// ---------------------------------------- IMU ---------------------------------------- //
  /* Initialise the IMU sensor */
void Init_BNO055(Adafruit_BNO055& bno)
{
  (DEBUG_MODE) ? Serial.println("Initializing IMU") : 0;
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  bno.setExtCrystalUse(true);
  (DEBUG_MODE) ? Serial.println("IMU Succefully initialized") : 0;  
}
// ---------------------------------------- Serial ---------------------------------------- //
  /* Initialise Serial */
void Serial_Start(int baud)
{
  Serial.begin(baud);
}

/* ---------------------------------------- Carlos functions ---------------------------------------- */

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

