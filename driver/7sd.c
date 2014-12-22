/*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 7sd driver
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Por Juian Perelli y Maximiliano Padulo
 * (L) 2009
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * DTS: señal de Datos (PIN 4 - DB9) - Blanco
 * RTS: señal de Clock (PIN 7 - DB9) - Azul
 * TxD: señal de strobe  (PIN 3 - DB9) (todavia no implementado)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */


#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>     //nanosleep()
#include <stdio.h>    //getch
#include <unistd.h>   //getch
#include <popt.h>     //parse de argumentos    ! ! usar opcion -lpopt del gcc, para linkear esto ! !


//definicion de booleano
#define FALSE 0
#define TRUE 1

#define INTERACTIVO   1  // valores de MODO de funcionamiento
#define AUTO_STRING   2
#define VUMETRO       3
#define DERECHA       1  // valores de DESPLAZAMIENTO_SENTIDO y de ALINEACION
#define IZQUIERDA     2
#define CENTRO        3
#define CICLICO       1  // valores de DESPLAZAMIENTO_TIPOO
#define ALTERNANTE    2
#define DESHABILITADO 3

//var declaradass globales... VALORES DE INICIALIZACION
int   DELAY_TRANSMISION_BITS     = 0; //unidad: milisegundos
int   CANT_CARACTERES_DISPLAY    = 12;
int   MODO                       = AUTO_STRING;
int   DESPLAZAMIENTO_SENTIDO     = DERECHA;  //para DESPLAZAMIENTO_TIPO == CICLICO
int   DESPLAZAMIENTO_DELAY       = 100;  //tiempo entre la trasnmision de un caracter y el siguiente (milisegundos)
int   DESPLAZAMIENTO_TIPO        = CICLICO;
int   ALINEACION                 = IZQUIERDA;
char* DEV_SERIE                  = "/dev/ttyS0";



/* we need a termios structure to clear the HUPCL bit */
struct termios tio;


// devuelve el string pasado como parametro, invertido
char* reverse(char* str)
{
	int i, j, len;
	char temp;
	char *ptr=NULL;
	i=j=len=temp=0;

	len=strlen(str);
	ptr=malloc(sizeof(char)*(len+1));
	ptr=strcpy(ptr,str);
	for (i=0, j=len-1; i<=j; i++, j--)
	{
		temp=ptr[i];
		ptr[i]=ptr[j];
		ptr[j]=temp;
	}
	return ptr;
}


// funcion para dormir en milisegundos (requiere time.h)
int msleep(unsigned long milisec)  
{  
	struct timespec req={0};  
	time_t sec=(int)(milisec/1000);  
	milisec=milisec-(sec*1000);  
	req.tv_sec=sec;  
	req.tv_nsec=milisec*1000000L;  
	while(nanosleep(&req,&req)==-1)  
		continue;  
	return 1;  
}


// espera un caracter un caracter (sin esperar retorno de carro)
int getch(void)
{
	int ch;
	struct termios oldt;
	struct termios newt;
	tcgetattr(STDIN_FILENO, &oldt); /*store old settings */
	newt = oldt; /* copy old settings to new settings */
	newt.c_lflag &= ~(ICANON | ECHO); /* make one change to old settings in new settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &newt); /*apply the new settings immediatly */
	ch = getchar(); /* standard getchar call */
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt); /*reapply the old settings */
	return ch; /*return received char */
}


// envia un bit al dispositivo 7sd
//   - fd es el file descriptor del puerto serie
//   - status es el status del puerto serie
//   - valor es el valor del bit a transmitir (0/1)
void enviar(int fd, int status, int valor)
{

	// pongo el valor de "valor" en el bit a transmitir
	if ( valor == 0 )  
		status &= ~TIOCM_DTR;
	else
		status |= TIOCM_DTR;
 
//	ioctl(fd, TIOCMSET, &status);   //poner valor

	//mando un uno y despues un cero en el clock para que lea el bit transmitido
	status |= TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);  //esta linea envia un 1 @clock y el valor @dato
	tcflush(fd, TCIOFLUSH); //flush bytes de entrada y de salida
	msleep(DELAY_TRANSMISION_BITS);

	status &= ~TIOCM_RTS;
	ioctl(fd, TIOCMSET, &status);
	tcflush(fd, TCIOFLUSH); //flush bytes de entrada y de salida
	
	msleep(DELAY_TRANSMISION_BITS);
	
}


