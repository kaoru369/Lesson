const int touchPin = 3; //3:ConnectorA 4:ConnectorB
const int spkrPin = 4; //3:ConnectorA 4:ConnectorB 10:Builtin

#define DO 261.6
#define _DO 277.18
#define RE 293.665
#define _RE 311.127
#define MI 329.63
#define FA 349.228
#define _FA 369.994
#define SO 391.995
#define _SO 415.305
#define RA 440
#define _RA 466.164
#define TI 493.883
#define octDO 523.251

 
void setup() {
  Serial.begin(115200);
  pinMode(touchPin, INPUT);
  pinMode(spkrPin, OUTPUT);
  ledcAttach(spkrPin, 12000, 8); 
}

void loop() { 
  if (digitalRead(touchPin) == HIGH) {
    Serial.println("Touch!");
    delay(50);
    playMusic(); // タッチされたら音楽を再生

  } else {
    Serial.println("...");
    delay(50);
  }
  delay(100);
}

void playMusic() {
  ledcWriteTone(spkrPin, DO);
  delay(250);
  ledcWriteTone(spkrPin, RE);
  delay(250);
  ledcWriteTone(spkrPin, MI);
  delay(350);
  ledcWriteTone(spkrPin, RE);
  delay(250);
  ledcWriteTone(spkrPin, DO);
  delay(250);
  ledcWriteTone(spkrPin, 0); // no sound
  delay(250);

  ledcWriteTone(spkrPin, DO);
  delay(250);
  ledcWriteTone(spkrPin, RE);
  delay(250);
  ledcWriteTone(spkrPin, MI);
  delay(250);
  ledcWriteTone(spkrPin, RE);
  delay(250);
  ledcWriteTone(spkrPin, DO);
  delay(250);
  ledcWriteTone(spkrPin, RE);
  delay(500);
  ledcWriteTone(spkrPin, 0); // no sound
  delay(250);
  
  delay(30000);
}