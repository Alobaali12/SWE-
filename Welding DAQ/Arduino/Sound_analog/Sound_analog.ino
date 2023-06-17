// Define hardware connections



void setup()
{
  Serial.begin(115200);
}

void loop() {
  // read the input on analog pin 0:
  float Sound = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  // print out the value you read:
  Serial.println(Sound);
  //delay(500);
}
