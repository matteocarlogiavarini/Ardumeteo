# Ardumeteo

Ardumeteo ha come obbiettivo la realizzazione di una piattaforma integrativa per un impianto di domotica. 
L'idea è quella di avere una piattaforma in grado di rilevare valori ambientali indoor come temperatura, umidità, luminosità, pressione ed altitudine, condividerli tramite protocollo MQTT con la piattaforma Home Assistant.
Tramite l'interfaccia grafica di Home Assitant si potranno visualizzare i grafici relativi ai dati rilevati, sarà possibile scegliere se controllare le luci interne in maniera manulare tramite l'uso di un potenziometro collegato alla piattaforma, oppure in maniera automatica in base ai valori rilevati dal sensore di luminostà. Inoltre sempre tramite interfaccia grafica sarà possibile impostare il periodo di lettura dei dati. La logica della gestione della piattaforma è implementata su Home Assitant grazie all'utilizzo di trigger ed automazioni che invieranno su topic MQTT gli opportuni comandi. 
Per comodità ho scelto di installare il broker MQTT su un PC Linux e non su un raspberry come è consueto fare. 
I vari sensori ambientali sono stati collegati ad un ESP32 connesso tramite Wifi mentre il potenziometro per il controllo manuale delle luci è stato collegato ad un clone di Arduino Uno connesso con ethernet shield, mentre le luci interne sono simulate da led collegati ad entrambe le piattaforme.  
Ho scelto di utilizzare due dispositivi per sottolineare la possibilità di avere più dispositvi che interagiscono fra di loro. 

## Hardware

- ESP32
- Sunfounder UNO con Ethernet Shield
- Sensore temperatura ed umidità DHT11
- Senosre temperatura, pressione ed altitudine BMP180
- Fotoresistenza con resistenza da 110 ohms
- Dislpay LCD 16x2 con resistenza da 220 ohms
- 2 diodi led
- Potenziometro 50K

## Links:

- Home Assistant - https://www.home-assistant.io/
- MQTT - http://mqtt.org/
- PubSubClient (MQTT) - https://pubsubclient.knolleary.net/
- SPI (Ehternet shiled) - https://www.arduino.cc/en/Reference/SPI
- Ehternet - https://www.arduino.cc/en/Reference/Ethernet
- Wire (I2C per bmp180) - https://www.arduino.cc/en/Reference/Wire
- DHT11 - https://github.com/beegee-tokyo/DHTesp
- WiFi -https://www.arduino.cc/en/Reference/WiFi
- BMP 180 - https://github.com/adafruit/Adafruit_BMP085_Unified
- TaskScheduler - https://github.com/arkhipenko/TaskScheduler
- LiquidCrystal - https://www.arduino.cc/en/Reference/LiquidCrystal
