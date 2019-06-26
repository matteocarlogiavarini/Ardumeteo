//https://randomnerdtutorials.com/esp32-with-bmp180-barometric-sensor/  LINK BMP180 
//https://www.danielealberti.it/2014/10/collegare-un-display-lcd-ad-arduino.html LINK SCHEMA DISPLAY LCD

//Importo le varie librerie
//#include <Arduino.h>
#include "DHTesp.h"
#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_BMP085.h>
#include <TaskScheduler.h>
#include <LiquidCrystal.h>
#include <PubSubClient.h>
#include <analogWrite.h>

//Definizione SSID e PASSWORD AccessPoint
const char* wifi_ssid = "FASTWEB-1-C70B5D";
const char* wifi_password =  "piscioduromattesuper1997";

//Definizione parametri broker MQTT
const char* mqttServer = "192.168.0.106";
const int mqttPort = 1883;
const char* mqttUser = "matgiav";
const char* mqttPassword = "supp";

int durataCiclo = 1; //Definisce ogni quanti minuti leggere i dati, modificabile via Home Assistant

//Definizione dei topic su cui inviare/leggere messaggi
#define topic_temperaturaDHT11 "sensor/temperaturaDHT11"
#define topic_temperaturaBMP180 "sensor/temperaturaBMP180"
#define topic_temperaturaMedia "sensor/temperaturaMedia"
#define topic_umidita "sensor/umidita"
#define topic_luminosita "sensor/luminosita"
#define topic_pressione "sensor/pressione"
#define topic_altitudine "sensor/altitudine"
#define topic_potenziometro "sensor/potenziometro"
#define topic_inizializzazione "stazioni/inizializzazione"

//Variabili client MQTT
WiFiClient espClient;
PubSubClient client(espClient);

int durataPausaMinuti = durataCiclo; //Variabile usata per mostrare sull'LCD i minuti rimantenti alla prossima lettura
bool luciAutomatiche = true; //Indica la modalità di regolazione delle luci

//Definizione sensori ed assegnazione pin
LiquidCrystal lcd(15,2,0,4,16,17); //RISPETTIVAMENTE RS = 15 ; E = 2 ; D4 = 0 ; D5 = 4 ; D6 = 16 ; D7 = 17 ; R/W = GND
Scheduler runner; 
DHTesp dht;
Adafruit_BMP085 bmp;
int potenzPin = 32;
int ledPin = 5;
int dhtPin = 19;
int lumPin = 34;
//PIN BMP180 SDA = 21 ; SCL = 22

float seaLevelPressure = 101325; // Memorizza l'attuale pressione dal livello del mare nella tua posizione in Pascal
float pressioneMbar; //Variabile per la pressione
float tempBMP180; //Variabile per la temperatura BMP180  
float altitudine; //Variabile per l'altitudine
    
TempAndHumidity valoriDHT11;//Primitiva sensore DHT
int luminosita; //Variabile per la luminosità
  
int tempDHT11; //Variabile per temperatura DHT
float temp_media; //Variabile per la temperatura media fra DHT e BMP180
int umidita; //Variabilie per umidità

