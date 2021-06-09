#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Define the pin so its easy to understand
#define pirPIN 16
#define ledPin 5
#define relayPin 4
#define potentioPin 0



int potentioValue; 
  
int A0Pin = A0;

int lightIntensityThres =70;
int reading , bright ;

// Update these with values suitable for your network.
// Digunakan untuk inisiasi network
const char* ssid =         "azmi";
const char* password =    "passwordbaru000";
const char* mqtt_server =  "test.mosquitto.org";
//"192.168.0.113";   /// Menggunakan alamat IP Laptop ini , dikarenakan telah terinstall mosquito

// Inisiasi Objects Wifi dan PubSub 
WiFiClient espClient;
PubSubClient client(espClient);

// Inisiasi Value dan msg untuk di pass pada method snprintf 
char msg[50];
int value = 1;

void setup() {

  // Inisiasi Seluruh pin yang digunakan dan koneksi 
  pinMode(ledPin, OUTPUT);     
  pinMode(pirPIN, INPUT);
  pinMode(potentioPin,INPUT);
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  // prosedure ini digunakan untuk melakukan koneksi ESP8266 kepada WiFi
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

void photoresistor() {
  // Method ini digunakan untuk membaca sensor photoresistor dan melakukan publish kepada broker
  reading=analogRead(potentioPin);

  if(reading > lightIntensityThres) {
      map(reading,0,1023,0,255); // MApping value
      String readd= String(reading); // Konversi int ke String
      const char* convertedRead = readd.c_str(); // Mengubah ketipe char*
      snprintf (msg, 75, convertedRead,value); // save char* kepada buffer
      //Serial.println("Publish message for PhotoResistor: ");
      client.publish("event/detectphotoresistor", msg); // Publish kepada topik event/detectphotoresistor
 
  }

}
  
  
void callback(char* topic, byte* payload, unsigned int length) {
  // Method
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if (strcmp((char*)payload, "No one Here") == 0 ){
     Serial.println("LOW");
     //digitalWrite(ledPin, LOW);  
     Serial.println("No One here !!");
     client.publish("event/detectRelayOutput","No one Here!! ");
     
  } 

 if (strcmp((char*)payload, "Someone Here") == 0) {
    Serial.println("HIGH");

   // digitalWrite(ledPin,HIGH);
    Serial.println("Someone here !!");
    client.publish("event/detectRelayOutput","Someone Here !!");
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
      client.subscribe("event/detectRelayOutput");
                  
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

    if(digitalRead(pirPIN) == HIGH) {
      photoresistor();
      delay(100);
      Serial.println("Motion Detected");
      potentioValue = analogRead(potentioPin);    
      digitalWrite(pirPIN, HIGH);
      digitalWrite(relayPin,LOW);
      potentioValue = map(potentioValue, 0, 1023, 0, 255);
      
      if(potentioValue > 100) {
        
        String readdPotentio= String(potentioValue);
        const char* convertedReadPotentio = readdPotentio.c_str();
        snprintf (msg, 75, convertedReadPotentio,value);
        //Serial.println("Publish message for Potentio: ");
        client.publish("event/potentioMeter", msg);

        delay(100);
        
        digitalWrite(ledPin,1);
        
        
      }
      else {
        String readdPotentio= String(potentioValue);
        const char* convertedReadPotentio = readdPotentio.c_str();
        snprintf (msg, 75, convertedReadPotentio,value);
        //Serial.println("Publish message for Potentio: ");
        client.publish("event/potentioMeter", msg);
        delay(100);
        analogWrite(ledPin,potentioValue);
        
        }
      
      snprintf (msg, 75, "1", value);
      //Serial.println("Publish message for PIR: ");
      client.publish("event/detectPIR", msg);
      snprintf (msg, 75,"Someone Here",value);
      client.publish("event/detectRelayOutput",msg);
      delay(50);    
   
    }
    
  if(digitalRead(pirPIN) == LOW) {
        
        Serial.println("Motion Ended");
        delay(50);
        digitalWrite(pirPIN, LOW);
        digitalWrite(ledPin, LOW);
        digitalWrite(relayPin,HIGH);
        snprintf (msg, 75, "0", value);
        //Serial.println("Publish message for PIR: ");
        client.publish("event/detectPIR", msg);
        
        snprintf (msg, 75,"No one Here",value);
        client.publish("event/detectRelayOutput",msg);
        }

  delay(5000);
}
