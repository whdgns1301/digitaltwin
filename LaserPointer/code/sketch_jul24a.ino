#include <ESPping.h>
#include <ping.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LASER1 D6
#define LED D1


const char* ssid = "SWIM";
//const char* ssid = "Jonghoon";
const char* password = "tkadndlajtus";
//const char* mqtt_server = "192.168.2.65"
const char* mqtt_server = "192.168.35.101";
//const char* mqtt_server = "swimjagalchi.iptime.org";
//const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

void (*addressZero)()=0; //초기화 함수
int timecount = 0;
int wificount =0;
int pingcount =0;
unsigned long past = 0;
int flag = 0;

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
    wificount++;
    if(wificount >=10){
      addressZero();
    }
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String myPayload = "";
  for (int i = 0; i < length; i++) {
  myPayload += (char)payload[i];
  Serial.println(myPayload);
  }
  
  Serial.println();

  if(strcmp(topic, "FireSimulator/TurnSignal") == 0){
    if(myPayload.substring(30,31) == "1"){
      digitalWrite(LASER1,LOW);
      digitalWrite(LED, HIGH);
    }else if (myPayload.substring(30,31) == "0"){
      digitalWrite(LASER1,HIGH);
      digitalWrite(LED, LOW);
    }
  }
}
  

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "FireSimulator/TurnSignal";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("FireSimulator/TurnSignal");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("attempt to reset");
      digitalWrite(LED,LOW);
      addressZero();
    }
  }
}

void setup() {
  pinMode(LASER1,OUTPUT);
  pinMode(LED,OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setServer(mqtt_server, 22883);
  client.setCallback(callback);

}

void loop() {
  unsigned long now = millis();

  if (now - past >= 1000) {
    past = now;
    flag = 1;
  }
  if (flag == 1) {
  IPAddress ip (192, 168, 35, 101);
  bool ping = Ping.ping(ip);
  if(ping == 0){
    pingcount++;
  }
  else if(ping == 1){
    pingcount--;
  }
  //Serial.print("pingcount = ");
  //Serial.println(pingcount);
  if(pingcount<0){
    pingcount++;
  }
  if(pingcount>4){
    addressZero();
  }
  flag = 0;
  }

  if (!client.connected()) {
    Serial.println("연결시도 : MQTT");
    reconnect();
  }
  client.loop();

}