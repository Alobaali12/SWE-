void setup() {
  Serial.begin(9600); // Initialize the serial communication
  randomSeed(analogRead(0)); // Seed the random number generator
}

void loop() {
  float x = float (random(-100, 100))/100; // Generate random number for x (-1 to 1)
  float y = float (random(-100, 100))/100;  // Generate random number for y (-1 to 1)
  float z = float (random(-100, 100))/100; // Generate random number for z (-1 to 1)
  float f = float (random(-100, 100))/100;  // Generate random number for f (-1 to 1)

  // Print the quaternion values
  Serial.print(x, 2); // Print with 2 decimal places
  Serial.print(",");
  Serial.print(y, 2);
  Serial.print(",");
  Serial.print(z, 2);
  Serial.print(",");
  Serial.println(f, 2);

  delay(1000); // Delay for 1 second before generating the next set of random numbers
}
