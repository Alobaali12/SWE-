#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define BNO055_SAMPLERATE_DELAY_MS (0)
#define BUTTON_PIN 3

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

bool buttonPressed = false;
imu::Quaternion initialOrientation;
imu::Quaternion offsetOrientation;

void displaySensorDetails(void)
{
  sensor_t sensor;
  bno.getSensor(&sensor);
  delay(500);
}

int Counter = 0;
void setup(void)
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(9600);

  if (!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);

  bno.setExtCrystalUse(true);
  bno.setMode(OPERATION_MODE_NDOF_FMC_OFF);
  displaySensorDetails();
}

void loop(void)
{
  sensors_event_t event;
  bno.getEvent(&event);

  if (!digitalRead(BUTTON_PIN)) {
    Counter = 1;

    if (!buttonPressed) {
      buttonPressed = true;
      initialOrientation = bno.getQuat();
      offsetOrientation = initialOrientation.conjugate();
    }
  } else {
    buttonPressed = false;
  }

  imu::Quaternion currentOrientation = bno.getQuat();
  imu::Quaternion relativeOrientation = currentOrientation * offsetOrientation;

  if(Counter == 1)
  {
    Serial.print("0");
    Serial.print(F(", "));
    Serial.print((float)relativeOrientation.w(), 4);
    Serial.print(F(", "));
    Serial.print((float)relativeOrientation.x(), 4);
    Serial.print(F(", "));
    Serial.print((float)relativeOrientation.y(), 4);
    Serial.print(F(", "));
    Serial.print((float)relativeOrientation.z(), 4);
    Serial.print(F(", "));
    uint8_t sys, gyro, accel, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    Serial.print(sys, DEC);
    Serial.print(F(", "));
    Serial.print(gyro, DEC);
    Serial.print(F(", "));
    Serial.print(accel, DEC);
    Serial.print(F(", "));
    Serial.print(mag, DEC);
    Serial.print(F(", "));
    Serial.print("0");
    Serial.print(F(", "));
    Serial.print("0");
    Serial.print(F(", "));
    Serial.println("0");
  }
  



  delay(BNO055_SAMPLERATE_DELAY_MS);
}
