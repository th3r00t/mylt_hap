// D.I.N.G.U.S
// Decentralized Internet NaviGable Utility System 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <PubSubClient.h>
//#include <ESP_EEPROM.h>

const char* ssid = "MyLocalTech";
const char* password = "";
const char* mqtt_server = "10.0.0.2";
const char* msg;
const int timeZone = 1;
const int timeOffset = -(4*60*60);
int relayPin = 5;
int lightState = 0;
boolean firstRun = true;
unsigned long lastMs = 0;

const char* hardwareId = "DINGUS_LAB_LIGHT_1";
const char* hardwareName = "benchlight";

WiFiUDP ntpUDP;
WiFiClient wifi_client;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset, 60000);
PubSubClient mqttClient(wifi_client);

long lastMsg = 0;
const char* announceTopic = "lighting/";
const char* topic = "lighting/benchlight";

void mqttConnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(hardwareId, "home", "")) {
      Serial.println("Mqtt Connected");
      mqttClient.publish(announceTopic, hardwareName);
      Serial.println("Anonuncement Sent");
      mqttClient.subscribe(topic);
      Serial.println("Topic Subscribed");
      digitalWrite(relayPin, LOW);
      Serial.println("Pin Low");
    }
    else {
      digitalWrite(relayPin, HIGH);
      Serial.print("mqtt connect failed, rc=");
      Serial.println(mqttClient.state());
    }
  }
}

void toggleLight(){
  if (lightState == 0){
    lightState = 1;
    digitalWrite(relayPin, HIGH);
  }
  else {
    lightState = 0;
    digitalWrite(relayPin, LOW);
  }
  if (firstRun){
    firstRun = false;
  }
}

void lightStatus(){
  if (lightState == 0){
    msg = "Off";
  }
  else {
    msg = "On";
  }
}

void callback(char* topic, byte* payload, unsigned int length){

  for (unsigned int i = 0; i < length; i++) {
    if (i == 0 && (char)payload[i] == '0'){
      toggleLight();
      msg = "Toggled";
    }
    else if (i == 0 && (char)payload[i] == '1'){
      msg = "Waiting";
    }
    else if (i == 0 && (char)payload[i] == '2'){
      lightStatus();
    }
    else {
      break;
    }
  }
  mqttClient.subscribe(topic);
}
void mqttIntervalPost() {
  if (msg !=NULL)
  {
    mqttClient.publish(announceTopic, msg);
    msg = NULL;
  }
  else {
    delay(50);
  }
}

void mqttCheckConnect() {
    if (WiFi.status() == WL_CONNECTED && !mqttClient.connected()){
    mqttConnect();
  }
}
// Begin Setup
void setup() {
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
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
  Serial.println(WiFi.hostname());
  WiFi.printDiag(Serial);
  Serial.println("");
  Serial.println("WiFi connected");
  timeClient.update();
  mqttCheckConnect();
}

void loop() {
  if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect();
        /* Post */
        mqttIntervalPost();
    }
  mqttClient.loop();
}
