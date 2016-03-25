//include needed libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

//temperature sensor variables
#define SENSORDATA_JSON_SIZE (JSON_OBJECT_SIZE(2))
#define DHTPIN 2 //sensor pin
const int tamanhoMsg = 50;
char msg[tamanhoMsg];
DHT dht(DHTPIN, DHT22);

//WiFi variable settings
const char* ssid = "Red Hat Guest";
const char* password = "innovate2015";

//MQTT settins
const char* mqtt_server = "10.216.121.124";
const char* mqtt_user = "admin";
const char* mqtt_password = "redhat";
const int mqtt_port = 1883;
const int updateTime= 2000;//define a taxa de atualização dos dados em milissegundos
long lastMsg = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  //set mqtt callback and server
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  connectToWiFi();
  connectToMQTT();
}

void loop() {
  client.loop();
  long now = millis();
  if (now - lastMsg > updateTime){
    lastMsg = now;
    float h,t;
    do{
      h=dht.readHumidity(); //ler os dados de humidade do sensor
      t=dht.readTemperature();//ler os dados de temperatura do sensor
    }while(isnan(h) || isnan(t));
    toJson(t,h,msg,tamanhoMsg);//converte os dados para o formato json
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensor/temp", msg);
  }
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
void toJson(float t, float h, char *json,size_t maxSize)
{
    StaticJsonBuffer<SENSORDATA_JSON_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["temperatura"] = t;
    root["umidade"] = h;
    root.printTo(json, maxSize); 
}
void connectToWiFi() 
{
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi is now connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}
void connectToMQTT(){
    while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //try to connect to MQTT with user and passoword
    if (client.connect("ESP8266Client",mqtt_user,mqtt_password)) {
      Serial.println("connected");
      //sends a message to an outTopic to say that it is now connected
      client.publish("outTopic", "o módulo está conectado!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 4 seconds");
      //try to connect to the mqtt broker every 4 second after a failed attempt 
      delay(4000);
    }
  }
}

