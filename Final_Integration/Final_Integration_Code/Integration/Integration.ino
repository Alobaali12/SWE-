#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include "SparkFun_VL53L1X.h"

#define BUTTON_PIN 3
#define WHITE_LED 4 // Calibration Indicator
#define PIN_CNTRL 2 // Controls N-Channel MOSFETs Gate Switching
#define BAUD_RATE 9600 // Serial Baud rate
/* Set the delay between fresh samples */
#define BUFFER_SIZE 5       // nth Order velocity buffer
#define BNO055_SAMPLERATE_DELAY_MS (0) // Ask Carlos ???
#define NUM_DIST_SENSORS_SIZE 2
#define PIN_ANALOG_IN A0
#define FILTER_READINGS 1 // (1 means returned filtered data, 0 means raw data) Applies a Low pass filter for displacement data, and a FIFO running average smoothing for velocity data.
#define DEBUG_MODE 0 // 1 to debug, 0 to run normal system operations
#define DEBUG_TIME_DELAY 0 // in millis 
#define SENSOR_ADDRESS_CHANGE 0 // change sensor zero address
#define SENS_I2C_ADDRESS 0x54
#define CALIBRATION_STATUS 0x03 // Fully Calibrated
#define START_DELAY 20000 // delay after calibration is done, system starts
#define CALIBRATION 1 // 1 (Do calibrate IMU) , 0 no calibration.
#define Rad2Deg 180.0 / PI


typedef struct
{
  // Quaternions 
  volatile float w_calibration = 0;
  volatile float x_calibration = 0;
  volatile float y_calibration = 0;
  volatile float z_calibration = 0;
  // Euler Angles
  volatile float Pitch_Calibration = 0;
  volatile float Roll_Calibration = 0;
  volatile float Yaw_Calibration = 0;

  imu::Quaternion offsetOrientation;

  // IMU Calibration 
  uint8_t System_Calibration = 0;
  uint8_t Accel_Calibration = 0;
  uint8_t Gyro_Calibration = 0;
  uint8_t Mag_Calibration = 0;

}Calibration_Data;

Calibration_Data Calibration; // Calibration data structure

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
    float GetRawW();
    float GetRawX();
    float GetRawY();
    float GetRawZ();
    float GetRawPitch();
    float GetRawYaw();
    float GetRawRoll();
    void PrintIMUData();
    imu::Quaternion Get_Quaternions();

  private:
    Adafruit_BNO055& _imu;
};

IMU_DATA::IMU_DATA(Adafruit_BNO055& imu) : _imu(imu) {}


imu::Quaternion IMU_DATA::Get_Quaternions()
{
  return _imu.getQuat();
}

float IMU_DATA::GetRoll() {
//  imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER); 
  return (_imu.getQuat().toEuler().x() * Rad2Deg) - Calibration.Roll_Calibration ;
}

float IMU_DATA::GetPitch() {
 // imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return (_imu.getQuat().toEuler().y() * Rad2Deg) - Calibration.Pitch_Calibration;
}

float IMU_DATA::GetYaw() {
// imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return (_imu.getQuat().toEuler().z() * Rad2Deg) - Calibration.Yaw_Calibration ;
}

float IMU_DATA::GetW() {
  imu::Quaternion quat = _imu.getQuat();
  quat = quat * Calibration.offsetOrientation;

  return quat.w();
}

float IMU_DATA::GetX() {
  imu::Quaternion quat = _imu.getQuat();
  quat = quat * Calibration.offsetOrientation;
  return quat.x();
}

float IMU_DATA::GetY() {
  imu::Quaternion quat = _imu.getQuat();
  quat = quat * Calibration.offsetOrientation;
  return quat.y();
}

float IMU_DATA::GetZ() {
  imu::Quaternion quat = _imu.getQuat();
  quat = quat * Calibration.offsetOrientation;
  return quat.z();
}

float IMU_DATA::GetRawW() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.w();
}

float IMU_DATA::GetRawX() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.x();
}

float IMU_DATA::GetRawY() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.y();
}

float IMU_DATA::GetRawZ() {
  imu::Quaternion quat = _imu.getQuat();
  return quat.z();
}

float IMU_DATA::GetRawPitch() {
 // imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return (_imu.getQuat().toEuler().y() * Rad2Deg);
}

float IMU_DATA::GetRawYaw() {
 // imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return (_imu.getQuat().toEuler().z() * Rad2Deg);
}

float IMU_DATA::GetRawRoll() {
 // imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
  return (_imu.getQuat().toEuler().x() * Rad2Deg);
}

