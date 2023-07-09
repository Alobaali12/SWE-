#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <MadgwickAHRS.h>

#define SAMPLE_RATE 100

unsigned long microsPerReading, microsPrevious;
float accelScale, gyroScale;

Madgwick filter;

Adafruit_BNO055 bno = Adafruit_BNO055();

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  bno.begin();
  bno.setExtCrystalUse(true);

  filter.begin(SAMPLE_RATE); // Set filter sample rate in Hz (adjust as needed)
}

void loop()
{
  float roll, pitch, heading;

  sensors_event_t event;
  bno.getEvent(&event);

  // Get accelerometer, gyroscope, and magnetometer data
  sensors_vec_t accelerometer, gyroscope, magnetometer;
  bno.getEvent(&event);
  
  /*
  accelerometer = event.acceleration;
  gyroscope = event.gyro;
  magnetometer = event.magnetic;

  // Update filter with sensor data
  filter.update( gyroscope.x, gyroscope.y, gyroscope.z, accelerometer.x, accelerometer.y, accelerometer.z,
                magnetometer.x, magnetometer.y, magnetometer.z);

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
    */



}