// convierte el caracter pasado por parametro a una secuencia de unos y ceros que representan el caracter en binario para un display de 7 segmentos
int* segmento7(char c)
{
	int  *binario = malloc(sizeof(int)*(8));
	char *auxStr  = malloc(sizeof(char)*(8));
	switch (c)
	{	//LETRAS
		case 'a': auxStr = "01110111"; break;
		case 'b': auxStr = "01111100"; break;
		case 'c': auxStr = "01011000"; break;
		case 'd': auxStr = "01011110"; break;
		case 'e': auxStr = "01111001"; break;
		case 'f': auxStr = "01110001"; break;
		case 'g': auxStr = "01111101"; break;
		case 'h': auxStr = "01110100"; break;
		case 'i': auxStr = "00110000"; break;
		case 'j': auxStr = "00011110"; break;
		case 'k': auxStr = "01110110"; break;
		case 'l': auxStr = "00111000"; break;
		case 'm': auxStr = "00010101"; break;
		case 'n': auxStr = "01010100"; break;
		case 'o': auxStr = "01011100"; break;
		case 'p': auxStr = "01110011"; break;
		case 'q': auxStr = "01100111"; break;
		case 'r': auxStr = "01010000"; break;
		case 's': auxStr = "01101101"; break;
		case 't': auxStr = "01111000"; break;
		case 'u': auxStr = "00111110"; break;
		case 'v': auxStr = "00011100"; break;
		case 'w': auxStr = "00101010"; break;
		case 'x': auxStr = "01110110"; break;
		case 'y': auxStr = "01101110"; break;
		case 'z': auxStr = "01011011"; break;

		//NUMEROS
		case '0': auxStr = "00111111"; break;
		case '1': auxStr = "00000110"; break;
		case '2': auxStr = "01011011"; break;
		case '3': auxStr = "01001111"; break;
		case '4': auxStr = "01100110"; break;
		case '5': auxStr = "01101101"; break;
		case '6': auxStr = "01111101"; break;
		case '7': auxStr = "00000111"; break;
		case '8': auxStr = "01111111"; break;
		case '9': auxStr = "01101111"; break;
    
		//ESPECIALES
		// case '.': auxStr = "10000000"; break;
		case '-': auxStr = "01000000"; break;
		case '_': auxStr = "00001000"; break;
		case '=': auxStr = "01001000"; break;
		case ' ': auxStr = "00000000"; break;
		case '?': auxStr = "01010011"; break;
		case '"': auxStr = "00100010"; break;
		case '\'': auxStr = "00100000"; break;

		default : auxStr = "00000000"; break;
	}

	int i;
	for ( i=0 ; i <= 7 ; i++)
		binario[i] = auxStr[i]=='1' ? 1 : 0;

	return binario;
}


//transmite un caracter al hardware
//   - fd es el file descriptor del puerto serie
//   - status es el status del puerto serie
//   - c es el caracter a mostrar en el display
int transmitirCaracter(int fd, int status, char c)
{
	int *s7 = segmento7(c);
	int i; 	

	for ( i=0 ; i <= 7 ; i++)
		enviar(fd, status, s7[i]);

	write(fd, "S", 1); //mando ruido en la linea STROBE

}


