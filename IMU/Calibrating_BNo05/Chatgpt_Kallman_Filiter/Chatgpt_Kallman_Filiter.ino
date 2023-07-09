#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <KalmanFilter.h>

// Initialize the IMU sensor
Adafruit_BNO055 bno = Adafruit_BNO055();

// Constants for ZUPT algorithm
const float THRESHOLD = 0.1;  // Threshold for detecting zero velocity (adjust as needed)
const int SAMPLE_INTERVAL = 10;  // Time between sensor readings in milliseconds

// Variables for ZUPT algorithm
float velocityX = 0.0;
float velocityY = 0.0;
float velocityZ = 0.0;
float roll = 0.0;
float pitch = 0.0;
unsigned long prevTime = 0;

// Kalman filter variables
KalmanFilter kalmanX;
KalmanFilter kalmanY;
KalmanFilter kalmanZ;

void setup() {
  Serial.begin(9600);

  // Initialize the IMU sensor
  if (!bno.begin())
  {
    Serial.println("Failed to initialize BNO055! Check your wiring and/or device!");
    while (1);
  }

  // Set the IMU sensor mode to NDOF (9 degrees of freedom)
  bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);

  prevTime = millis();

  // Initialize Kalman filters
  kalmanX.init(1.0, 1.0, 0.01);
  kalmanY.init(1.0, 1.0, 0.01);
  kalmanZ.init(1.0, 1.0, 0.01);
}

void loop() {
  // Read the accelerometer and gyroscope data from the IMU
  sensors_event_t event;
  bno.getEvent(&event);

  // Get the current time
  unsigned long currentTime = millis();

  // Calculate the time difference since the last sensor reading
  float dt = (currentTime - prevTime) / 1000.0;  // Convert milliseconds to seconds

  // Calculate roll and pitch angles using the accelerometer and gyroscope
  roll = 0.98 * (roll + event.gyro.roll * dt) + 0.02 * event.acceleration.x;
  pitch = 0.98 * (pitch + event.gyro.pitch * dt) + 0.02 * event.acceleration.y;

  // Calculate the acceleration due to gravity
  float accelerationX = event.acceleration.x - sin(roll) * 9.81;
  float accelerationY = event.acceleration.y + sin(pitch) * 9.81;
  float accelerationZ = event.acceleration.z - cos(roll) * cos(pitch) * 9.81;

  // Apply zero velocity update (ZUPT) if the acceleration is below the threshold
  if (abs(accelerationX) < THRESHOLD &&
      abs(accelerationY) < THRESHOLD &&
      abs(accelerationZ) < THRESHOLD) {
    velocityX = 0.0;
    velocityY = 0.0;
    velocityZ = 0.0;
  } else {
    // Update velocity estimates using the Kalman filter
    velocityX = kalmanX.update(velocityX, accelerationX, dt);
    velocityY = kalmanY.update(velocityY, accelerationY, dt);
    velocityZ = kalmanZ.update(velocityZ, accelerationZ, dt);
  }

  // Print the current velocity readings
  Serial.print("Velocity X: ");
  Serial.print(velocityX);
  Serial.print(" m/s\t");
  Serial.print("Velocity Y: ");
  Serial.print(velocityY);
  Serial.print(" m/s\t");
  Serial.print("Velocity Z: ");
  Serial.print(velocityZ);
  Serial.println(" m/s");

  // Update the previous time
  prevTime = currentTime;

  // Delay for the specified sample interval
  delay(SAMPLE_INTERVAL);
}
