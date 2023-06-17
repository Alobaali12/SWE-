int button = 2;
void setup() {
  // put your setup code here, to run once:
  pinMode(button, INPUT);
  digitalWrite(button, HIGH); //activate arduino internal pull up
  Serial.begin(9600);
}
void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(button)==LOW){
    Serial.println("Button is pressed");
  }
  delay(500);
}  
