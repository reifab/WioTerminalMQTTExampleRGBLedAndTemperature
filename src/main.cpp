#include <Arduino.h>
#include <rpcWiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <seeed_bme680.h>
#include <TimePeriod.h>
#include <Wire.h>

//============================== WIFISETUP ==============================
// Update these with values suitable for your network.
const char *ssid = "tbd";      // your network SSID
const char *password = "tbd"; // your network password
 
const char *ID = "Wio-Terminal-Client";  // Name of our device, must be unique
const char *pubTopic = "outTopic";  // Topic to publish to
const char *subTopic = "inTopic";  // Topic to subcribe to
const char *server = "172.20.1.31"; // Server URL

//============================== RGB LEDs ===============================
#define NUM_LEDS 10
#define LEDS_DATA_PIN D0
Adafruit_NeoPixel ledStrip(NUM_LEDS, LEDS_DATA_PIN, NEO_GRB + NEO_KHZ800);

//============================== Sensor BME280 ==========================
#define IIC_ADDR uint8_t(0x76)
Seeed_BME680 bme680(IIC_ADDR); /* IIC PROTOCOL */
/*Timeinterval to publish sensor data*/
TimeInvervall timerToPublish(5000);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

//============================== Incoming Data ===========================
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  Serial.println("-------new message from broker-----");
  Serial.print("topic:");
  Serial.println(topic);
  Serial.print("data:");
  Serial.write(payload, length);
  Serial.println();

  deserializeJson(doc, payload);

  int red = doc["r"];
  int green = doc["g"];
  int blue = doc["b"];

  ledStrip.setPixelColor(1, red, green, blue);
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

  Serial.begin(115200);
  //while (!Serial); // Wait for Serial to be ready
  if (!bme680.init()) {
        Serial.println("bme680 init failed ! can't find device!");
  }
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
    float temperature =  bme680.read_temperature();
    Serial.println(temperature);
    String temperatureAsString = String(temperature);
    client.publish(pubTopic,temperatureAsString.c_str());
  }
}