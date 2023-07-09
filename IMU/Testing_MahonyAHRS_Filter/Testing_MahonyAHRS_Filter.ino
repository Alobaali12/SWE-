#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <math.h>
#include <MadgwickAHRS.h>

#define SAMPLE_RATE 100

Madgwick filter;
unsigned long microsPerReading, microsPrevious;
float accelScale, gyroScale;


#define SAMPLE_PERIOD (0.01f) // replace this with actual sample period

Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  Serial.begin(9600);
  delay(1000);

  // Initialize BNO055 sensor
  if (!bno.begin())
  {
    Serial.println("Failed to initialize BNO055 sensor!");
    while (1);
  }
  // Set operation mode to Fusion mode (NDOF)
  bno.setMode(OPERATION_MODE_NDOF);

  // Madgwick filter
  filter.begin(SAMPLE_RATE);
    // initialize variables to pace updates to correct rate
  microsPerReading = 1000000 / SAMPLE_RATE;
  microsPrevious = micros();
}

void loop() {

  float roll, pitch, heading;
  unsigned long microsNow;

  // check if it's time to read data and update the filter
  microsNow = micros();
  if (microsNow - microsPrevious >= microsPerReading) {

    // read raw data from CurieIMU
    CurieIMU.readMotionSensor(aix, aiy, aiz, gix, giy, giz);

    // convert from raw data to gravity and degrees/second units
    ax = convertRawAcceleration(aix);
    ay = convertRawAcceleration(aiy);
    az = convertRawAcceleration(aiz);
    gx = convertRawGyro(gix);
    gy = convertRawGyro(giy);
    gz = convertRawGyro(giz);

    // update the filter, which computes orientation
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // print the heading, pitch and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
    Serial.print("Orientation: ");
    Serial.print(heading);
    Serial.print(" ");
    Serial.print(pitch);
    Serial.print(" ");
    Serial.println(roll);

    // increment previous time, so we keep proper pace
    microsPrevious = microsPrevious + microsPerReading;
  }
}
