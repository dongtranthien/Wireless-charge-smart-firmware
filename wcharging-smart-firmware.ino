#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// WiFi
const char* ssid = "";
const char* password = "";

// MQTT Broker
const char *mqtt_broker = "";// broker address
const char *topic = "wirelesscharge/data"; // define topic 
const char *mqtt_username = ""; // username for authentication
const char *mqtt_password = "";// password for authentication
const int mqtt_port = 8883;// port of MQTT over TLS/SSL

// load DigiCert Global Root CA ca_cert
const char* ca_cert= \
"";

// init secure wifi client
WiFiClientSecure espClient;
// use wifi client to init mqtt client
PubSubClient client(espClient); 


void connectMqtt() {
  // set root ca cert
    espClient.setCACert(ca_cert);
    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    int indexConnectMqtt = 0;
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(client.state());
            Serial.println("Retrying in 5 seconds.");
            delay(5000);
        }
        
        indexConnectMqtt++;
        if(indexConnectMqtt > 20){
          ESP.restart();
        }
    }
    // publish and subscribe
    client.publish(topic, "Hi, I'm wcharging smart");
    client.subscribe(topic);
}

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    pinMode(8, OUTPUT);
    pinMode(10, OUTPUT);
    digitalWrite(8, HIGH);
    // connecting to a WiFi network
    WiFi.begin(ssid, password);
    int indexConnectWifi = 0;
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(8, !digitalRead(8));
        delay(500);
        Serial.println("Connecting to WiFi..");

        indexConnectWifi++;
        if(indexConnectWifi > 60){
          ESP.restart();
        }
    }
    Serial.println("Connected to the WiFi network");
    digitalWrite(8, LOW);
    delay(3000);

    connectMqtt();

    digitalWrite(8, HIGH);
    delay(1000);
    digitalWrite(8, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    if(payload[0] == '1'){
      digitalWrite(10, HIGH);
    }
    else if(payload[0] == '0'){
      digitalWrite(10, LOW);
    }
    
    Serial.println();
    Serial.println("-----------------------");
}

void reconnect() {
  int indexReconnectMqtt = 0;
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT broker...");
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Reconnected to MQTT broker.");
        client.subscribe(topic);
    } else {
        Serial.print("Failed to reconnect to MQTT broker, rc=");
        Serial.print(client.state());
        Serial.println("Retrying in 5 seconds.");
        delay(5000);
    }

    indexReconnectMqtt++;
    if(indexReconnectMqtt > 20){
      ESP.restart();
    }
  }
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost, attempting to reconnect...");
        WiFi.begin(ssid, password);

        int indexReconnectWifi = 0;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.println("Reconnecting to WiFi...");

            indexReconnectWifi++;
            if(indexReconnectWifi > 20){
              ESP.restart();
            }
        }
        Serial.println("WiFi reconnected.");
    }

    if (!client.connected()) {
        Serial.println("MQTT client connection lost, attempting to reconnect...");
        connectMqtt();
    }

    client.loop();
    delay(1);
}