void IMU_DATA::PrintIMUData() {
  float roll = GetRoll();
  float pitch = GetPitch();
  float yaw = GetYaw();
  float w = GetW();
  float x = GetX();
  float y = GetY();
  float z = GetZ();

//  Serial.print("Quaternion: ");
  Serial.print(w,3);
  Serial.print(", ");
  Serial.print(x,3);
  Serial.print(", ");
  Serial.print(y,3);
  Serial.print(", ");
  Serial.print(z,3);
  Serial.print(", ");
//  Serial.print(" Euler Angles: ");
  Serial.print(pitch);
  Serial.print(", ");
  Serial.print(yaw);
  Serial.print(", ");
  Serial.print(roll);
}


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
  void Insert(int Displacement)
  {
    Current_Data = Displacement;
  }
  int Get_Raw_Data(void)
  {
    return Current_Data;
  }
  int Get_Filtered_Data(void)
  {
    Current_Data = alpha * Current_Data + (1-alpha) * Prev_data;
    Prev_data = Current_Data;
    return Current_Data;
  }

  private:
  const float alpha = 0.286;  // (There is a bug somewhere here it's not working correctly hence (1))EMA filter coefficient, controls signal time constant.
  float Current_Data = 0;
  float Prev_data = 0;
};

// -------------------------------------- System Objects --------------------------------------- //  
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire); /// IMU
IMU_DATA IMU(bno); // Easy IMU data access
/*  1: [CTWD: Distance between nozel tip and weldbead "Along the nozel direction !!!" ]
    2: [HD: Horizontal distance for velocity measurements]
*/
SFEVL53L1X DIST_SENSORS[NUM_DIST_SENSORS_SIZE];
EMA_Filter DIST_FILTER[NUM_DIST_SENSORS_SIZE];

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
bool LED_FLAG = false;


void System_Initialization(void);
void Serial_Start(int baud); // Start serial communication
void Init_BNO055(Adafruit_BNO055& bno); // IMU Sensor initialization 
void Init_I2C(void); // Using default pins A4 (SDA) amd A5 (SCL)
void Init_Distance_Sensor(SFEVL53L1X* distanceSensor, int Sens_num); // Intialize Distance Sensors via default I2C
void Init_All_Dist_Sensors(void);
void Init_Button(int buttonPin); // Sets Digital pin as Input held at Vcc rail.
void Init_Cali_LED(int LED); // Initalizing Calibration LED indicator
void Init_timer(unsigned long timer); // Carlos timer intialization
void Init_Sound_Detector(void);
void Init_PIN_CNTRL(void); // N-Channel MOSFETS gate contrl
void SET_I2C_ADDRESS(SFEVL53L1X* distanceSensor,int Address);
void ENABLE_PIN_CNTRL(void);

int Get_Sound(void); // Currently configured to return analog data from pin A0.
imu::Vector<3> Get_Euler_Vector(void); //BNO055: A vector containing orientation data, pitch, yaw and roll
// Old IMU Methods
float Get_Pitch(void); // BNO055 Pitch angle in degrees 
float Get_Roll(void); // Roll angle degrees   
float Get_Yaw(void); // Yaw angle degrees
float Get_Displacement(SFEVL53L1X& distanceSensor, EMA_Filter& DIST_FILTER);
float Get_CTWD(void); // Returns "filtered" CTWD distance defined earlier, in mm.
float Get_HD(void); // Returns "filtered" Horizontal Displacement, in mm.
float Get_H_Velocity(float displacement); // Horizontal velocity 
void IMU_Calibration(Adafruit_BNO055& bno);
void Print_Calibration_Data(Adafruit_BNO055& bno);
void FLASH_LED(int LED);
void Auto_Calibrate(void);
void Read_Serial(void);
void Print_Excel_Columns(void);
void Get_Calibration_Data(void);
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
  IMU.PrintIMUData();
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
  Serial.print("IMU Data: ");
  IMU.PrintIMUData();
  Serial.println();
  Serial.print("CTWD: ");
  Serial.println(Get_CTWD());
  Serial.print("HD: ");
  Serial.println(Get_HD());
  Serial.print("Velocity: ");
  Serial.println(Get_H_Velocity(Get_HD()));
  Serial.print("Calibration Data ( w, x, y, z, Pitch, Yaw , Roll, Sys, Gyro, Accel, Mag): ");
  Print_Calibration_Data();
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
  Init_Cali_LED(WHITE_LED);
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
  Serial.println("Time,Q_w,Q_x,Q_y,Q_z,Pitch,Yaw,Roll,CTWD,HD,HV,Mic");
}
// Carlos function
void Read_Serial(void)
{
  if (Serial.available() > 0) {       // if data is available
    String str = Serial.readStringUntil('\n');
    readFromPC(str);
  }
}

void Print_Calibration_Data(void)
{

  Serial.print(Calibration.w_calibration);
  Serial.print(", ");
  Serial.print(Calibration.x_calibration);
  Serial.print(", ");
  Serial.print(Calibration.y_calibration);
  Serial.print(", ");
  Serial.print(Calibration.z_calibration);
  Serial.print(", ");
  Serial.print(Calibration.Pitch_Calibration);
  Serial.print(", ");
  Serial.print(Calibration.Yaw_Calibration);
  Serial.print(", ");
  Serial.print(Calibration.Roll_Calibration);
  Serial.print(", ");
  Serial.print(Calibration.System_Calibration);
  Serial.print(", ");
  Serial.print(Calibration.Gyro_Calibration);
  Serial.print(", ");
  Serial.print(Calibration.Accel_Calibration);
  Serial.print(", ");
  Serial.println(Calibration.Mag_Calibration);

}


