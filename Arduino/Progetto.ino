#include <EEPROM.h> //memoria non volatile in cui immagazziniamo dati che vogliamo conservare al riavvio
#include <FileIO.h>
#include <time.h>
#include <TimeLib.h>
#include<Bridge.h> // libreria che permette le chiamate res, utile per le funzionalità di rete
#include<BridgeServer.h>
#include<BridgeClient.h>
#include<Process.h>//Necessaria per il funzionamento di bridge, permette di fare il run di un comando da Shell in modo asincrono
#include<HttpClient.h> // permette di effettuare HTTPRequest
#include<SoftwareSerial.h> //contiene le funzioni per la gestione delle seriali software
BridgeServer server; // di default ascolta sulla porta 5555
float soglia;
int addr = 0;
int first_time=0; //necessaria per il calcolo del tempo trascorso con la funzione millis()
int current_time=0;//necessaria per il calcolo del tempo trascorso con la funzione millis()
int loop_time=0;//necessaria per il calcolo del tempo trascorso con la funzione millis()
boolean sd=false;
boolean flag=false;
SoftwareSerial myserial(10,11); // RX Pin 10, TX Pin 11

void setup() {
  pinMode(10,INPUT); //settiamo in input il pin di ricezione della seriale software
  pinMode(11,OUTPUT); //settiamo in output il pin di trasmissione della seriale software
  myserial.begin(9600);//fissiamo la baud rate
  setta_soglia(33.0);

  pinMode(7,OUTPUT); // settiamo in output il led giallo che indica la scrittura in EEPROM7
  pinMode(8,OUTPUT); // settiamo in output il led rosso che indica la scrittura in SD
  pinMode(2, OUTPUT);
  pinMode(13,OUTPUT);// settiamo in output il led l13 integrato su arduino7
  Bridge.begin(); //Inizializza bridge, facilitando la comunicazione tra l'avr (microcontrollore) e il processore linux
  Serial.begin(9600); //Inizializza la seriale hardware con baud rate di 9600
  if(!FileSystem.begin()){//inizializza la scheda sd
    Serial.println("ERROR:Scheda sd non riconosciuta");
  }
  else
  {
    sd=true;
    Serial.println("SD rilevata")
    //se non esistono la directory e il file in cui andranno salvati i valori di temperatura questi vengono creati
    if(!FileSystem.exists("/mnt/sda1/temperatura.txt")){
      File temperature=FileSystem.open("sda1/temperatura.txt",FILE_WRITE);
      temperature.close();
    }
  }
  server.begin(); // inizializza le funzionalità server di arduino
  while(! SerialUSB); // Aspetta che la porta seriale venga connessa
  SerialUSB.println(getTimeStamp() + " - Progetto Arduino\n");
  }

void loop() {

//-------------------------------------------------------------------------------------------//
// Arduino si mette in ascolto sulla seriale software: se riceve un 1 (da STM32) incrementa
//la soglia di un grado, se riceve 2 decrementa la soglia di 1 grado

  myserial.listen();// mette in ascolto la seriale software su una porta; non vi può essere più di una seriale in ascolto; i dati in arrivo sulle altre porte verranno scartati
  if (myserial.available){// prende il numero di byte disponibile per la lettura dalla seriale software
    if(myserial.read()==49) //legge dalla seriale e confronta il valore
    {
      incrementa();
      Serial.println("Nuova soglia" + (String)soglia)
    }
    if(myserial.read()==50){
      decrementa();
      Serial.println("Nuova soglia: " + (String)soglia);
    }
  }
//-------------------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------//

current_time=millis();//ritorna il numero di millisecondi dall'inizio dell'esecuzione
if(current_time-loop_time>=1000){ //fa in modo che le seguenti operazioni siano effettuate ogni secondo
    digitalWrite(7,LOW);  // imposta il pin 7 a LOW
    digitalWrite(8,LOW);  // imposta il pin 8 a LOW
    digitalWrite(13,LOW); // imposta il pin 13 a LOW
    loop_time=current_time; //necessario nel prossimo ciclo come riferimento

    //leggo il valore dato dal sensore di temperatura collegato al pin analogico A1 e deduco la  temperatura in gradi Celsius
    float val = analogRead(1);// lettura dal pin analogico 1;
    float cel = (val/1024)*500; //0->-55°C; 1024->150°C (FORSE)
    if(cel>soglia){
      flag=true;  //setta a true il flag di superamento soglia
      myserial.write('1'); //Scrive 1 in seriale software
    }else{
      myserial.write('2'); //Scrive 2 in seriale software
    }

    //Verifico se sono passati 5 secondi dall'utima scrittura in EEPROM ed eventualmente salvo il nuovo valore di temperatura
    if(current_time-first_time>=5000){
        first_time = current_time;
        if(EEPROM.put(addr, cel))//scrive in EEPROM cel alla locazione addr inizializzata a zero
        {
          Serial.println(cel);
          digitalWrite(7,HIGH); //ogni volta che avviene un accesso alla memoria EEPROM il led collegato al pin 7 viene acceso per circa un secondo
          addr = addr + sizeof(float); //incremento il puntatore al prossimo indirizzo di scrittura  per la EEPROM
        }
        if(addr==EEPROM.length())//Se la EEPROM è piena
        {
          addr=0; //sovrascrive dall'inizio
        }
         if(flag){
          notifica("chat_id"); //invia un messaggio telegram di warning
          flag=false;// setta il flag a false per la prossima iterazione
          }
      }
      //se la temperatura rilevata è maggiore  della soglia impostata, salvo nella memoria SD la data, l'ora e la temperatura attuali.
      if(cel>soglia && sd){
      digitalWrite(8,HIGH); //accende il pin rosso
      analogWrite(3,255); //Da corrente al transistor, che altrimenti bloccherebbe il passaggio della corrente verso la ventola (interruttore in regione non lineare)
      File temperature=FileSystem.open("/mnt/sda1/temperatura.txt",FILE_APPEND); //append sul file temperatura all'interno della SD
      temperature.println(getTimeStamp() + "- Temperatura rilevata: " + (String)cel + "°C");// stampa la data
      Serial.println("Sto scrivendo nella SD");
      temperature.close(); // Chiude il file system dell'SD
      }else analogWrite(3,0); //spegne la ventola se accesa
      }
      BridgeClient client = server.accept();  // Verifica la presenza di eventuali client
      if (client) {
      process(client); //La  richiesta viene processata(Vedi in Funzioni_Web)
      client.stop();
      }

}

//-------------------------------------------------------------------------------------------//
