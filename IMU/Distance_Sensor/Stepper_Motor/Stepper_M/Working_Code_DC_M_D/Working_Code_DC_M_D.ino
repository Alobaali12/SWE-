/* Example sketch to control a stepper motor with L298N motor driver, Arduino UNO and Stepper.h library. More info: https://www.makerguides.com */

// Include the Stepper library:
#include <Stepper.h>

const int PAUSE_Pin = 2; // Pause sysem
const int ReverseDir_Pin = 3; // Reverse stepper motor direction
volatile bool REVERSE_DIR_FLAG = false;
volatile bool PAUSE_FLAG = true;
unsigned long debounceDelay = 400;
unsigned long lastDebounceTime = 0;
// Define number of steps per revolution:
const int stepsPerRevolution = 360/1.8;

// Initialize the stepper library on pins 8 through 11:
Stepper myStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);

int counter= 0;

void setup() {
  // Set the motor speed (RPMs):
  Serial.begin(115200);
  myStepper.setSpeed(200);
  pinMode(PAUSE_Pin, INPUT_PULLUP);
  pinMode(ReverseDir_Pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PAUSE_Pin), Pause_Interrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(ReverseDir_Pin), Reverse_Interrupt, FALLING);
    Serial.println("Starting ....");
    delay(1000);
}

void loop() {
  // Step one revolution in one direction:
 // Serial.print("PIN 2 Count: ");

  //Serial.println(counter);
  if(!PAUSE_FLAG)
  {
     Serial.println("System resumed ");
    if(!REVERSE_DIR_FLAG)
    {
       Serial.println(" Direction up ");
       myStepper.step(int(stepsPerRevolution));
    }
    else
    {
      Serial.println(" Direction Reversed ");
       myStepper.step(-int(stepsPerRevolution));
    }
                                      
  }
  else
  {
    Serial.println("System Paused ");
  }
    // debugging freeze
  //  delay(200);
  //for(int i = 0; i < 1000 ; i++) Serial.println("Do nothing .....................");


}

void Pause_Interrupt() {
  if((millis() - lastDebounceTime) > debounceDelay)
  {
    PAUSE_FLAG ^= true;
  //  counter++;
  }
 lastDebounceTime = millis();  

}

void Reverse_Interrupt()
{
    if((millis() - lastDebounceTime) > debounceDelay)
  {
    REVERSE_DIR_FLAG ^= true;
    //  counter++;
  }
  lastDebounceTime = millis();  
}
