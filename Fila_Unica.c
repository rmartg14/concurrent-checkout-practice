#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define ClientesMax 20 
#define Cajeros 3
pthread_t cajeros, reponedor, clientes;
pthread_mutex_t mutex_logs;
pthread_mutex_t mutex_ListaClientes;
pthread_mutex_t mutex_Reponedor;
pthread_cond_t reponedorAcceso;
FILE *logFile;
const char *logFileName = "./archivo.log";
void cajeroFuncion(void arg);
void reponedorFuncion(void arg);
void clienteFuncion(void arg);
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

void *cajeroFuncion(void *arg);
void *reponedorFuncion(void *arg);
void *clienteFuncion(void *arg);
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
