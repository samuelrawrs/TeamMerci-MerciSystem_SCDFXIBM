#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson/releases/tag/v5.0.7
#include "DHT.h"
#define BLUELED 4 //D2
#define YELLOWLED 12 //D1
#define DHTPIN 2 //D4
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
//-------- Customise these values -----------
const char* ssid = "Ash-1F9C";
const char* password = "Ashtrace07";

#define ORG "p2cher"
#define DEVICE_TYPE "NODEMCU"
#define DEVICE_ID "Esp8266_nodemcu"
#define TOKEN "J@)uWnbcl+a&1nO!f5"
//-------- Customise the above values --------

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

const char eventTopic1[] = "iot-2/evt/temperature/fmt/json";
const char eventTopic2[] = "iot-2/evt/humidity/fmt/json";
const char cmdTopic[] = "iot-2/cmd/led/fmt/json";



WiFiClient wifiClient;
void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if (payload[0] == '1') {
    digitalWrite(BLUELED, HIGH);  //ASCII VALUE OF '0' IS 48 
    digitalWrite(YELLOWLED, LOW);
    Serial.println( "BLUE ON" );   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(YELLOWLED, HIGH);   //ASCII VALUE OF '1' IS 49 
    digitalWrite(BLUELED, LOW);
    Serial.println( "YELLOW ON" );  }

}
PubSubClient client(server, 1883, callback, wifiClient);

int publishInterval = 1000; // 5 seconds//Send adc every 5sc
long lastPublishMillis;

void setup() {
  Serial.begin(9600); Serial.println();
  pinMode(BLUELED ,OUTPUT); 
  pinMode(YELLOWLED ,OUTPUT);
  wifiConnect();
  mqttConnect();
}

void loop() {
  if (millis() - lastPublishMillis > publishInterval) {
    publishData();
    lastPublishMillis = millis();
  }

  if (!client.loop()) {
    mqttConnect();
  }

  warning();
}

void warning() {
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  if(temp>40 && humid<50) {
    digitalWrite(YELLOWLED, HIGH);
    digitalWrite(BLUELED, LOW);
    delay(500);
    digitalWrite(YELLOWLED, LOW);
    digitalWrite(BLUELED, LOW);
    Serial.println("WARNING");
  }
  else {
    digitalWrite(BLUELED, HIGH);
    digitalWrite(YELLOWLED, LOW);

  }
}

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

}

void mqttConnect() {
  if (!!!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!!!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    if (client.subscribe(cmdTopic)) {
      Serial.println("subscribe to responses OK");
    } else {
      Serial.println("subscribe to responses FAILED");
    }
    Serial.println();
  }
}


void publishData() {
  // read the input on analog pin 0:
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();// Reading humidity 

  String payload1 = "{\"Sensor_data\":{\"Temperature\":";
  payload1 += String(temp, DEC);
  payload1 += "}}";

  String payload2 = "{\"Sensor_data\":{\"Humidity\":";
  payload2 += String(humid, DEC);
  payload2 += "}}";


  Serial.print("Sending payload: "); Serial.println(payload1);

  if (client.publish(eventTopic1, (char*) payload1.c_str())) {
    Serial.println("Publish1 OK");
  } else {
    Serial.println("Publish1 FAILED");
  }
  
  if (client.publish(eventTopic2, (char*) payload2.c_str())) {
    Serial.println("Publish2 OK");
  } else {
    Serial.println("Publish2 FAILED");
  }
}
