#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define ClientesMax 20 
#define Cajeros 3
struct Cliente{
	int estado;
	int id;

};
struct Cliente *clientes;
pthread_t cajeros, reponedor, cliente;
pthread_mutex_t mutex_logs;
pthread_mutex_t mutex_ListaClientes;
pthread_mutex_t mutex_Reponedor;
pthread_cond_t reponedorAcceso;
FILE *logFile;
const char *logFileName = "./registroCaja.log";
int contadorClientes, numeroClientes;
void crearCliente(int sig);
int calculaNumRandom(int min, int max);
void *cajeroFuncion(void *arg);
void *reponedorFuncion(void *arg);
void *clienteFuncion(void *arg);
//manejador señales (FALTA)

int main(int argc, char const *argv[]){
	 struct sigaction ss;
	 ss.sa_handler=crearCliente;
	 
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
    	pthread_create(&cajeros, NULL, cajeroFuncion, NULL);
    	pthread_create(&cajeros, NULL, cajeroFuncion, NULL);
    	pthread_create(&cajeros, NULL, cajeroFuncion, NULL);
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
}
void *cajeroFuncion(void *arg);
void *reponedorFuncion(void *arg);
void *clienteFuncion(void *clienteID){
	int tiempoEspera;
	int id=*(int*)clienteID;
	char idString[15];
	char entryString[52];
	char exitString[65];
	time_t hour=time(0);
	sprintf(idString, "cliente_%02d", id);
	strftime(entryString, sizeof(entryString), "La hora de entrada es: %Y-%m-%d %H:%M:%S", localtime(&hour));
    	writeLogMessage(idString, entryString);
	sleep(10);
	tiempoEspera=calculaNumRandom(1,100);
	if(tiempoEspera<90){
		while(clientes[id-1].estado!=2){
			pause();
			
		}
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente termina de ser atendido a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
    		writeLogMessage(idString, exitString);
		
	}else{
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente no es atendido y se va a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
    		writeLogMessage(idString, exitString);
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
