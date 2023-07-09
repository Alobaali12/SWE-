#include <Wire.h> // Required for I2C communication

extern "C" {
#include "VL53L1X_api.h" // Include the ULD driver header file
}

#define I2C_ADDRESS 0x29 // Replace with your VL53L1X sensor I2C address

VL53L1_Dev_t device; // Create a VL53L1X device structure

void setup() {
  Wire.begin(); // Initialize I2C communication
  Wire.setClock(400000); // Set I2C bus speed (400kHz for VL53L1X)

  device.I2cDevAddr = I2C_ADDRESS; // Set the I2C address of the VL53L1X sensor

  // Initialize the VL53L1X sensor
  if (VL53L1_RdWord(&device, VL53L1_IDENTIFICATION__MODEL_ID, 0) == 0xEEAA) {
    Serial.begin(9600);
    Serial.println("VL53L1X sensor initialized successfully!");
  } else {
    Serial.begin(9600);
    Serial.println("Failed to initialize VL53L1X sensor!");
    while (1);
  }
}

void loop() {
  // Read distance measurement from the VL53L1X sensor
  uint16_t distance;
  VL53L1_GetDistance(&device, &distance);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  delay(500);
}
