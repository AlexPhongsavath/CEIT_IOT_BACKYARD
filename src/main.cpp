#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <WiFiManager.h>
#include <Wire.h>

#include "config.h"

// config pin on ESP32
#define DHTPIN 4
#define WifiLED 13
#define MqttLED 12
#define LDRPIN 2
#define SOILPIN 15
#define BUTTONPIN 23

#define DHTTYPE DHT22 // DHT 22
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
int value = 0;
void callback(char *topic, byte *message, unsigned int length);
void connectwifi();
void connectmqtt();
void resetWifi();
WiFiManager wm;
bool res;

void setup()
{
  // Start Communication
  Serial.begin(115200);
  connectwifi();
  connectmqtt();

  // Begin any sensors
  dht.begin();
  pinMode(WifiLED, OUTPUT);
  pinMode(MqttLED, OUTPUT);
  pinMode(BUTTONPIN, INPUT);
}

void resetWifi()
{
  // Button for reset wifi
  int value = digitalRead(BUTTONPIN);
  if (value == 1)
  {
    wm.resetSettings();
  }
}

// Connect wifi
void connectwifi()
{
  // ESP32 start as AP mode
  res = wm.autoConnect(ssid, password);

  // If failed to connect wifi. press reset wifi button for start AP mode again
  if (!res)
  {
    Serial.println("Failed to connect");
    resetWifi();
  }
  else
  {
    Serial.println("connected");
    Serial.println(WiFi.SSID());
    digitalWrite(WifiLED, HIGH);
  }
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

// Convert mac to String
String macToStr(const uint8_t *mac)
{
  String result;
  for (int i = 0; i < 6; ++i)
  {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

// Connect Mqtt
void connectmqtt()
{
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    String clientName;
    clientName += "esp32-s";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    clientName += "-";
    clientName += String(micros() & 0xff, 16);
    Serial.print("Connecting to ");
    Serial.print(mqtt_server);
    Serial.print(" as ");
    Serial.println(clientName);

    // Attempt to connect
    // If you want to use a username and password, change next line to
    if (client.connect((char *)clientName.c_str())) 
    //if (client.connect("esp32-s", mqttUser, mqttPass))
    {
      Serial.println("connected");
      digitalWrite(MqttLED, HIGH);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      digitalWrite(MqttLED, LOW);
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    connectmqtt();
  }

  client.loop();
  long now = millis();

  // Send data every 10seconds
  if (now - lastMsg > 10000)
  {
    lastMsg = now;
    resetWifi();

    // Read temperature and humidity from dht22
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Read moisture from soil sensor
    int s = analogRead(SOILPIN);
    int soil = map(s, 0, 4095, 100, 0);

    // Read value from LDR sensor
    int l = analogRead(LDRPIN);
    int ldr = map(l, 0, 4095, 100, 0); // for esp32
    //int ldr = map(l, 0, 1024, 100, 0); // for esp8266

    if (isnan(h) || isnan(t))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.println();
    Serial.println("---------------------------");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Moisture: ");
    Serial.print(soil);
    Serial.println(" %");
    Serial.print("Light: ");
    Serial.print(ldr);
    Serial.println("---------------------------");

    // Pepare String to send data as json
    String temp = String(t).c_str();
    String humi = String(h).c_str();
    String moisture = String(soil).c_str();
    String light = String(ldr).c_str();

    // format data to json
    String pub_data = "{\"humidity\":" + humi + ",\"moisture\":" + moisture + ", \"temperature\":" + temp + ", \"light\":" + light + "}";
    
    // Publish to Mqtt
    char msg_data[90];
    pub_data.toCharArray(msg_data, (pub_data.length() + 1));
    client.publish(PUB_Topic, msg_data);
  }
}