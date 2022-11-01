#include <WiFi.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#define DHTPIN 25
#define DHTTYPE DHT22
#define DELAY 1000 * 30
#define DEEP_SLEEP_TIME 15
#define VCC_2 26

DHT dht(DHTPIN, DHTTYPE);


// Replace with your network credentials
const char* ssid = "SAMPLE_SSID";
const char* password = "SAMPLE_PASSWORD";

WiFiClient client;

// Replace with your ThingSpeak credentials
unsigned long myChannelNumber = 2;
const char* myWriteAPIKey = "SAMPLE_API_KEY";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 171);

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}

void goToDeepSleep() {
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60L * 1000000L);

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

void setup() {
  // Wi-Fi in non-usb connections might be unstable, waiting for stabilizing.
  delay(DELAY);
  btStop();
  esp_bt_controller_disable();

  // set dht pin to input with pullup resistance
  pinMode(DHTPIN, INPUT_PULLUP);
  pinMode(VCC_2, OUTPUT);
  digitalWrite(VCC_2, HIGH);

  // initialize wifi
  initWiFi();

  // initialize dht
  dht.begin();

  // initialize thingspeak
  ThingSpeak.begin(client);
}


void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      delay(DELAY);
    }
  }

  // Read from DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Send to thingspeak
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  while (x != 200) {
    x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    delay(DELAY);
  }

  goToDeepSleep();
}