
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SEND_INTERVAL_SECS  10

const char *WIFI_NAME = "LEGO";
const char *PASSWORD = "y3nz3artx";
const char* BROKER = "broker.hivemq.com";
const char* RASPBERRY_TOPIC = "myRaspBerryName";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

void connectWiFi()
{

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, PASSWORD);
  
  Serial.print("Connecting to");
  Serial.print(WIFI_NAME);
  
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(3000);
  }
  Serial.println(" Wifi conected!");
}

void mqttCallback(char* topic, byte* message, unsigned int len){}

/// @brief connect to MQTT broker
void mqttConnect() 
{
  Serial.print("Conecting to mqtt.");
  while(!mqtt.connected()){
    // Connect to MQTT Broker without username and password
    if(mqtt.connect("project123djdjd")){
      Serial.println("Connected to mqtt successfully");
      return;
    }
    Serial.print(".");
  }
}

void mqttInit()
{
  mqtt.setServer(BROKER, 1883);
  mqtt.setCallback(mqttCallback);
  mqttConnect();
}

void initializeDataSender()
{
  connectWiFi();
  mqttInit();
}


void sendData(float ph_value, float turbidity)
{
  static unsigned int lastSent = 0;
  if((millis() - lastSent) < (SEND_INTERVAL_SECS*1000))
  {
    return;
  }


  char data[30] = "";
  sprintf(data, "{\"ph\":%.2f,\"ntu\":%.2f}", ph_value, turbidity);

  if(!mqtt.connected())
  {
    Serial.println("mqtt connection lost");
    mqttConnect();
  }

  mqtt.publish(RASPBERRY_TOPIC, data);
  Serial.print("Sent-> ");
  Serial.println(data);

  lastSent = millis();
}