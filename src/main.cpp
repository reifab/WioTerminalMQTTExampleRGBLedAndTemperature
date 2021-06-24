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
const char *ssid = "IoTB";      // your network SSID
const char *password = "Zukunftswerkstatt*5"; // your network password
 
const char *ID = "Wio-Terminal-1-Haus1";  // Name of our device, must be unique
const char *pubTopic = "H291/HausX/1OG/BME680SensorDaten";  // Topic to publish to
const char *subTopic = "H291/HausX/1OG/RelaisHeizung";  // Topic to subcribe to
const char *server = "172.20.1.31"; // Server URL

//============================== Relays ==========================
#define RELAIS_CTRL_PIN D0

//============================== Sensor BME280 ==========================
#define IIC_ADDR uint8_t(0x76)
Seeed_BME680 bme680(IIC_ADDR); /* IIC PROTOCOL */
/*Timeinterval to publish sensor data*/
TimeInvervall timerToPublish(1000);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

//============================== Incoming Data ===========================
void callback(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<50> doc;
  Serial.println("-------new message from broker-----");
  Serial.print("topic:");
  Serial.println(topic);
  Serial.print("data:");
  Serial.write(payload, length);
  Serial.println();
  deserializeJson(doc, payload);
  int stateRelays = doc["stateRelays"];
  digitalWrite(RELAIS_CTRL_PIN, stateRelays);
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
  pinMode(RELAIS_CTRL_PIN, OUTPUT);
  digitalWrite(RELAIS_CTRL_PIN, 0);
  Serial.begin(115200);
  //while (!Serial); // Wait for Serial to be ready
  if (!bme680.init()) {
        Serial.println("bme680 init failed ! can't find device!");
  }
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

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
    StaticJsonDocument<128> doc;
    char sensorDataAsCharArray[128];

    doc["temperature"] = bme680.read_temperature();
    doc["pressure"] =  bme680.read_pressure();
    doc["humidity"] =  bme680.read_humidity();
    doc["gas"] =  bme680.read_gas();
  
    serializeJson(doc,sensorDataAsCharArray);
    Serial.println(sensorDataAsCharArray);
    client.publish(pubTopic,sensorDataAsCharArray);
  }
}