int main(int argc, char *argv[])
{
	// puerto	
	int fd;
	int status;
	
	// especifico de la implementacion de los algoritmos de movimiento
	int i;
	char *display = malloc(sizeof(char)*(CANT_CARACTERES_DISPLAY));
	int pos = -1;	
	int moviendoALaDerecha = TRUE;

	
	// PARSE ARGUMENTOS EN VARIABLES (popt)

	//valores por defecto
	char *CADENA                     = NULL;
	int   VERBOSE                    = 0;

	char c;
	poptContext optCon;   // context for parsing command-line options

	struct poptOption optionsTable[] = {
		// longname     short  arginfo         *arg      val      desc ARGdesc
		{ "interactivo", 'i', POPT_ARG_VAL   , &MODO                    , INTERACTIVO  , "Habilita el modo con entrada interactiva desde teclado (valor por defecto)", NULL },
		{ "string"     , 's', POPT_ARG_STRING, &CADENA                  , 0            , "Habilita el modo de envio de string por parametro", "String a enviar al dispositivo" },
		{ "vumetro"    , 'v', POPT_ARG_VAL   , &MODO                    , VUMETRO      , "Habilita el modo vumetro (todavia no implementado)", NULL },
		{ "mizquierda" , 'z', POPT_ARG_VAL   , &DESPLAZAMIENTO_SENTIDO  , IZQUIERDA    , "Establece el sentido de desplazamiento del mensaje hacia la izquierda", NULL },
		{ "mderecha"   , 'd', POPT_ARG_VAL   , &DESPLAZAMIENTO_SENTIDO  , DERECHA      , "Establece el sentido de desplazamiento del mensaje hacia la derecha (valor por defecto)", NULL },
		{ "aderecha"   , 'R', POPT_ARG_VAL   , &ALINEACION              , DERECHA      , "Establece la alineacion a la derecha", NULL },
		{ "aizquierda" , 'L', POPT_ARG_VAL   , &ALINEACION              , IZQUIERDA    , "Establece la alineacion a la izquierda (valor por defecto)", NULL },
		{ "acentro"    , 'C', POPT_ARG_VAL   , &ALINEACION              , CENTRO       , "Establece la alineacion al centro", NULL },
		{ "ciclico"    , 'c', POPT_ARG_VAL   , &DESPLAZAMIENTO_TIPO     , CICLICO      , "Movimiento en forma ciclica (valor por defecto)", NULL },
		{ "alternante" , 'a', POPT_ARG_VAL   , &DESPLAZAMIENTO_TIPO     , ALTERNANTE   , "Movimiento en forma alternante", NULL },
		{ "estatico"   , 'x', POPT_ARG_VAL   , &DESPLAZAMIENTO_TIPO     , DESHABILITADO, "Sin movimiento (trunca caracteres)", NULL },
		{ "tbit"       , 'T', POPT_ARG_FLOAT , &DELAY_TRANSMISION_BITS  , 0            , "Establece el tiempo de delay entre el envio de un bit y el siguiente (valor por defecto = 0)", "Tiempo en milisegundos (numero real)" },
		{ "tcaracter"  , 't', POPT_ARG_FLOAT , &DESPLAZAMIENTO_DELAY    , 0            , "Establece el tiempo de delay existente entre un frame de movimiento y el siguiente (valor por defecto = 500)", "Tiempo en milisegundos (numero real)" },
		{ "dispositivo", 'D', POPT_ARG_STRING, &DEV_SERIE               , 0            , "Dispositivo serie que conecta al hardware 7sd (valor por defecto \"/dev/ttyS0\")", "Archivo de dispositivo serie (/dev/ttyS0, /dev/ttyS1 ...)" },
		{ "maxcaract"  , 'N', POPT_ARG_INT   , &CANT_CARACTERES_DISPLAY , 0            , "Establece el numero de caracteres a utilizar del dispositivo de hardware (valor por defecto = 12)", "Cantidad de caracteres (numero entero)" },
		{ "verbose"    , 'V', POPT_ARG_VAL   , &VERBOSE                 , 1            , "Mostrar salidas (modo debug)", NULL },
		POPT_AUTOHELP
		POPT_TABLEEND
	};

	optCon = poptGetContext(NULL, argc, (const char **)argv, optionsTable, 0);

	if (argc < 2) {
		poptPrintUsage(optCon, stderr, 0);
		exit(1);
	}

	// Now do options processing, get portname
	while ((c = poptGetNextOpt(optCon)) >= 0) if ( c == 's' ) MODO = AUTO_STRING;
	
	if ( (c < -1) || ( poptGetArgs(optCon) != 0 ) ){
		/* an error occurred during option processing */
		fprintf(stderr, "%s: %s\n",
				poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
				poptStrerror(c));
		return 1;
	}

	// FIN PARSE ARGUMENTOS EN VARIABLES

	if (VERBOSE) printf("%s\n", CADENA);





	// APERTURA E INICIALIZACION DEL PUERTO SERIE
	//  Chequear inicializacion de la linea TxD para hacer uso del strobe
	//  (en particular el registro de configuracion de la cola fifo: bit de arranque y parada, baudios, etc)
	//     write(fd, "00000000", 5); //asi se escribe en la linea TxD

	if ((fd = open(DEV_SERIE, O_RDWR)) < 0)
	{
		printf("No se pudo abrir el puerto %s\nPrograma terminado\n", DEV_SERIE);
	 	exit(1);
	}
	tcgetattr(fd, &tio);          /* get the termio information */
	tio.c_cflag &= ~HUPCL;        /* clear the HUPCL bit */
	tcsetattr(fd, TCSANOW, &tio); /* set the termio information (set HUPCL bit)*/
  
	ioctl(fd, TIOCMGET, &status); /* get the serial port status */

	// FIN APERTURA DEL PUERTO



	// ALGORITMOS DE MANEJO DE CADENAS
	//  Esto es independiente del puerto serie, hace llamadas al puerto, pero funciona de manera independiente

	switch ( MODO )
	{
	case INTERACTIVO:
		while (c!=27) {transmitirCaracter(fd, status, c=getch()); if ( VERBOSE ) printf("%c", c); }
	break;
	case AUTO_STRING:
		if ( strlen(CADENA) > CANT_CARACTERES_DISPLAY )	
		{
			switch ( DESPLAZAMIENTO_TIPO )
			{
			case ALTERNANTE:
				while ( TRUE )
				{
					if ( strlen(CADENA) <= pos+CANT_CARACTERES_DISPLAY ) moviendoALaDerecha = FALSE;
					if ( pos <= 0 ) moviendoALaDerecha = TRUE;
					if ( moviendoALaDerecha ) pos++; 
						else pos--;
					strncpy(display, CADENA+pos, CANT_CARACTERES_DISPLAY);
					for ( i=0 ; i <= strlen(display)-1 ; i++) 
						transmitirCaracter(fd, status, reverse(display)[i]);
					if ( VERBOSE ) printf("%s\n", display);
					msleep(DESPLAZAMIENTO_DELAY);
				}
			break;
			case CICLICO:
				pos = 0;
				while ( TRUE )
				{
					if ( pos < 0 ) pos = strlen(CADENA)-1;
					if ( pos > strlen(CADENA) ) pos = 1;
					strncpy(display, CADENA+pos, CANT_CARACTERES_DISPLAY);
					strncpy(display+(strlen(CADENA)-pos), CADENA, CANT_CARACTERES_DISPLAY-strlen(display));
					for ( i=0 ; i <= strlen(display)-1 ; i++) 
						transmitirCaracter(fd, status, reverse(display)[i]);
					if ( VERBOSE ) printf("%s\n", display);
					msleep(DESPLAZAMIENTO_DELAY);
					if ( DESPLAZAMIENTO_SENTIDO = DERECHA )	pos--; else pos++;
				}
			break;
			case DESHABILITADO:
				strncpy(display, CADENA, CANT_CARACTERES_DISPLAY);
				for ( i=0 ; i <= strlen(display)-1 ; i++) {
					transmitirCaracter(fd, status, reverse(display)[i]);
					msleep(DESPLAZAMIENTO_DELAY);
				}
				if ( VERBOSE ) printf("%s\n", display);
			break;
			}
		}
		else //strlen(cadena) <= CANT_CARACTERES_DISPLAY
		{
			if ( ALINEACION == DERECHA )
			{
				while ( strlen(display) < CANT_CARACTERES_DISPLAY - strlen(CADENA) ) display = strcat(display, "-");
				strcat(display, CADENA);
			}
			else
			{
				strcat(display, CADENA);
				while ( strlen(display) < CANT_CARACTERES_DISPLAY ) display = strcat(display, "-");
			}
			//strcpy(CADENA, display);

			switch ( DESPLAZAMIENTO_TIPO )
			{
			case ALTERNANTE:
				while ( TRUE ) ////////////FALTA ESTE ALGORITMITO!!! NADA MAS!//////////////
				{
					if ( strlen(CADENA)-CANT_CARACTERES_DISPLAY == pos ) moviendoALaDerecha = TRUE;
					
					if (pos<=0)  moviendoALaDerecha = FALSE;
					if ( moviendoALaDerecha ) pos++; 
						else pos--;
					display = malloc(sizeof(char)*CANT_CARACTERES_DISPLAY);
					if ( pos >= 0 )	strncpy(display, CADENA+pos, CANT_CARACTERES_DISPLAY);
					else {for ( i=1 ; i <= 0-pos ; i++) strcat(display,  "-"); strcat(display, CADENA);}
					
					for ( i=0 ; i <= strlen(display)-1 ; i++) 
						transmitirCaracter(fd, status, reverse(display)[i]);
					if ( VERBOSE ) printf("pos:%i strlen(cadena):%i cadena:%s\n",pos, (int)strlen(CADENA),display);
					msleep(DESPLAZAMIENTO_DELAY);
				}
			break;
			case CICLICO:
				pos = 0;
				while ( TRUE )
				{
					if ( pos < 0 ) pos = strlen(CADENA)-1;
					if ( pos > strlen(CADENA) ) pos = 1;
					strncpy(display, CADENA+pos, CANT_CARACTERES_DISPLAY);
					strncpy(display+(strlen(CADENA)-pos), CADENA, CANT_CARACTERES_DISPLAY-strlen(display));
					for ( i=0 ; i <= strlen(display)-1 ; i++) 
						transmitirCaracter(fd, status, reverse(display)[i]);
					if ( VERBOSE ) printf("%s\n", display);
					msleep(DESPLAZAMIENTO_DELAY);
					if ( DESPLAZAMIENTO_SENTIDO = DERECHA )	pos--; else pos++;
				}
			break;
			case DESHABILITADO:
				strncpy(display, CADENA, CANT_CARACTERES_DISPLAY);
				for ( i=0 ; i <= strlen(display)-1 ; i++) {
					transmitirCaracter(fd, status, reverse(display)[i]);
					msleep(DESPLAZAMIENTO_DELAY);
				}
				if ( VERBOSE ) printf("%s\n", display);
			break;
			}
		}
	break;
	}
	
	// FIN ALGORITMOS DE MANEJO DE CADENAS


	close(fd);                    /* close the device file */

	exit(0);
}

