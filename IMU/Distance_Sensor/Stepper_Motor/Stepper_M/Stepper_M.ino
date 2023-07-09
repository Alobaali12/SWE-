#define IN1 2
#define IN2 3
#define IN3 4
#define IN4 5
int Steps = 0;
boolean Direction = true;
void setup() {
Serial.begin(9600);
pinMode(IN1, OUTPUT);
pinMode(IN2, OUTPUT);
pinMode(IN3, OUTPUT);
pinMode(IN4, OUTPUT);
}
void loop() {
for(int i=0; i<4096; i++){ // What is 4096? You'll have to change things here to do rotational control
stepperFULL(1);
delayMicroseconds(800); // Don't change this number - see if you can find it on the scope! 
}
Direction = !Direction;  // What does this do? 
}

void stepperFULL(int xw) {
for (int x = 0; x < xw; x++) {
switch (Steps) {
case 0:
digitalWrite(IN1, LOW);
digitalWrite(IN2, LOW);
digitalWrite(IN3, LOW);
digitalWrite(IN4, HIGH);
break;
case 1:
digitalWrite(IN1, LOW);
digitalWrite(IN2, LOW);
digitalWrite(IN3, HIGH);
digitalWrite(IN4, LOW);
break;
case 2:
digitalWrite(IN1, LOW);
digitalWrite(IN2, HIGH);
digitalWrite(IN3, LOW);
digitalWrite(IN4, LOW);
break;
case 3:
digitalWrite(IN1, HIGH);
digitalWrite(IN2, LOW);
digitalWrite(IN3, LOW);
digitalWrite(IN4, LOW);
break;
default:
digitalWrite(IN1, LOW);
digitalWrite(IN2, LOW);
digitalWrite(IN3, LOW);
digitalWrite(IN4, LOW);
break;
}
SetDirectionFULL();
}
} // END StepperFull() 

void SetDirectionFULL() {
if (Direction == 1) {
Steps++;
}
if (Direction == 0) {
Steps--;
}
if (Steps > 4) {
Steps = 0;
}
if (Steps < 0) {
Steps = 4;
}
}

/* FILL OUT YOUR CODE HERE FOR HALF STEPS!!*/
/*
void stepperHALF(int xw) {
for (int x = 0; x < xw; x++) {
switch (Steps) {
/*case 0:

case xxx:
digitalWrite(IN1, xx);
digitalWrite(IN2, xx);
digitalWrite(IN3, xx);
digitalWrite(IN4, xxx);
break;
default:
digitalWrite(IN1, LOW);
digitalWrite(IN2, LOW);
digitalWrite(IN3, LOW);
digitalWrite(IN4, LOW);
break;
}
SetDirection();
}
}
void SetDirection() {
if (Direction == 1) {
Steps++;
}
if (Direction == 0) {
Steps--;
}
if (Steps > 7) {
Steps = 0;
}
if (Steps < 0) {
Steps = 7;
}
}
*/