float Get_H_Velocity(float displacement)
{
  return Sensor_Velocity.Smooth_Velocity(displacement); // filtered velocity
}

float Get_HD(void)
{
  return Get_Displacement(DIST_SENSORS[1], DIST_FILTER[1]);
}

float Get_CTWD(void)
{
   return Get_Displacement(DIST_SENSORS[0], DIST_FILTER[0]);
}

float Get_Displacement(SFEVL53L1X& distanceSensor, EMA_Filter& DIST_FILTER)
{
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  DIST_FILTER.Insert(distanceSensor.getDistance()); // Insert displacement
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  return (FILTER_READINGS) ? DIST_FILTER.Get_Filtered_Data() : DIST_FILTER.Get_Raw_Data();
}

// Old IMU Methods
// Check vectors will carlos, pitch, roll and yaw definition !!!
float Get_Pitch(void) 
{
    return bno.getVector(Adafruit_BNO055::VECTOR_EULER).x() - Calibration.Pitch_Calibration;
}

float Get_Roll(void)
{
   return bno.getVector(Adafruit_BNO055::VECTOR_EULER).y() - Calibration.Roll_Calibration;
}

float Get_Yaw(void)
{
   return bno.getVector(Adafruit_BNO055::VECTOR_EULER).z() - Calibration.Yaw_Calibration; 
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

void Init_Cali_LED(int LED)
{
  pinMode(LED, OUTPUT);
}

void FLASH_LED(int LED)
{
  if(LED_FLAG) 
  {
    digitalWrite(LED, LOW);
    LED_FLAG ^= true; // toggle 
  }
  else
  {
    digitalWrite(LED, HIGH);
    LED_FLAG ^= true; // toggle 
  }
  delay(50);
}

void Auto_Calibrate(void)
{
 // (DEBUG_MODE) ? Serial.println("Button Interrupt Handler Accessed.") : 0;
  if(!digitalRead(BUTTON_PIN))
  {
      if((millis() - lastDebounceTime) > debounceDelay)
    {
      (DEBUG_MODE) ? Serial.println("Button was pressed, starting data calibration") : 0;
      Calibration.offsetOrientation = IMU.Get_Quaternions().conjugate(); // offset orientation is caluclated as the conjugate of current quaternion to perform correct rotation offsetting later.
      // Calibration sequence
      /*
      Calibration.w_calibration = IMU.GetRawW();
      Calibration.x_calibration = IMU.GetRawX();
      Calibration.y_calibration = IMU.GetRawY();
      Calibration.z_calibration = IMU.GetRawZ();
      */
      Calibration.Pitch_Calibration = IMU.GetRawPitch();
      Calibration.Yaw_Calibration = IMU.GetRawYaw();
      Calibration.Roll_Calibration = IMU.GetRawRoll();
    
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
  bno.setMode(OPERATION_MODE_NDOF); // Use all degrees of freedom, Acc. Gyro & Mag  
  (DEBUG_MODE) ? Serial.println("IMU Succefully initialized") : 0;  
  delay(100);

  if(CALIBRATION)
  {
    IMU_Calibration(bno);
    // Look for LED solid white light, means ready to start
    digitalWrite(WHITE_LED, HIGH);
    delay(START_DELAY);
    while(digitalRead(BUTTON_PIN)) Auto_Calibrate(); // wait until button is pressed then flash LED
    digitalWrite(WHITE_LED, LOW); // ready to start
  }


}


void IMU_Calibration(Adafruit_BNO055& bno)
{
  (DEBUG_MODE) ? Serial.println("Starting IMU Calibration ...") : 0; 
  delay(200);
  while( (Calibration.System_Calibration != CALIBRATION_STATUS) || (Calibration.Gyro_Calibration != CALIBRATION_STATUS) || (Calibration.Accel_Calibration != CALIBRATION_STATUS) || (Calibration.Mag_Calibration != CALIBRATION_STATUS)) Get_Calibration_Data(bno);
  (DEBUG_MODE) ? Serial.println("IMU Fully Calibrated Ready to go !!!") : 0;
  digitalWrite(WHITE_LED, LOW); // Turn off Indicator and start system 
}

void Get_Calibration_Data(Adafruit_BNO055& bno)
{
  FLASH_LED(WHITE_LED); 
  bno.getCalibration(&(Calibration.System_Calibration), &(Calibration.Gyro_Calibration), &(Calibration.Accel_Calibration), &(Calibration.Mag_Calibration)); // Get IMU calibration Status
  
  if(DEBUG_MODE)
  {
    Serial.print("Calibration: Sys=");
    Serial.print(Calibration.System_Calibration);
    Serial.print(" Gyro=");
    Serial.print(Calibration.Gyro_Calibration);
    Serial.print(" Accel=");
    Serial.print(Calibration.Accel_Calibration);
    Serial.print(" Mag=");
    Serial.println(Calibration.Mag_Calibration);
    Serial.println("--");
    delay(BNO055_SAMPLERATE_DELAY_MS);
  }

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

