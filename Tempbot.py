#libreria python-telegram-bot
from telegram import InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext  import  Updater, MessageHandler, CommandHandler, CallbackQueryHandler, RegexHandler, Filters
#libreria per l'ouput sulla shell
import logging
#libreria per avviare processi da shell, usata per inviare richieste http ad arduino tramite il comando curl
import subprocess32

#enable logging, sfrutta la libreria per stampare i messaggi e gli errori, in questo caso stampa un messaggio di conferma all'avvio
logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',level=logging.INFO)
logger = logging.getLogger(__name__)
logging.info('Doing something')

#comanda ad arduino di settare la sua temperatura passando il valore indicato dall'utente via messaggio come parametro get
def setta(bot,update,**optional_args):
	#composizione del comando da lanciare
	comando='curl "192.168.43.84:5555/setta?token=386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M&chat=%s&soglia=' %update.message.chat_id
	comando = comando + '%s"'%optional_args['args'][0]
	#invia la richiesta http ad arduino
	subprocess32.call(comando, shell=True)

#Manda in chat lo schema da cui gli utenti possono selezionare le informazioni da richiedere ad arduino
#Al click verra richiamata una funzione che chiameremo Invia_info che agira in base al valore di callback (elemento cliccato dall'utetne)
def info(bot,update):
	keyboard =[
     [InlineKeyboardButton("Dati real-time", callback_data="1")],
     [InlineKeyboardButton("Temperatura media negli ultimi 20 minuti",callback_data="2")],
     [InlineKeyboardButton("Attivita ventola nell'ultima ora", callback_data="3")]
	]
	reply_markup=InlineKeyboardMarkup(keyboard)
	update.message.reply_text("Indica l'informazione che ti interessa", reply_markup=reply_markup)


#comanda ad arduino di inviare i dati richiesti
def Invia_info(bot,update):
	#query contiene il valore  del bottone cliccato, in base ad esso si compone il comando da lanciare
	query=update.callback_query
	if  query.data == "1":
		comando = 'curl "192.168.43.84:5555/attuale?token=386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M&chat=%s"' %query.message.chat_id
	elif query.data == "2":
		comando = 'curl "192.168.43.84:5555/media?token=386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M&chat=%s"' %query.message.chat_id
	else:
		comando = 'curl "192.168.43.84:5555/ultima?token=386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M&chat=%s"' %query.message.chat_id
	#invia la richiesta http ad arduino
	subprocess32.call(comando, shell=True)
	bot.editMessageText(text="Arduino risponde:",chat_id=query.message.chat_id,	message_id=query.message.message_id)

def main():
	#inizializza l'oggetto updater sul nostro bot, di cui indichiamo il token, l'updater ha il compito di rilevare aggiornamenti dai server di telegram riguardanti messaggi inviati al bot
	updater = Updater("386429372:AAG7xgNjODVqBFg517NpbmhDNDbpVEdtO5M")
	#inizializza l'oggetto dispatcher, che raccoglie gli aggiornamenti colti dall'updater e li gira a diversi handler in base al contenuto di questi
	dp=updater.dispatcher
	#Definiamo un handler per la ricezione del comando "info", questo handler lancia la funzione info
	dp.add_handler(CommandHandler('info',info))
	#Definiamo un handler per la ricezione del comando "setta", questo handler lancia la funzione setta
	dp.add_handler(CommandHandler('setta',setta,pass_args=True))
	#Definiamo un handler per la ricezione di risposte fornite dall'utente
	#in questo caso l'unica risposta di questo tipo puo essere ricevuta quando l'utente seleziona una delle opzioni offerte dalla funzione info, viene eseguita la funzione invia_info
	dp.add_handler(CallbackQueryHandler(Invia_info))

	#avvia polling del bot
	updater.start_polling()
	#esegue "updater.stop()" quando si usa ctrl+C
	updater.idle()

if __name__ == '__main__':
	main()
