//Questa funzione interpreta e soddisfa le richieste http ricevute da arduino
void process(BridgeClient client) {
  // read the command
  digitalWrite(13,HIGH);
  String command = client.readStringUntil('/');//scorre la stringa
  command=client.readStringUntil('?');//legge la richiesta e salva in command la funzione da eseguire
  String token = client.readStringUntil('=');//scorre la stringa
  token=client.readStringUntil('&');//id del bot
  String chat = client.readStringUntil('=');//scorre la stringa
    
  if(command == "attuale"){
    chat=client.readStringUntil(' ');//id chat
    float cel=analogRead(1);
    cel=(500.0/1024.0)*cel;
    real_time(cel,chat);//vedi giù
  }
  if(command == "media"){
    chat=client.readStringUntil(' ');//id chat
    invia_media(chat); //vedi giù  
  }
  if(command == "ultima"){
       chat=client.readStringUntil(' ');
       invia_ultima(chat);//vedi giù
  }
  if(command == "setta"){
    chat=client.readStringUntil('&');//prende la chat
    String new_soglia =  client.readStringUntil('='); //scorre la stringa
    new_soglia = client.readStringUntil(' ');//prende il parametro in ingresso
    int soglia_int = new_soglia.toInt();
    setta_soglia((float)soglia_int);//vedi funzioni interna;
    if(soglia_int<150 && soglia_int>-55) invia_messaggio("Temperatura di soglia aggiornata!",chat);
    else invia_messaggio("Temperatura non consentita, soglia non aggiornata!",chat);
  }
}
// Invia una richiesta http dato l'URL
void runCurl(String comando) { 
  Process p;
  p.begin("curl"); //inizializza il processo per eseguire il comando curl 
  p.addParameter("-k");// opzione curl che impedisce la verifica del certificato ssl
  p.addParameter(comando);// aggiunge il comando da inviare al bot
  p.run();// esegue il process
}
//Notifica tramite un messaggio su telegram che la temperatura di soglia è stata superata
void notifica(String chat){
  String testo = "WARNING: Temperatura di soglia superata";
  invia_messaggio(testo, chat);
}

//Funzione generica per comandare l'invio di un messaggio al bot di telegram ad una chat indicata
void invia_messaggio(String testo, String chat){
  String comando = "https://api.telegram.org/bot386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M/sendmessage?text=";
  comando += testo;
  comando += "&chat_id=" + chat; 
  runCurl(comando);
}
//Comanda l'invio del messaggio per i dati attuali al bot di telegram
void real_time(float cel, String chat){
  String testo = "Temperatura: " + (String)cel + "°C";
  invia_messaggio(testo, chat);
  testo ="Soglia: " + (String)soglia + "°C";
  invia_messaggio(testo, chat);
}
//Comanda l'invio del messaggio per il dato sulla media al bot di telegram
void invia_media(String chat){
  String valore = (String)media();//vedi funzioni interne
  String testo = "Media: " + valore + "°C";
  invia_messaggio(testo, chat);
}
//Comanda l'invio del messaggio  per il dato sull'ultima ora al bot di telegram
void invia_ultima(String chat){
  String valore = ultima_ora();//vedi funzioni interne
  String testo = valore;
  invia_messaggio(testo, chat);
}

