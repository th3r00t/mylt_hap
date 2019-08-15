// D.I.N.G.U.S
// Decentralized Internet NaviGable Utility System 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <PubSubClient.h>
//#include <ESP_EEPROM.h>

const char* ssid = "MyLocalTech";
const char* password = "a!ec4597778";
const char* mqtt_server = "10.0.0.2";
const int timeZone = 1;
const int timeOffset = -(4*60*60);
int relayPin = 5;
int lightState = 0;


const char* hardwareId = "DINGUS_LAB_LIGHT_2";
const char* hardwareName = "benchlight";

WiFiUDP ntpUDP;
WiFiClient wifi_client;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset, 60000);
PubSubClient mqttClient(wifi_client);

long lastMsg = 0;
const char* announceTopic = "lighting";
const char* topic = "lighting/benchlight";

void mqttConnect() {
  while (!mqttClient.connected()) {
    digitalWrite(relayPin, HIGH);
    if (mqttClient.connect(hardwareId, "home", "A@2cb13")) {
      Serial.println("Connected");
      mqttClient.publish(announceTopic, hardwareName);
      mqttClient.subscribe(topic);
      digitalWrite(relayPin, LOW);
    }
    else {
      Serial.print("mqtt connect failed, rc=");
      Serial.println(mqttClient.state());
    }
  }
}

void toggleLight(){
  Serial.println("toggleLight");
  if (lightState == 0){
    lightState = 1;
    digitalWrite(relayPin, LOW);
  }
  else {
    lightState = 0;
    digitalWrite(relayPin, HIGH);
  }
}

void lightStatus(){
  if (lightState == 0){
    mqttClient.publish(topic, "0");
  }
  else {
    mqttClient.publish(topic, "1");
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    Serial.println();
    if (i == 0 && payload[i] == '0'){
      toggleLight();
    }
    else if (i == 0 && payload[i] == '1'){
      mqttClient.publish(topic, "1");
      mqttClient.subscribe(topic);
    }
    else if (i == 0 && payload[i] == '2'){
      lightStatus();
    }
  }
  
  mqttClient.subscribe(topic);
}
// Begin Setup
void setup() {
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  delay(10);
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  // Connect WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(hardwareId);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  timeClient.update();
}
 
void loop() {
  Serial.println("");
  Serial.println(WiFi.hostname());
  WiFi.printDiag(Serial);
  while (WiFi.status() == WL_CONNECTED){
    if (!mqttClient.connected()) {
      mqttConnect();
    }
    mqttClient.loop();
  }
  setup();
}