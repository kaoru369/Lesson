const int swPin = 3; //3:ConnectorA 4:ConnectorB
const int ledPin = 4; //3:ConnectorA 4:ConnectorB 10:Builtin

int Count = 0;

void setup() {
  pinMode(swPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  if (digitalRead(swPin) == HIGH) {
    Count++;
    Serial.println(" " + String(Count) + " 回"); 
    delay(300);
  }

  if (Count >= 5) {
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    Count = 0;
    Serial.println(" 5 回目 点灯確認　ヨシ！");
  }
  
  delay(100);
}