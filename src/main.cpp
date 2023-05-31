#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 15
#define ONE_MINUTE 60000
DHT dht(DHTPIN, DHTTYPE); 


const char *ssad = "OiFIBRA_200";
const char *pass = "oifib200";
const char *ssad2 = "gabriel";
const char *pass2 = "secret123";
int temperatura;
int atrasoTemp = 2500;  //tempo do delay para chamar novamente a função de medir temperatura
unsigned long tempoAnteriorTemp = 0;

unsigned long lastMsg = 0;        //unsigned long = inteiro de 32 bits sem sinal
#define MSG_BUFFER_SIZE  (50)     
char msg[MSG_BUFFER_SIZE];  

WiFiClient espClient;

const char *mqtt_broker = "broker.hivemq.com";
const char *temp = "esp32/gabriel/temp";
const char *umdt = "esp32/gabriel/umt";
const int mqtt_port = 1883;

PubSubClient client(espClient);


void controlleRele(bool isTurnOn){
    if(isTurnOn){
        digitalWrite(12,HIGH);
    }else{
        digitalWrite(12,LOW);
    }
}

void callback(char* topic, byte* payload, unsigned int lenght) {
    for(int i =0; i < lenght; i++){
        if((char)payload[0] == '1'){
            controlleRele(true);
        }
        if((char)payload[0] == '0'){
            controlleRele(false);
        }
    }
   
}


void connectBroken(){
    client.setServer(mqtt_broker,mqtt_port);
    client.setCallback(callback);
    if(!client.connected()){
        Serial.println("tentando conectar ao mqtt...");
        String clientId = "";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println(clientId);
            Serial.println("Conectado o broken");              
            client.subscribe(temp);
        }
    }
}


void reconectBroken() {
  if (!client.connected()) {
    connectBroken();
  }
  client.loop();
}

void publicAnyThing(){
    client.publish(temp,"teste pelo esp32");
    delay(1000);
}

void wifiConnect(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssad2,pass2);
    while (WiFi.status() != WL_CONNECTED){
        delay(1000);
        Serial.println("tentando conectar no wifi...");
        digitalWrite(23,HIGH);
        delay(500);
        digitalWrite(23,LOW);
    }
    Serial.println("Conectado ao Wifi");
    digitalWrite(22,HIGH);
}

void setSensor(){
    dht.begin();
    if (millis() > (tempoAnteriorTemp + atrasoTemp)) {
    tempoAnteriorTemp = millis();
    temperatura = dht.readTemperature(); //lê temperatura no dht22
    int umidade = dht.readHumidity();
    Serial.print(temperatura);
    Serial.println(F("°C"));
    if(temperatura > 29){
        controlleRele(true);
        digitalWrite(26,HIGH);

    }else{
        controlleRele(false);
        digitalWrite(26,LOW);
    }
    
    //delay(ONE_MINUTE);
    sprintf(msg, "{\"temp\":%i, \"umid\":%i}", temperatura,umidade);
    client.publish(temp,msg);
    
    //delay(ONE_MINUTE);
  }
}



void setPins(){
    pinMode(13,OUTPUT);
    pinMode(23,OUTPUT);
    pinMode(22,OUTPUT);
    pinMode(12,OUTPUT);
    pinMode(26,OUTPUT);
}

void setup(){
    Serial.begin(115200);
    setPins();
    wifiConnect();
    connectBroken();
}

void loop(){
    reconectBroken();
    setSensor();
}