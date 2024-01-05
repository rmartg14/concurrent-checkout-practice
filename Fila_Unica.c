#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

int ClientesMax;
int CajerosMax;
struct Cliente{
	int estado;
	int id;
};
struct Cajero{
    int id;
};
struct Cliente *clientes;
struct Cajero *cajeros;
pthread_t cajero, reponedor, cliente;
pthread_mutex_t mutex_logs;
pthread_mutex_t mutex_ListaClientes;
pthread_mutex_t mutex_Reponedor;
pthread_cond_t reponedorAcceso;
FILE *logFile;
const char *logFileName = "./registroCaja.log";
int contadorClientes, numeroClientes, estadoReponedor;
void crearCliente(int sig);
int calculaNumRandom(int min, int max);
void *cajeroFuncion(void *arg);
void *reponedorFuncion(void *arg);
void *clienteFuncion(void *arg);
void writeLogMessage(char *id, char *msg);

int main(int argc, char const *argv[]){
	 struct sigaction ss;
	 ss.sa_handler=crearCliente;
	if (argv[1] <= 0){
		ClientesMax = 20;
	} else {
		ClientesMax = atoi(argv[1]);
	}
	
	if (argv[2] <= 0) {
		CajerosMax = 3;
	} else {
		CajerosMax = atoi(argv[2]);
	}
	
	if (-1 == sigaction(SIGUSR1, &ss, NULL)) {
	  	perror("ENTRADA DE CLIENTES: sigaction");
		exit(-1);
    }
	if (pthread_mutex_init(&mutex_logs, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&mutex_ListaClientes, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&mutex_Reponedor, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&reponedorAcceso, NULL) != 0){
		exit(-1);
	}
	numeroClientes=ClientesMax;
	clientes =(struct Cliente *)malloc(sizeof(struct Cliente) * numeroClientes);
	contadorClientes=0;
	for (int i = 0; i < CajerosMax; i++) {
	
		cajeros[i].id = i + 1;
		pthread_create(&cajero, NULL, cajeroFuncion, &cajeros[i].id);

	}
    	pthread_create(&reponedor, NULL, reponedorFuncion, NULL);
	
    
    	printf("Introduzca 'kill -10 %d' desde otro terminal para añadir un cliente \n",getpid());
	/* Esperar por señales de forma infinita*/
	while (1)
	{
		pause();
	}
    return 0;
}
void crearCliente(int sig){
	pthread_mutex_lock(&mutex_ListaClientes);
	if(contadorClientes>=ClientesMax){
		printf("NO se pueden introducir más clientes\n");
	}else{
		int i=0;
		int posVacia=0;
		while(i<ClientesMax&&posVacia==0){
			if(clientes[i].id==0){
				posVacia=1;
			}
		}
		clientes[i].estado=0;
		clientes[i].id=contadorClientes+1;
		pthread_create(&cliente, NULL, clienteFuncion, &clientes[i].id);
		contadorClientes++;
	}	
	pthread_mutex_unlock(&mutex_ListaClientes);
}void *cajeroFuncion(void *cajero_ID){
	int numAtenciones;
	int min_ID = CajerosMax+1;
	int id =*(int*) cajero_ID;
	char idClienteString[15];
	char idString[15];
	char precioString[10];
	int precio;
	char entryString[70];
	char exitString[130];
    while(1){
        while (contadorClientes == 0) {
            sleep(1);
        }
        pthread_mutex_lock(&mutex_ListaClientes);
        
        for (int i = 0; i < contadorClientes; i++) {
            if(clientes[i].id<min_ID && clientes[i].id != 0){
                min_ID = clientes[i].id;
            }
        }
        clientes[min_ID].estado = 1;
        
        
        pthread_mutex_unlock(&mutex_ListaClientes);
        
        int tiempo_atencion = calculaNumRandom(1,5);
		
        sprintf(idString, "cajero_%02d", id);
		sprintf(idClienteString, "%02d", min_ID);
		sprintf(entryString, "El cliente %s empieza a ser atendido a las: ", idClienteString);
		time_t hour=time(0);
       	strftime(entryString + strlen(entryString), sizeof(entryString) - strlen(entryString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
    	
		writeLogMessage(idString,entryString);
		
        sleep(tiempo_atencion);
        int res = calculaNumRandom(1,100);
        precio= calculaNumRandom(1,100);
        if (res <= 70) {
			
        	sprintf(precioString, "%02d", precio);
        	sprintf(exitString, "El cliente %s efectua su compra sin problemas gastando %s a las: ", idClienteString, precioString);
        	hour=time(0);
        	strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
			
			writeLogMessage(idString,exitString);
			
        } else if (res>= 71 && res <= 95) {
            pthread_mutex_lock(&mutex_Reponedor);
            estadoReponedor=1;
            pthread_cond_signal(&reponedorAcceso);
            while(estadoReponedor==1){
                pthread_cond_wait(&reponedorAcceso, &mutex_Reponedor);
            }
            pthread_mutex_unlock(&mutex_Reponedor);
            
			sprintf(precioString, "%02d", precio);
            sprintf(exitString, "El cliente %s efectua su compra tras esperar al reponedor gastando %s a las: ", idClienteString, precioString);
            hour=time(0);
            strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
            
			writeLogMessage(idString,exitString);
			
        } else if (res >= 96) {
            switch (res) {
            case 96:
			
               	sprintf(exitString, "El cliente %s no tenía dinero y no ha podido realizar la compra y se va a las: ", idClienteString);
       		hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		
			writeLogMessage(idString,exitString);
			
                break;
            case 97:
			
                sprintf(exitString, "Al cliente %s no le funciona la tarjeta y no ha podido realizar la compra y se va a las: ", idClienteString);
            	hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		
			writeLogMessage(idString,exitString);
			
                break;
            case 98:
			
                sprintf(exitString, "El cliente %s ha tenido una urgencia medica y no ha podido realizar la compra y se va a las: ", idClienteString);
                hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		
			writeLogMessage(idString,exitString);
			
                break;
            case 99:
			
                sprintf(exitString, "El cliente %s era un fugitivo y ha sido arrestado por la policía no realizando su compra a las: ", idClienteString);
		hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		
			writeLogMessage(idString,exitString);
			
                break;
            default:
			
                sprintf(exitString, "El cliente %s ha sido sorprendido robando choped y se le ha echado de la tiendasin realizar su compra a las: ", idClienteString);
		hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		
			writeLogMessage(idString,exitString);
			
            }
            
        }
        pthread_mutex_lock(&mutex_ListaClientes);
        clientes[min_ID].estado = 2;
        pthread_mutex_unlock(&mutex_ListaClientes);
		min_ID = CajerosMax+1;
        numAtenciones ++;
        if (numAtenciones == 10) {
            numAtenciones = 0;
            sleep(20);
        }
    }    
}
void *reponedorFuncion(void *arg){
    pthread_mutex_lock(&mutex_Reponedor);
    while (estadoReponedor==0){
        pthread_cond_wait(&reponedorAcceso, &mutex_Reponedor);
    }
    int tiempo_trabajo = calculaNumRandom(1,5);
    sleep(tiempo_trabajo);
    estadoReponedor=0;
    pthread_cond_signal(&reponedorAcceso);
    pthread_mutex_unlock(&mutex_Reponedor);
}


void *clienteFuncion(void *clienteID){
	int tiempoEspera;
	int id=*(int*)clienteID;
	char idString[15];
	char entryString[52];
	char exitString[65];
	
	time_t hour=time(0);
	sprintf(idString, "cliente_%02d", id);
	strftime(entryString, sizeof(entryString), "La hora de entrada es: %Y-%m-%d %H:%M:%S", localtime(&hour));
    	//El problema creo que esta en esto del strftime
		writeLogMessage(idString,exitString);
		
	sleep(10);
	tiempoEspera=calculaNumRandom(1,100);
	if(tiempoEspera<90){
		while(clientes[id-1].estado!=2){
			sleep(1);
			
		}
		
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente termina de ser atendido a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
    		
			writeLogMessage(idString,exitString);
			
		
	}else{
		
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente no es atendido y se va a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
    		
			writeLogMessage(idString,exitString);
			
	}
	pthread_mutex_lock(&mutex_ListaClientes);
	clientes[contadorClientes].estado=0;
	clientes[contadorClientes].id=0;
	contadorClientes--;
	pthread_mutex_unlock(&mutex_ListaClientes);

}
int calculaNumRandom(int min, int max) {
    //calcula un numero random entre el minimo y el maximo
    srand(getpid());
    return rand() % (max-min+1) +min;
}
void writeLogMessage(char *id, char *msg) {
    // Calculamos la hora actual
    time_t now = time(0);
    struct tm *tlocal = localtime(&now);
    char stnow[25];
    strftime(stnow, 25, " %d/ %m/ %y %H: %M: %S ", tlocal);

    // Escribimos en el log
    logFile = fopen(logFileName, "a");
    fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
    fclose(logFile);
}
