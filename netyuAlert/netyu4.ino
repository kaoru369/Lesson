#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

const char* ssid = "AP01-02";
const char* password = "1qaz2wsx";
const char* n8nURL = "http://192.168.100.28:5678/webhook/c8a82d19-f62d-4f0c-9da3-34d3f61b22a2";

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int ledPin = 4;  // LEDピン

// State variables
float currentTemp = 0;
float currentHumidity = 0;
String currentLevel = "safe";
bool alertActive = false;
unsigned long lastCheck = 0;
unsigned long alertStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(ledPin, OUTPUT);  // LEDピンを出力として設定
  digitalWrite(ledPin, LOW);
  
  Serial.println("Heatstroke Monitor Starting");
  
  // Initialize OLED (SDA=1, SCL=3)
  Wire.begin(1, 3);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("ERROR: SSD1306 allocation failed");
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5);
  display.print(F("Heatstroke"));
  display.setCursor(0, 15);
  display.print(F("Monitor"));
  display.setCursor(0, 25);
  display.print(F("Starting..."));
  display.display();
  
  initWiFi();
}

void initWiFi() {
  Serial.println("Initializing WiFi");
  
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    attempts++;
    
    Serial.print("Connecting... (");
    Serial.print(attempts);
    Serial.println("/30)");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 5);
    display.print(F("WiFi Connecting..."));
    display.setCursor(0, 25);
    display.print(F("Attempt: "));
    display.print(attempts);
    display.print(F("/30"));
    display.display();
    
    if (attempts == 15) {
      Serial.println("Retrying connection...");
      WiFi.disconnect();
      delay(2000);
      WiFi.begin(ssid, password);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    
    displayStartupComplete();
  } else {
    Serial.println("WiFi Connection Failed");
    displayWiFiError();
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect");
    reconnectWiFi();
  }
  
  // Check weather data every minute
  if (WiFi.status() == WL_CONNECTED && 
      (currentTime - lastCheck >= 60000 || lastCheck == 0)) {
    checkHeatstroke();
    lastCheck = currentTime;
  }
  
  // Handle alert state
  if (alertActive) {
    if (currentTime - alertStartTime < 30000) { // 30 seconds alert
      displayAlert();
      if ((currentTime - alertStartTime) % 2000 < 1000) { // LED点滅：2秒間隔で1秒点灯
        digitalWrite(ledPin, HIGH);
      } else {
        digitalWrite(ledPin, LOW);
      }
    } else {
      alertActive = false;
      digitalWrite(ledPin, LOW);  // アラート終了時にLEDを消灯
      Serial.println("Alert deactivated");
    }
  } else {
    // Normal display
    if (WiFi.status() == WL_CONNECTED) {
      displayNormal();
    } else {
      displayWiFiError();
    }
  }
  
  delay(100);
}

void reconnectWiFi() {
  Serial.println("Reconnecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(1000);
    attempts++;
    
    Serial.print("Reconnection attempt: ");
    Serial.print(attempts);
    Serial.println("/10");
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 5);
    display.print(F("WiFi Reconnecting"));
    display.setCursor(0, 25);
    display.print(F("Attempt: "));
    display.print(attempts);
    display.print(F("/10"));
    display.display();
    
    if (attempts == 5) {
      WiFi.disconnect();
      delay(2000);
      WiFi.begin(ssid, password);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi reconnected successfully");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi reconnection failed");
  }
}

void checkHeatstroke() {
  Serial.println("Checking heatstroke data");
  
  HTTPClient http;
  http.begin(n8nURL);
  http.setTimeout(15000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("HTTP Response received");
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      if (doc.containsKey("level")) {
        currentLevel = doc["level"].as<String>();
      }
      if (doc.containsKey("temperature")) {
        float tempF = doc["temperature"];
        currentTemp = (tempF - 273.15); // Convert Fahrenheit to Celsius
      }
      if (doc.containsKey("humidity")) {
        currentHumidity = doc["humidity"];
      }
      
      Serial.print("Temperature: ");
      Serial.print(currentTemp);
      Serial.println("°C");
      Serial.print("Humidity: ");
      Serial.print(currentHumidity);
      Serial.println("%");
      Serial.print("Level: ");
      Serial.println(currentLevel);
      
      if (currentLevel == "warning" || currentLevel == "severe") {
        Serial.println("*** Warning level reached! ***");
        triggerAlert();
      }
    } else {
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

void triggerAlert() {
  alertActive = true;
  alertStartTime = millis();
  Serial.println("Alert triggered");
  Serial.print("Warning level: ");
  Serial.println(currentLevel);
}

void displayAlert() {
  display.clearDisplay();
  
  // Flashing warning message
  if ((millis() / 500) % 2 == 0) {
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 0);
    display.print(F("WARNING!"));
  }
  
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(F("Heatstroke Risk"));
  
  display.setCursor(0, 35);
  display.print(F("Temp: "));
  display.print(currentTemp, 1);
  display.print(F("C"));
  
  display.setCursor(0, 45);
  display.print(F("Humd: "));
  display.print(currentHumidity, 0);
  display.print(F("%"));
  
  display.setCursor(0, 55);
  display.print(F("Level: "));
  display.print(currentLevel);
  
  display.display();
}

void displayNormal() {
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print(F("Heatstroke Monitor"));
  
  display.setCursor(0, 12);
  display.print(F("Temp: "));
  display.print(currentTemp, 1);
  display.print(F("C"));
  
  display.setCursor(0, 22);
  display.print(F("Humd: "));
  display.print(currentHumidity, 0);
  display.print(F("%"));
  
  display.setCursor(0, 32);
  display.print(F("Stat: "));
  display.print(currentLevel);
  
  display.setCursor(0, 42);
  display.print(F("WiFi: "));
  display.print(WiFi.RSSI());
  display.print(F("dBm"));
  
  display.setCursor(0, 52);
  display.print(F("IP: "));
  display.print(WiFi.localIP());
  
  display.display();
}

void displayWiFiError() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 5);
  display.print(F("WiFi ERROR"));
  
  display.setCursor(0, 15);
  display.print(F("Stat: "));
  display.print(WiFi.status());
  
  display.setCursor(0, 25);
  display.print(F("Connection Lost"));
  
  display.setCursor(0, 35);
  display.print(F("SSID: "));
  display.print(ssid);
  
  display.setCursor(0, 45);
  display.print(F("Check Network"));
  
  display.setCursor(0, 55);
  display.print(F("Reconnecting..."));
  
  display.display();
}

void displayStartupComplete() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0);
  display.print(F("WiFi Connected!"));
  
  display.setCursor(0, 12);
  display.print(F("IP: "));
  display.print(WiFi.localIP());
  
  display.setCursor(0, 22);
  display.print(F("Signal: "));
  display.print(WiFi.RSSI());
  display.print(F("dBm"));
  
  display.setCursor(0, 32);
  display.print(F("System Ready"));
  
  display.setCursor(0, 42);
  display.print(F("Monitoring..."));
  
  display.display();
  delay(5000);
}