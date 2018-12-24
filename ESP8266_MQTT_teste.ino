#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

#define TOPICO_SUBSCRIBE  "MQTTReleEnvia"     //tópico MQTT de escuta
#define TOPICO_PUBLISH    "MQTTReleRecebe"    //tópico MQTT de envio
#define ID_MQTT           "HomeAut"

const char* BROKER_MQTT = "192.168.15.16"; 
int BROKER_PORT = 1883;

const char* ssid = "SSID";
const char* password = "PASSWORD";

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient MQTT(espClient);

const int rele = 2;
int statusRele = false;

void handleRoot() {
  
  String html = "<HTML>";
  html += "<HEAD><TITLE>ESP8266 - Automação</TITLE></HEAD>";
  html += "<BODY>";
  if (!statusRele) {
    html += "<form method=POST action='/on'><input type=submit value=Ligar /></form>";
  } else {
    html += "<form method=POST action='/off'><input type=submit value=Desligar /></form>";
  }
  html += "</BODY>";
  html += "</HTML>";
  
  server.send(200, "text/html", html);
}

//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma ação dependendo da string recebida:
    if (msg.equals("L"))
    {
        digitalWrite(rele, HIGH);
        statusRele = true;
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("D"))
    {
        digitalWrite(rele, LOW);
        statusRele = false;
    }
     
}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     //reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void setup() {
  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");

  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);

  Serial.println("MQTT started");
}

void loop() {

  //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
 
    //envia o status de todos os outputs para o Broker no protocolo esperado
    //EnviaEstadoOutputMQTT();
 
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
    
  server.handleClient();
}
