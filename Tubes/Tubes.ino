#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define pirPIN 16
#define ledPin 5

int calibrationTime = 30;
long unsigned int lowIn;
long unsigned int pause = 500;
boolean lockLow = true;
boolean takeLowTime;
int PIRValue = 0;

// Update these with values suitable for your network.

const char* ssid =         "azmi";
const char* password =    "passwordbaru000";
const char* mqtt_server = "192.168.0.113";   /// example 192.168.0.19

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(ledPin, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(pirPIN, INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  Serial.println("Sampis");
  Serial.println((char)payload[0]);
  if ((char)payload[0] == '0') {
     Serial.println("LOW");
    digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } 

 if ((char)payload[0] == '1') {
    Serial.println("HIGH");
    digitalWrite(pirPIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String clientIds = "ESP8266-";
    clientIds += String(random(0xffff),HEX);
    Serial.println(clientIds);
    if (client.connect(clientIds.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("event/detectPIR", "hello world,Connected");
      // ... and resubscribe
      client.subscribe("event/detectPIR");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      millis();
      delay(10000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  long lastMsg = 0;

    if(digitalRead(pirPIN) == HIGH) {

    if(lockLow) {
      PIRValue = 1;
      lockLow= false;
      Serial.println("Motion Deteced");
      digitalWrite(ledPin,1);
      ++value;
      snprintf (msg, 75, "1", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("event/detectPIR", msg);
      delay(50);    
      
      }
      takeLowTime = true;
    }
    
  if(digitalRead(pirPIN) == LOW) {
    if(takeLowTime) {
      lowIn = millis();
      takeLowTime = false;
      }
      if(!lockLow && millis() - lowIn > pause) {
        PIRValue = 0;
        lockLow =true;
        digitalWrite(ledPin,0);
        Serial.println("Motion Ended");
        delay(50);
     
        ++value;
        snprintf (msg, 75, "0", value);
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("event/detectPIR", msg);
        }
    
  }
}
