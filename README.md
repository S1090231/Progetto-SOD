# Progetto-SOD

## OBIETTIVI
Gli obietti di questo progetto consistono in: 
- Acquisire dati dal sensore di luminosità e dal sensore di temperatura, pressione e altitudine.
- Gestire la comunicazione tra ESP32 e i vari sensori tramite protocollo MQTT
- Visualizzare i dati acquisiti tramite un interfaccia web 

<p align="center" style="margin-top: 10px;margin-bottom: 10px">
<img src="https://github.com/S1090231/Progetto-SOD/blob/main/schema iniziale.png" width="550" > 
</p>

## AMBIENTE DI SVILUPPO
Per lo sviluppo del progetto è stato utilizzato l'IDE ArduinoIDE

<p align="center" style="margin-top: 10px;margin-bottom: 10px">
<img src="https://github.com/S1090231/Progetto-SOD/blob/main/Arduino.png" width="550" > 
</p>

## CONFIGURAZIONE MQTT

Per quanto riguarda la configurazione del protocollo MQTT, è necessario installare, all'interno della macchina virtuale con OS Ubuntu 22.04, il software Mosquitto con il seguente comando: 
 ```
 sudo apt install -y mosquitto mosquitto-clients
 ```
Per verificare l'avvenuta installazione è necessario eseguire: 

 ```
 sudo systemfctl status mosquitto
 ```
## ACQUSITZIONE DEI DATI DA PARTE DELL'ESP32
Nel progetto l'ESP32 acquisisce i dati dal sensore di luminosità(BH1750), dal sensore di temperatura e pressione(BMP280) e dal modulo RTC (PCF8523). 

Questi dati saranno poi, attraverso il protocollo MQTT, inviati al broker MQTT per poi renderli visibili su un'interfaccia web, creata con l'applicatico Node-RED intallato tramite terminale con il comando: 

 ```
 npm install -g --unsafe-perm node-red
 ```
E si otterrà cosi un'interfaccia del tipo in figura. 

<p align="center" style="margin-top: 10px;margin-bottom: 10px">
<img src="https://github.com/S1090231/Progetto-SOD/blob/main/nodered.png" width="550" > 
</p>
