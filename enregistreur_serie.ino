	/*		enregistrement des donnees serie sur carte sd
	 * 		nomage du fichier enregistre a partir du 1 er chanmp de reception
	 * 		devant contenir la date et l heure 
	 * /

	/*			history
	 * 	0.0.1 	creation du programme, enregistrement de trames provenant du port serie acquisition courant

	*/

/*	recapitulatif des i/o arduino
	#define rx		D0			//	serial rx
	#define tx		D1			//	serial tx
	#define 		D2			// 	led_pin
	#define 		D3			// 	
	#define 		D4			// 	
	#define 		D5			// 	
	#define 		D6			// 	
	#define 		D7			// 	
	#define 		D8			// 	
	#define 		D9			// 	
	#define cs		D10			// 	SPI SD	Chip Select
	#define mosi	D11			//	SPI SD	MOSI
	#define miso	D12			//	SPI SD	MISO
	#define sck 	D13			//	SPI SD	SCK
	#define 		A0			// 	
	#define 		A1			// 	
	#define 		A2			// 	
	#define 		A3			// 	
	#define sda		A4			// I2C
	#define scl		A5			// I2C
*/

// programme :
const char title[] = "enregistreur acq stat I";
const char version[] = "0.0.1";

#include <Arduino.h>
#include <SD.h>
#include <Timer.h>
#include <Streaming.h>        //http://arduiniana.org/libraries/streaming/

//	definition des constantes
#define INPUT_SIZE 63						// taille de la variable de message serie

const unsigned int BAUD_SERIAL = 19200 ;	// vitesse liaison console/bluetooth affichée à diviser par 2
const int LedPin = 2;						// the number of the LED pin
const char start	=	0x32;				// caractere 2 ( de 2018 ) 

// definition des variables
char filename[12] = {0};
char message [INPUT_SIZE + 1] = {0} ;		// espace de stockage pour trame complete
char* ptr = message ;						//pointeur sur message[]
bool stringComplete = false;				// whether the string is complete
int ltrame = 0;
bool sd_fault = true;
int tim_sd = 0;

struct date{
	char Year[4];
	char sep0;
	char Month[2];
	char sep1;
	char Day[2];
	char end;
} ;

// Timer de temporisation
Timer timber;

//			****	sous programmes		****

char* get_filename(char *bfr){

	date aujourdhui;
	memcpy ( &aujourdhui.Year, &message, 10 );
	aujourdhui.sep0 = '\0';
	aujourdhui.sep1 = '\0';
	aujourdhui.end = '\0';
	/*Serial << F("message : ") << message << endl;
	Serial << F("annee : ") << aujourdhui.Year << endl;
	Serial << F("mois : ") << aujourdhui.Month << endl;
	Serial << F("jour : ") << aujourdhui.Day << endl;*/
	sprintf(bfr, "%04s%02s%02s.csv", aujourdhui.Year, aujourdhui.Month, aujourdhui.Day);  //  %d pour un int
	//Serial << F("fichier : ") << bfr << endl;
	return bfr ;
}// end of get_filename

void save_buffer(){

	File logfile;
	char *bfr = filename;
	get_filename(bfr);
	Serial << F("Fichier : ") << filename << endl;
	logfile = SD.open(filename, FILE_WRITE);
	if(logfile){
		logfile.print(message);
		digitalWrite(LedPin, LOW);		// reception
		logfile.close(); // close the file:
	}
	else{
		timber.oscillate(LedPin, 200, LOW, 2);
		Serial << F("Erreur d enregistrement sur sdcard") << endl;
	}
	Serial.println(message);

}// end of save_buffer
 
//			****	fin des sous programmes		****

void start_sd(){
	if(SD.begin(10)){
		sd_fault = false;
		timber.oscillate(LedPin, 200, LOW, 1);
		timber.stop(tim_sd);
		Serial.println("\n sdcard detectee.");
	}
	else{
		sd_fault = true;
		timber.oscillate(LedPin, 100, LOW, 5);
		Serial.print(".");
	}
}

void setup(){
	pinMode(LedPin, OUTPUT);
	// Open serial communications and wait for port to open:
	Serial.begin(BAUD_SERIAL);
	tim_sd = timber.every(5000, start_sd);
	timber.pulse(LedPin, 500, LOW);
	Serial << F("attente sdcard ") << endl;
}// end of setup

void loop(){

	if (stringComplete) {
		if(!sd_fault){
			save_buffer();
			stringComplete = false;
			timber.pulse(LedPin, 500, LOW);
		}
		else{
			while (Serial.read() > 0){
				; // effacement buffer
			}

		}
	}
		timber.update();	// mise à jour du timer*/

}// end of loop

void serialEvent() {

	static char* fifo = ptr ; // pointeur sur la chaine de message
	static bool debut = false;
	static bool RC = false;

	while (Serial.available()) {
		// get the data:
		*fifo = Serial.read();
		//Serial.print(*fifo, HEX);
		if (*fifo == start){
			if (!debut){
				debut = true;
				digitalWrite(LedPin, HIGH);		// reception
				// debut de trame trouvé remise à 0 du pointeur
				fifo = ptr;
				ltrame = 1;
			}
			fifo ++;
			ltrame ++;
		}
		/*else if ( *fifo  == 0x0D){ // les trames de acq_I N ONT PAS DE \r
			RC = true;
			fifo ++;
			ltrame ++;
		}*/
		else if (*fifo == 0x0A) {
			fifo ++;
			ltrame ++;
			//if ( RC ){
			if ( debut ){
				*fifo = '\0' ;
				digitalWrite(LedPin, HIGH);		// reception FIN
				stringComplete = true ;
				debut = false;
				RC = false;
				break ;
				
			}
			else{
				fifo = ptr ;
				Serial.print("fin sans debut\n");
				timber.pulse(LedPin, 500, HIGH);
			//	}
			}

		}
		else{
			if (fifo >= (ptr + INPUT_SIZE -1)){
				fifo = ptr ;
				ltrame = 0;
				Serial.print("depassement\n");
			}
			fifo ++;
			ltrame ++;
		}
	}

} // end of serialEvent


