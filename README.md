# Progetto-SOD

##OBIETTIVI
Gli obietti di questo progetto consistono in: 
- Acquisire dati dal sensore di luminosità e dal sensore di temperatura, pressione e altitudine.
- Gestire la comunicazione tra ESP32 e i vari sensori tramite protocollo MQTT
- Visualizzare i dati acquisiti tramite un interfaccia web

##PROGETTAZIONE E SVILUPPO
###Ambiente di sviluppo
Il progetto è stato implementanto su una macchina virtuale con sistema operativo Linux, sviluppato tramite l'ide ArduinoIde per la gestione delle librerie. <br>
Per poter utilizzare l'ESP32 come broker è stato utilizzato il software open-source [Mosquitto] (https://mosquitto.org/). <br>
