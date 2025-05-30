const int swPin = 3; //3:ConnectorA 4:ConnectorB
const int motorPin = 4; //3:ConnectorA 4:ConnectorB 10:Builtin

volatile int state = LOW;
 
void setup() {
  pinMode(swPin, INPUT);
  pinMode(motorPin, OUTPUT);
  attachInterrupt(swPin, motor_blink, FALLING);
}

void loop() {
  digitalWrite(motorPin, state);
}

void motor_blink(){
  delay(10);
  state = !state;
}