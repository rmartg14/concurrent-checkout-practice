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

int clientesMax;
int cajerosMax ;
struct Cliente{
	pthread_t clienteHilo;
	int estado;
	int id;

};
struct Cajero{
    pthread_t cajeroHilo;
    int id;
    int estado;
};
struct Cliente *clientes;
struct Cajero *cajeros;
pthread_t reponedorHilo;
pthread_mutex_t mutex_logs;
pthread_mutex_t mutex_ListaClientes;
pthread_mutex_t mutex_ListaCajeros;
pthread_mutex_t mutex_Reponedor;
pthread_cond_t reponedorAcceso;
pthread_cond_t clienteAtendido;
pthread_cond_t clienteCreado;
pthread_cond_t cajeroOcupado;
FILE *logFile;
const char *logFileName = "./registroCaja2.log";
int contadorClientes, numeroClientes, numeroCajeros, estadoReponedor, estadoHilo;
int totalClientes,clientesAtendidos;
void crearCliente(int sig);
int calculaNumRandom(int min, int max);
void *cajeroFuncion(void *arg);
void *reponedorFuncion(void *arg);
void *clienteFuncion(void *arg);
void writeLogMessage(char *id, char *msg);

int main(int argc, char const *argv[]){
	srand(getpid());
	 if(argc==3){
	 	if (atoi(argv[1]) <= 0){
		clientesMax = 20;
		} else {
			clientesMax = atoi(argv[1]);
		}
	
		if (atoi(argv[2]) <= 0) {
			cajerosMax = 3;
		} else {
			cajerosMax = atoi(argv[2]);
		}
	 }else{
	 	clientesMax = 20;
	 	cajerosMax = 3;
	 
	 }
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
	if (pthread_mutex_init(&mutex_ListaCajeros, NULL) != 0){
		exit(-1);
	}
	if (pthread_mutex_init(&mutex_Reponedor, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&clienteCreado, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&clienteAtendido, NULL) != 0){
		exit(-1);
	}
	
	if (pthread_cond_init(&cajeroOcupado, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&reponedorAcceso, NULL) != 0){
		exit(-1);
	}
	

	numeroClientes=clientesMax;
	numeroCajeros=cajerosMax;
	clientes =(struct Cliente *)malloc(sizeof(struct Cliente) * numeroClientes);
	cajeros =(struct Cajero *)malloc(sizeof(struct Cajero) * numeroCajeros);
	contadorClientes=0;
	for (int i = 0; i < cajerosMax; i++) {
	
		cajeros[i].id = i + 1;
		pthread_create(&cajeros[i].cajeroHilo, NULL, cajeroFuncion, &cajeros[i].id);

	}
    	pthread_create(&reponedorHilo, NULL, reponedorFuncion, NULL);
	
    	
    	printf("Introduzca 'kill -10 %d' desde otro terminal para añadir un cliente \n",getpid());
	/* Esperar por señales de forma infinita*/
	
	while (1)
	{
		sleep(1);
		pause();
		
	}
    return 0;
}
void crearCliente(int sig){
	pthread_mutex_lock(&mutex_ListaClientes);
	if(contadorClientes>=clientesMax){
		printf("NO se pueden introducir más clientes\n");
	}else{
		clientes[contadorClientes].estado=0;
		clientes[contadorClientes].id=contadorClientes+1;
		pthread_create(&clientes[contadorClientes].clienteHilo, NULL, clienteFuncion, &clientes[contadorClientes].id);
		printf("creado hilo %d\n",contadorClientes+1);
		contadorClientes++;
		totalClientes++;
	}	
	pthread_mutex_unlock(&mutex_ListaClientes);
}
void *cajeroFuncion(void *cajero_ID){
	sleep(1);
	int numAtenciones;
	int idImprimir;
	int min_ID = 21;
	int id=*(int*)cajero_ID;
	char idClienteString[15];
	char idString[15];
	char precioString[10];
	int precio;
	char entryString[70];
	char exitString[130];
	
    while(1){
    	min_ID = 50;
    	pthread_mutex_lock(&mutex_ListaCajeros);
        while (cajeros[id-1].estado == 0) {
              pthread_cond_wait(&clienteCreado, &mutex_ListaCajeros);
        }
        pthread_mutex_unlock(&mutex_ListaCajeros);
        pthread_mutex_lock(&mutex_ListaClientes);
        for (int i = 0; i < contadorClientes; i++) {
            if(clientes[i].id<min_ID&&clientes[i].estado==0&&clientes[i].id>0){
                min_ID = clientes[i].id;
            }
          
          
        }
        printf("soy el cajero %d y he encontrado el id %d\n",id,min_ID);
        if(clientes[min_ID-1].estado==0&&clientes[min_ID-1].id==min_ID){
         printf("soy %d coincido con %d y mi estado es %d\n",clientes[min_ID-1].id,min_ID,clientes[min_ID-1].estado);
        idImprimir=clientesAtendidos+min_ID;
        clientes[min_ID-1].estado = 1;
     	pthread_cond_signal(&cajeroOcupado);
        pthread_mutex_unlock(&mutex_ListaClientes);
        
        int tiempo_atencion = calculaNumRandom(1,5);
        sprintf(idString, "cajero_%02d", id);
	sprintf(idClienteString, "%02d", idImprimir);
	sprintf(entryString, "El cliente %s empieza a ser atendido a las: ", idClienteString);
	time_t hour=time(0);
       	strftime(entryString + strlen(entryString), sizeof(entryString) - strlen(entryString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       	pthread_mutex_lock(&mutex_logs);
    	writeLogMessage(idString,entryString);
    	pthread_mutex_unlock(&mutex_logs);
        sleep(tiempo_atencion);
        int res = calculaNumRandom(1,100);
        precio= calculaNumRandom(1,100);
        if (res <= 70) {
            	sprintf(precioString, "%02d", precio);
        	sprintf(exitString, "El cliente %s efectua su compra sin problemas gastando %s a las: ", idClienteString, precioString);
        	hour=time(0);
        	strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
        	pthread_mutex_lock(&mutex_logs);
        	writeLogMessage(idString,exitString);
        	pthread_mutex_unlock(&mutex_logs);
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
            pthread_mutex_lock(&mutex_logs);
            writeLogMessage(idString,exitString);
            pthread_mutex_unlock(&mutex_logs);
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
       		pthread_mutex_lock(&mutex_logs);
       		writeLogMessage(idString,exitString);
       		pthread_mutex_unlock(&mutex_logs);
                break;
            case 98:
                sprintf(exitString, "El cliente %s ha tenido una urgencia medica y no ha podido realizar la compra y se va a las: ", idClienteString);
                hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		pthread_mutex_lock(&mutex_logs);
       		writeLogMessage(idString,exitString);
       		pthread_mutex_unlock(&mutex_logs);
                break;
            case 99:
                sprintf(exitString, "El cliente %s era un fugitivo y ha sido arrestado por la policía no realizando su compra a las: ", idClienteString);
		hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		pthread_mutex_lock(&mutex_logs);
       		writeLogMessage(idString,exitString);
       		pthread_mutex_unlock(&mutex_logs);
                break;
            default:
                sprintf(exitString, "El cliente %s ha sido sorprendido robando choped y se le ha echado de la tiendasin realizar su compra a las: ", idClienteString);
		hour=time(0);
       		strftime(exitString + strlen(exitString), sizeof(exitString) - strlen(exitString), "%Y-%m-%d %H:%M:%S", localtime(&hour));
       		pthread_mutex_lock(&mutex_logs);
       		writeLogMessage(idString,exitString);
       		pthread_mutex_unlock(&mutex_logs);
            }
            
        }
        printf("Soy el cliente %d y TERMINE DE comprar\n",min_ID);
        
        
        pthread_mutex_lock(&mutex_ListaClientes);
        clientes[min_ID-1].estado = 2;
        pthread_cond_signal(&clienteAtendido);	
        pthread_mutex_unlock(&mutex_ListaClientes);
        pthread_mutex_lock(&mutex_ListaCajeros);
        while (cajeros[id-1].estado == 1) {
              pthread_cond_wait(&clienteCreado, &mutex_ListaCajeros);
        }
        pthread_mutex_unlock(&mutex_ListaCajeros);
        numAtenciones ++;
        /*
        if (numAtenciones == 10) {
            numAtenciones = 0;
            sleep(20);
        }
        */
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
	int idImprimir;
	char entryString[52];
	char exitString[65];
	time_t hour=time(0);
	pthread_mutex_lock(&mutex_ListaClientes);
	idImprimir=clientesAtendidos+id;
	pthread_mutex_unlock(&mutex_ListaClientes);
	sprintf(idString, "cliente_%02d", idImprimir);
	strftime(entryString, sizeof(entryString), "La hora de entrada es: %Y-%m-%d %H:%M:%S", localtime(&hour));
	pthread_mutex_lock(&mutex_logs);
    	writeLogMessage(idString, entryString);
    	pthread_mutex_unlock(&mutex_logs);
	sleep(10);
	tiempoEspera=calculaNumRandom(1,100);
	if(tiempoEspera<90){
		pthread_mutex_lock(&mutex_ListaCajeros);
		int i=0;
		int cajeroEncontrado=0;
		while(i<cajerosMax&&cajeroEncontrado==0){
			if(cajeros[i].estado==0){
				cajeros[i].estado=1;
				cajeroEncontrado=1;
				pthread_cond_signal(&clienteCreado);
			}else{
				i++;
			}
		}
			
		pthread_mutex_unlock(&mutex_ListaCajeros);
		pthread_mutex_lock(&mutex_ListaClientes);
		printf("llego hasta aqui %d\n",id);
		
		while(clientes[id-1].estado==0){
			pthread_cond_wait(&cajeroOcupado, &mutex_ListaClientes);
			
		}
		
		printf("Soy atendido %d\n",id);
		
		while(clientes[id-1].estado!=2){
			pthread_cond_wait(&clienteAtendido, &mutex_ListaClientes);
			
		}
		printf("HE TERMINADO DE SER atendido %d\n",id);
		pthread_mutex_unlock(&mutex_ListaClientes);
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente termina de ser atendido a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
		pthread_mutex_lock(&mutex_logs);
    		writeLogMessage(idString, exitString);
    		pthread_mutex_unlock(&mutex_logs);
		pthread_mutex_lock(&mutex_ListaCajeros);
		cajeros[i].estado=0;
		pthread_cond_signal(&clienteCreado);
		pthread_mutex_lock(&mutex_ListaCajeros);
	}else{
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente no es atendido y se va a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
		pthread_mutex_lock(&mutex_logs);
    		writeLogMessage(idString, exitString);
    		pthread_mutex_unlock(&mutex_logs);
    		
	}
	pthread_mutex_lock(&mutex_ListaClientes);
	estadoHilo=0;
	int i=id-1;
	while(i<contadorClientes){
		clientes[i].estado=clientes[i+1].estado;
		clientes[i].id=clientes[i+1].id;
		i++;
	}
	contadorClientes--;
	clientesAtendidos++;
	pthread_mutex_unlock(&mutex_ListaClientes);

}
int calculaNumRandom(int min, int max) {
    //calcula un numero random entre el minimo y el maximo
    
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
