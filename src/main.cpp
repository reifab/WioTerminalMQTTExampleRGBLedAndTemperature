#include <Arduino.h>
#include <rpcWiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <TimePeriod.h>
#include <Wire.h>

//============================== WIFISETUP ==============================
// Update these with values suitable for your network.
const char *ssid = "IoTB";      // your network SSID
const char *password = "tbd"; // your network password
 
const char *ID = "Wio-Terminal-Client";  // Name of our device, must be unique
const char *pubTopic = "H291/Haus1";   // Topic to publish to
const char *pubTopicLichtstaerke = "H291/Haus1/1OG/Lichtstaerke";   // Topic to publish to
const char *pubTopicWindow = "H291/Haus1/2OG/Fenster";   // Topic to publish to
const char *subTopic = "H291/Haus1/1OG/RGBLeds";        // Topic to subcribe to
const char *server = "172.20.1.31"; // Server URL

//============================== RGB LEDs ===============================
#define NUM_LEDS 10
#define LEDS_DATA_PIN D0
Adafruit_NeoPixel ledStrip(NUM_LEDS, LEDS_DATA_PIN, NEO_GRB + NEO_KHZ800);

/*Timeinterval to publish sensor data*/
TimeInvervall timerToPublish(1000);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

//============================== Incoming Data ===========================
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  bool switchAllLEDsOnUntilPixelIndex;
  uint32_t pixelIndex;
  uint32_t color;

  Serial.println("-------new message from broker-----");
  Serial.print("topic:");
  Serial.println(topic);
  Serial.print("data:");
  Serial.write(payload, length);
  Serial.println();
  deserializeJson(doc, payload);

  if(doc["r"].isNull()){
    red = 0;
  }else{
    red = doc["r"];
  }

  if(doc["g"].isNull()){
    green = 0;
  }else{
    green = doc["g"];
  }

  if(doc["b"].isNull()){
    blue = 0;
  }else{
    blue = doc["b"];
  }

  if(doc["pixelIndex"].isNull()){
    pixelIndex = 0;
  }else{
    pixelIndex = doc["pixelIndex"];
  }

  if(doc["switchAllLEDsOnUntilPixelIndex"].isNull()){
    switchAllLEDsOnUntilPixelIndex = false;
  }else{
    switchAllLEDsOnUntilPixelIndex = doc["switchAllLEDsOnUntilPixelIndex"];
  }

  color = ledStrip.Color(red, green, blue);

  if(pixelIndex >= NUM_LEDS){
    pixelIndex = 9;
  }
  if(doc["pixelIndex"].isNull()){
    ledStrip.fill(color);
  }else{
    if(switchAllLEDsOnUntilPixelIndex){
      ledStrip.clear();
      for(uint8_t i=0; i<=pixelIndex; i++){
        ledStrip.setPixelColor(i, color);
      }
    }else{
      ledStrip.setPixelColor(pixelIndex, color);
    }
  }
  ledStrip.show();
}

//============================== Reconnect if necessary ==================
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(pubTopic, "{\"message\": \"Wio Terminal is connected!\"}");
      Serial.println("Published connection message successfully!");
      // ... and resubscribe
      client.subscribe(subTopic);
      Serial.print("Subcribed to: ");
      Serial.println(subTopic);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//============================== Setup ==================
void setup()
{
  pinMode(WIO_LIGHT, INPUT);
  pinMode(D2, INPUT);
  Serial.begin(115200);
  //while (!Serial); // Wait for Serial to be ready
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  ledStrip.begin();
  ledStrip.show();

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    WiFi.begin(ssid, password);
    // wait 1 second for re-trying
    delay(1000);
  }
 
  Serial.print("Connected to ");
  Serial.println(ssid);
  delay(500);
 
  client.setServer(server, 1883);
  client.setCallback(callback);
}

//============================== Loop ==================
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(timerToPublish.isTimeElapsed()){
    uint16_t lightIntensity =  analogRead(WIO_LIGHT);
    Serial.println("Lichtintensität " + lightIntensity);
    String lightIntensityAsString = String(lightIntensity);
    client.publish(pubTopicLichtstaerke,lightIntensityAsString.c_str());
    if (digitalRead(D2) == HIGH) {
      client.publish(pubTopicWindow,"closed");
      Serial.println("Magnetschalter geschlossen");
    }else{
      client.publish(pubTopicWindow,"open");
      Serial.println("Magnetschalter offen");
     
    }
  }
}