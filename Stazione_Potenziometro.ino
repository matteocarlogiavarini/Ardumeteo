#include <SPI.h> //Libreria eth schield
#include <Ethernet.h> //Libreria eth schield
#include <PubSubClient.h> //Libreria MQTT

byte mac[]={ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; //Indirizzo mac arduino

//Definizione parametri per connessione al broker MQTT
const char* mqttServer = "192.168.0.106";
const int mqttPort = 1883;
const char* mqttUser = "matgiav";
const char* mqttPassword = "supp";

//definizione topic per invio dati
#define topic_potenziometro "sensor/potenziometro"
#define topic_inizializzazione "stazioni/inizializzazione"

//Definizione pin led e potenziometro variabili per la lettura
int ledPin = 5;

#define potPin A0
#define tolleranzaPotenziometro 50
int nuovoValPot;
int vecchioValPot;
bool luciAutomatiche = true;

//Variabiili client MQTT
EthernetClient ArduinoUnoClient;
PubSubClient client(ArduinoUnoClient);

//Lettura potenziometro ed invio nuovo valore a Home Assistant
void letturaPotenziometro(){
  nuovoValPot = analogRead(potPin);
  int differenza = abs(nuovoValPot - vecchioValPot);
  
  if(differenza > tolleranzaPotenziometro){  
    vecchioValPot = nuovoValPot;
    client.publish(topic_potenziometro, String(nuovoValPot).c_str(), true);
  }
}

//Funzione di callback eseguita quando viene ricevuto un messaggio su uno dei topic a cui siamo iscritti
void callback(char* topic, byte* payload, unsigned int length) {
  //Stampa su seriale messagio ricevuto e topic sul quale è stato ricevuto
  Serial.print("Mesaggio arrivato dal topic: ");
  Serial.println(topic);
  String command = "";
  
  Serial.print("Messaggio: ");
  for (int i = 0; i < length; i++) {
    command += ((char)payload[i]);
  }
  
  Serial.println(command);
  Serial.println();

  //Regola l'intesità della luce se la modalità è in "automatico"
  if((strcmp(topic,"command/luce") == 0) && luciAutomatiche){
      int valoreLum = command.toInt();
      if(valoreLum > 1500){
        analogWrite(ledPin,0);
      }else if(valoreLum < 300){
        analogWrite(ledPin,255);
      }else{
        int valoreLumMap = map(valoreLum, 300, 1500, 150, 1);
        analogWrite(ledPin,valoreLumMap);
      }
  }
  //Regola l'intesità della luce se la modalità è in "manuale" (potenziometro)
  if((strcmp(topic, "command/potenziometro") == 0) && !(luciAutomatiche)){
      int valorePotenz = command.toInt();
      int valoreLumMap = map(valorePotenz, 0, 1023, 0, 255);
      analogWrite(ledPin,valoreLumMap);
  }
  //Imposta la modalità di regolazione luci
  if(strcmp(topic, "command/luciAutomatiche") == 0){
    if(command == "on") luciAutomatiche = true;
    else if (command == "off") luciAutomatiche = false;
  }
}

//Procedura per connessione al broker MQTT
void connectMQTT() {
  //Settaggio server e procedura callback
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  //Connessione
  while (!client.connected()) {
    Serial.println("Connesione al broker MQTT...");
    if (client.connect("ArduinoUnoClient", mqttUser, mqttPassword )) {
      Serial.println("Connesso!!");  
      client.publish(topic_inizializzazione, "true", true);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      Serial.println();
      delay(2000);
    }
  }

  //Iscrizione ai topic relaivi ai comandi
  client.subscribe("command/luce");
  client.subscribe("command/potenziometro");
  client.subscribe("command/luciAutomatiche");
}

void setup() {
  Serial.begin(115200); //Inizializzazione comunicazione seriale con pc
  
  //Connessione via ethernet con DHCP
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Configurazione DHCP fallita");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Cavo Ethernet non connesso");
    }
  }
  
  //Stampa IP address:
  Serial.print("IP address: ");
  Serial.println(Ethernet.localIP());

  pinMode(potPin,INPUT);
  pinMode(ledPin,OUTPUT);  
  connectMQTT();
}

void loop() {
  client.loop();
  if(luciAutomatiche == false){
    letturaPotenziometro();
  }
}
