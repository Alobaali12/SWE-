#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include "SparkFun_VL53L1X.h"
#include <Servo.h>

#define BUTTON_PIN 3
#define WHITE_LED 4 // Calibration Indicator
#define BAUD_RATE 9600 // Serial Baud rate
#define BUFFER_SIZE 5       // nth Order velocity buffer
#define BNO055_MULTIPLEXER_PIN 3 // index of the IMU in the multiplexer
#define NUM_DIST_SENSORS_SIZE 3
#define PIN_ANALOG_IN A0
#define FILTER_READINGS 1 // (1 means returned filtered data, 0 means raw data) Applies a Low pass filter for displacement data, and a FIFO running average smoothing for velocity data.
#define DEBUG_MODE 0 // 1 to debug, 0 to run normal system operations
#define DEBUG_TIME_DELAY 0 // in millis 
#define CALIBRATION_STATUS 0x03 // Fully Calibrated
#define START_DELAY 20000 // delay after calibration is done, system starts
#define CALIBRATION 0 // 1 (Do calibrate IMU) , 0 no calibration.
#define Rad2Deg 180.0 / PI
#define TCA9548A_ADDRESS 0x70  // TCA9548A multiplexer address (adjust as needed)
#define ServoAngle_Compensator -43  // Adjust the inial angle of the servo
#define Angle_Compensator 9    // Compensate the mounting of the IMU
#define distance0_Compensator 18 //[mm] adjust the distance due to the mounting of the sensor  
#define distance1_Compensator 15 //[mm] adjust the distance due to the mounting of the sensor
#define SERVO_PIN 9 

// -------------------------------------- System Objects --------------------------------------- //  
Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28, &Wire); /// IMU

// -------------------------------------- Distance Sensors --------------------------------------- //  
/*  1 & 2: [CTWD: Distance between nozel tip and weldbead "Along the nozel direction !!!" ]
    3: [HD: Horizontal distance for velocity measurements]
*/

SFEVL53L1X DIST_SENSORS[NUM_DIST_SENSORS_SIZE]; // Create an array to store the VL53L4CD sensor objects
// EMA_Filter DIST_FILTER[NUM_DIST_SENSORS_SIZE];

// Velocity_FIFO Sensor_Velocity; // Velocity Object, horizontal velcoity "Differentiation of displacement signal"

int DISTANCE[NUM_DIST_SENSORS_SIZE];  // Create an array of integers distances

// -------------------------------------- Servo --------------------------------------- //  
Servo servoMotor;
float IMU_Angle = 0.00;
float Working_Angle = 0.00;
float CTWD = 0.00;

float Roll = 0.00;
float Pitch = 0.00;
float Yaw = 0.00;

float Pitch_Calibration = 0.00;
float Roll_Calibration = 0.00;
float Yaw_Calibration = 0.00;

// -------------------------------------- Button Debouncing --------------------------------------- //  
int buttonState = 0;     // Variable to store the button state
int lastButtonState = 0; // Variable to store the previous button state

// -------------------------------------- Functions --------------------------------------- //  

// TCA9548A multiplexer 
void TCA9548A(uint8_t bus){
  if (bus > 7) return; // Multiplexer maximum ports: 8 aka starting from 0.
  Wire.beginTransmission(TCA9548A_ADDRESS);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

int Get_Sound(void) {return analogRead(PIN_ANALOG_IN);}



// -------------------------------------- Void Setup --------------------------------------- // 
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
      //Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
      while (1)
        ;
    }
    //Serial.println("Sensor online!");
  }
  
  TCA9548A(BNO055_MULTIPLEXER_PIN);
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    //Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  //Serial.println("BNO online!");
  servoMotor.attach(9);

  pinMode(BUTTON_PIN, INPUT);

}

// -------------------------------------- Void Loop --------------------------------------- // 

void loop() {
 TCA9548A(BNO055_MULTIPLEXER_PIN);

  sensors_event_t event;
  bno.getEvent(&event);

  sensors_event_t quaternionEvent;
  bno.getEvent(&quaternionEvent, Adafruit_BNO055::VECTOR_QUATERNION);

  // Map the orientation data to servo angles (adjust as needed)
  if(event.orientation.z < 0){
    IMU_Angle = 180 + event.orientation.z;
  }
  else {
    IMU_Angle = -180 + event.orientation.z;
  }

  float servoAngle = map(IMU_Angle, -90, 90, 0, 200);
  servoAngle = constrain(servoAngle, 0, 250);

  servoMotor.write(servoAngle + ServoAngle_Compensator);

  //servoMotor.write(180);

  Roll = event.orientation.z;

  // Serial.print("Orientation z: ");
  // Serial.print(IMU_Angle - Angle_Compensator);
  // Serial.print("     Servo Angle: ");
  // Serial.print(servoAngle);
  // Serial.print("     Roll: ");
  // Serial.println(Roll);
  
  buttonState = digitalRead(BUTTON_PIN); // Read the state of the button

  // Check if the button is pressed
  if (buttonState == HIGH && lastButtonState == LOW) {
    Pitch_Calibration = event.orientation.z;
    Roll_Calibration = event.orientation.y;
    Yaw_Calibration = event.orientation.x;
  }

  lastButtonState = buttonState; // Save the current button state for the next iteration

  Roll = event.orientation.y - Roll_Calibration;
  Pitch = event.orientation.z - Pitch_Calibration;
  Yaw = event.orientation.x - Yaw_Calibration;

  Serial.print("Time: ");
  Serial.print((float) millis()/1000, 3); // time in seconds
  Serial.print("w: ");
  Serial.print(quaternionEvent.quat.w);
  Serial.print("x: ");
  Serial.print(quaternionEvent.quat.x);
  Serial.print("y: ");
  Serial.print(quaternionEvent.quat.y);
  Serial.print("z: ");
  Serial.print(quaternionEvent.quat.z);
  Serial.print("Roll: ");
  Serial.print(Roll  );
  Serial.print("  Pitch: ");
  Serial.print(Pitch  );
  Serial.print("  Yaw: ");
  Serial.print(Yaw  );
  Serial.print("  Sound: ");
  Serial.println(Get_Sound());

// void Send_Data_To_Port(void)
// {
//   Auto_Calibrate();
//   Serial.print((float) millis()/1000, 3); // time in seconds
//   Serial.print(", ");
//   TCA9548A(BNO055_MULTIPLEXER_PIN); // look up for IMU first.
//   IMU.PrintIMUData();
//   Serial.print(", ");
//   Serial.print(Get_CTWD());
//   Serial.print(", ");
//   Serial.print(Get_HD()); // Bug !!! only works if Get_CTWD is called first.
//   Serial.print(", ");
//   Serial.print(Get_H_Velocity(Get_HD()),4); // bug delta T
//   Serial.print(", ");
//   Serial.println(Get_Sound());
//   delay(BNO055_SAMPLERATE_DELAY_MS);
// }


}






















