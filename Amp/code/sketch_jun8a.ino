#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "SWIM";
const char* password = "tkadndlajtus";
const char* mqtt_server = "192.168.2.65";
//"broker.mqtt-dashboard.com" 테스트용
//"192.168.0.65" 테스트용
//"192.168.35.101" 현장용
float readA0;
float filteredA0;
float sensitivity = 0.1; //LPF를 위한 정확도 0에 가까울수록 느리지만 정확

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char ampdata[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

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
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "FireSimulator/AMP";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  readA0 = analogRead(A0);
  filteredA0 = readA0;
  pinMode(A0,INPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  digitalWrite(BUILTIN_LED, LOW);
  delay(500);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(500);

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    readA0 = analogRead(A0);
    filteredA0 = filteredA0*(1-sensitivity)+readA0*sensitivity;
    float mappedA0 = ((((filteredA0-500.5)*1/0.036)/1024)*100);
    snprintf (ampdata, MSG_BUFFER_SIZE, "%.2f", mappedA0);
    Serial.print("FireAlarm DATA : ");
    Serial.print(filteredA0);
    Serial.println("mA");
    client.publish("FireSimulator/AMP", ampdata);
  }
}