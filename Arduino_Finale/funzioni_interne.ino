void setta_soglia(float t)
{
  if (t<150 && t>-55) soglia=t;
  return;  
}
void incrementa(){
  setta_soglia(soglia+1);
  return;
}

void decrementa(){
  setta_soglia(soglia-1);
  return;
}
//-------------------------------------------------------------------------------------------//
//Ritorna la data e l'ora in formato mm/dd/yy hh:mm:ss tramite una stringa
String getTimeStamp() {
  String result;
  Process time; // crea un'instanza di un process che verrà utilizzata per ottentere il tempo
  time.begin("date");// inizializza il process per eseguire il comando date sulla shell di arduino
  time.addParameter("+%D-%T");  // parametri: D per mm/dd/yy, T per hh:mm:ss
  time.run();
  //Lettura output del comando
  while (time.available() > 0) {//fino a quando il numero di byte disponibili per la scrittura è maggiore di zero
    char c = time.read();//leggo un carattere e lo metto in c
    if (c != '\n') {//se c è \n mi fermo
      result += c; // appendo c al risultato
    }
  }
  return result;
}
//-------------------------------------------------------------------------------------------//
//Funzioni che restituiscono i dati richiesti dall'utente tramite l'interfaccia python

//restituisce la media dei  valori presenti nella memoria EEPROM
float media(){
  int addr=0;
  float somma=0;
  float temp;
  float cicli=0;
  while(addr<1024){ //legge tutta la EEPROM
    if(EEPROM.get(addr,temp)){
        if(isnan(temp))break;
        cicli+=1.0;
        somma+=temp;
        addr=addr+sizeof(float);
     }
  }
    
  return somma/cicli;
}

//Restituisce il numero di secondi in cui la ventola è stata accesa nell'ultima ora
String ultima_ora(){
  int secondi = 0;
  time_t attuale = get_numeric_timestamp(getTimeStamp());// prende il timestamp attuale
  String riga;
  int c;
  File report = FileSystem.open("/mnt/sda1/temperatura.txt",FILE_READ);  
  while((c=report.read())>0){
    riga+=(char)c;    
    if((char)c=='\n'){
      if(((int)attuale-(int)get_numeric_timestamp(riga))<=3600) secondi++;  //aumenta di un secondo per ogni riga il cui timestamp differisca di massimo un'ora(indietro) dall'attuale
      riga="";
    }
  }
  report.close();  
  return tempo(secondi);
}

//-------------------------------------------------------------------------------------------//
//Questa funzione dato un numero di secondi restituisce una stringa in cui questo viene espresso come x minuti e y secondi
String tempo(int secondi){
  int minuti=0;
  while(secondi>59){
    secondi = secondi - 60;
    minuti++;
  }       
  String result = "";
  if(minuti>0){
    if(minuti==1){
      result= "1 minuto e ";
    }else result= (String)minuti + " minuti e ";
  }
  result+= (String)secondi;
  result+=" secondi";
  return result;
}
//restituisce il timestamp corrispondente alla data stampata su una stringa secondo la formattazione data dalla funzione getTimeStamp()
time_t get_numeric_timestamp(String riga){
  time_t timestamp;
  String anno,mese,giorno,ora,minuto,secondo;
  String data = splitString(riga,'-',0);
  String orario = splitString(riga,'-',1);
  mese=splitString(data,'/',0);
  giorno=splitString(data,'/',1);
  anno=splitString(data,'/',2);
  ora=splitString(orario,':',0);
  minuto=splitString(orario,':',1);
  secondo=splitString(orario,':',2);
  timestamp=tmConvert_t(anno.toInt(),mese.toInt(),giorno.toInt(),ora.toInt(),minuto.toInt(),secondo.toInt());
  return timestamp;
}

//Restituisce il timestamp di una data indicata specificando singolarmente anno,mese,giorno,ora,minuti,secondi
time_t tmConvert_t(int YYYY, int MM, int DD, int hh, int mm, int ss){
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet); 
}

//Restituisce la sottostringa estratta secondo il separatore indicato alla ricorrenza indicata nell'ultimo parametro
String splitString(String str, char sep, int index){
  int found = 0;//variabile atta alla verifica della presenza del separatore nella stringa; contiene il numero di istanze trovate del separatore
  int strIdx[] = { 0, -1 };
  int maxIdx = str.length() - 1; 
  for (int i = 0; i <= maxIdx && found <= index; i++){// se i è minore della lunghezza della stringa e found è minore uguale dell'indice specificato
    if (str.charAt(i) == sep || i == maxIdx){//se viene trovato il carattere separatore
      found++;
      strIdx[0] = strIdx[1] + 1; //setta l'indice di inizio stringa
      strIdx[1] = (i == maxIdx) ? i+1 : i;§//setta l'indice di fine stringa
    }
  }
  return found > index ? str.substring(strIdx[0], strIdx[1]) : "";//se found è > index ritorna la stringa contenuta tra le posizioni di inizio e fine
}