//Procedura per connessione alla rete WIFI
void setup_wifi() {
  Serial.println();
  Serial.print("Connessione a ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
     Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connesso");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Procedura per connessione al broker MQTT
void connectMQTT() {
  //Settaggio server e procedura callback
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  //Connessione
  while (!client.connected()) {
    Serial.println("Connesione al broker MQTT...");

    lcd.clear();
    lcd.print("Connesione MQTT");
   
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      Serial.println("Connesso!!");
      client.publish(topic_inizializzazione, "true", true);  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      lcd.setCursor(0,1);
      lcd.print("Errore connessione");
    }
  }
  
  //Iscrizione ai topic relaivi ai comandi
  client.subscribe("command/luce");
  client.subscribe("command/potenziometro");
  client.subscribe("command/luciAutomatiche");
  client.subscribe("command/periodoLetturaSensori");
}

//Procedura per la lettura dei sensori
void letturaSensori(){
    //Scrittura LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Lettura dati...");
    
    //Lettura parametri
    pressioneMbar = bmp.readPressure();
    tempBMP180 = bmp.readTemperature();  
    altitudine = bmp.readAltitude(seaLevelPressure);
    
    valoriDHT11 = dht.getTempAndHumidity();
    luminosita = analogRead(lumPin);
  
    tempDHT11 = valoriDHT11.temperature;
    temp_media = (tempBMP180+tempDHT11)/2;
    umidita = valoriDHT11.humidity;

    durataPausaMinuti = durataCiclo;

    //Scrittura seriale
    Serial.println("Lettura temperatura: " + String(temp_media) + "C°"); 
    Serial.println("Lettura umidità: " + String(umidita) + " %");
    Serial.println("Lettura luminosità: " + String(luminosita));
    Serial.println("Lettura pressione: " + String(pressioneMbar) + " Pa");
    Serial.println("Lettura altitudine: " + String(altitudine) + " m");
    Serial.println(" ");
    
    lcd.clear();
    lcd.print("Lettura dati OK");
}

//Procedura che invia al broker MQTT i dati letti
void invioDati(){
    //Scrittura LCD
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Invio dati...");

    //Controllo di essere connesso al broker
    if (!client.connected()) {
      lcd.clear();
      lcd.print("Non connesso.");
      connectMQTT();
    }
     
    //Condivide i dati con Home Assistant
    client.publish(topic_temperaturaDHT11, String(tempDHT11).c_str(), true);
    client.publish(topic_temperaturaBMP180, String(tempBMP180).c_str(), true);
    client.publish(topic_temperaturaMedia, String(temp_media).c_str(), true);
    client.publish(topic_umidita, String(umidita).c_str(), true);
    client.publish(topic_luminosita, String(luminosita).c_str(), true);
    client.publish(topic_pressione, String(pressioneMbar).c_str(), true);
    client.publish(topic_altitudine, String(altitudine).c_str(), true);
    //c_str() converte il contenuto di una stringa come stringa in stile C, terminata da null. 
    //Si noti che questo dà accesso diretto al buffer String interno restituendo un puntatore.

    //Scrittura Seriale e LCD
    Serial.println("Dati inviati.");
    Serial.println();
 
    lcd.clear();
    lcd.print("Invio dati OK.");
}

//Procedura per la visualizzazione dei minuti mancanti alla prossima lettura su LCD
void lcdPrintTimer(){
    if(durataPausaMinuti > 0){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Nuova lettura in");
        lcd.setCursor(0,1);
        lcd.print(durataPausaMinuti);
        lcd.print(" Minuti");
        durataPausaMinuti -= 1;
    }
    else{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Nuova Lettura");
        lcd.setCursor(0,1);
        lcd.print("inizierà a breve");
    }
}

//Dichiarazione task per lettura dati, invio dati e gestione LCD
Task taskLettura(durataCiclo*TASK_MINUTE, TASK_FOREVER, letturaSensori);
Task taskInvioDati(durataCiclo*TASK_MINUTE, TASK_FOREVER, invioDati);
Task taskTimer(TASK_MINUTE, TASK_FOREVER, lcdPrintTimer);

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
      if(valoreLum > 700){
        analogWrite(ledPin,0);
      }else if(valoreLum < 200){
        analogWrite(ledPin,255);
      }else{
        int valoreLumMap = map(valoreLum, 200, 700, 100, 1);
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
  //Imposta il periodo di lettura dei sensori
  if(strcmp(topic, "command/periodoLetturaSensori") == 0){
    durataCiclo = (int) command.toFloat();
    taskLettura.setInterval(durataCiclo*TASK_MINUTE);
    taskInvioDati.setInterval(durataCiclo*TASK_MINUTE);
    durataPausaMinuti = durataCiclo;
  }
}

void setup()
{   
    //Inizializzazione Seriale    
    Serial.begin(115200);
    Serial.println("Booting...");
    
    //Inizializzazione LCD
    lcd.begin(16, 2);
    lcd.setCursor(0,0);
    lcd.print("Booting...");
    
    dht.setup(dhtPin, DHTesp::DHT11); // Inizializzazione sensore DHT11
    
    // Controllo di connessione sul sensore BMP180.
    if (!bmp.begin()) {
      Serial.println("Errore Connessione BMP180");
      while (1) {} //Altrimenti continua a stampare errori
    }

    //Chiamata alla procedura per la connessione WIFI
    setup_wifi();
    
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print("WiFi connesso");

    //Inizilizzazione Scheduler
    runner.init();
    
    //Aggiunta task allo scheudler e attivazione
    runner.addTask(taskLettura);
    runner.addTask(taskInvioDati);
    runner.addTask(taskTimer);
    
    taskLettura.enable();
    taskInvioDati.enable();
    taskTimer.enable();

    //In questo modo evito che l'invio dei dati venga eseguito prima della lettura
    taskInvioDati.delay(3000);
    taskTimer.delay(4000);

    Serial.println("Task Scheduler inizializzato");
    Serial.println();

    //Chiamata alla procedura per la connessione MQTT
    connectMQTT();
 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Booting");
    lcd.setCursor(0,1);
    lcd.print("Completato");
    lcd.clear();
}

void loop()
{
    //Esecuzione dello scheduler e del client MQTT
    runner.execute();
    client.loop();
}
