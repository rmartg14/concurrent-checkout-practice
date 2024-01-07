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
	int posicion;

};
struct Cajero{
    pthread_t cajeroHilo;
    int id;
    int atendidos;
};
struct Cliente *clientes;
struct Cajero *cajeros;
pthread_t reponedorHilo;
pthread_mutex_t mutex_logs;
pthread_mutex_t mutex_ListaClientes;
pthread_mutex_t mutex_Reponedor;
pthread_cond_t reponedorAcceso;
pthread_cond_t clienteAtendido;
pthread_cond_t clienteAtendido2;
pthread_cond_t clienteCreado;
pthread_cond_t cajeroOcupado;
FILE *logFile;
const char *logFileName = "./registroCaja2.log";
int contadorClientes, numeroClientes, numeroCajeros, estadoReponedor, estadoHilo, cajerosDisponibles;
int totalClientes,clientesAtendidos;
void crearCliente(int sig);
void finPrograma(int sig);
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
	 struct sigaction fin;
	 fin.sa_handler=finPrograma;
	
	if (-1 == sigaction(SIGUSR1, &ss, NULL)) {
	  	perror("ENTRADA DE CLIENTES: sigaction");
		exit(-1);
  	 }
    	if (sigaction(SIGINT, &fin, NULL) == -1){
		perror("Finaliza el programa: sigaction");
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
	if (pthread_cond_init(&clienteCreado, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&clienteAtendido, NULL) != 0){
		exit(-1);
	}
	if (pthread_cond_init(&clienteAtendido2, NULL) != 0){
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
	printf("Introduzca 'kill -2 %d' desde otro terminal para FINALIZAR el programa.\n",getpid());
	
	while (1)
	{
		
		pause();
		
	}
    return 0;
}
void crearCliente(int sig){
	pthread_mutex_lock(&mutex_ListaClientes);
	if(contadorClientes>=clientesMax){
		printf("NO se pueden introducir más clientes\n");
	}else{
		int i=0;
		int posEnc=0;
		while(i<clientesMax&&posEnc==0){
			if(clientes[i].id==0){
				posEnc=1;
			}else{
				i++;
			}
		}
		clientes[i].estado=0;
		clientes[i].id=totalClientes+1;
		clientes[i].posicion=i;
		pthread_create(&clientes[i].clienteHilo, NULL, clienteFuncion, &clientes[i].posicion);
		printf("creado hilo %d con posicion= %d\n",totalClientes+1, i);
		contadorClientes++;
		totalClientes++;
	}	
	pthread_mutex_unlock(&mutex_ListaClientes);
}
void *cajeroFuncion(void *cajero_ID){
	sleep(1);
	int numAtenciones;
	int idImprimir;
	int min_ID = 51;
	int pos=50;
	int id=*(int*)cajero_ID;
	char idClienteString[15];
	char idString[15];
	char precioString[10];
	int precio;
	char entryString[70];
	char exitString[130];
    while(1){
    	min_ID = 50;
    	pthread_mutex_lock(&mutex_ListaClientes);
        while (estadoHilo == 0) {
              pthread_cond_wait(&clienteCreado, &mutex_ListaClientes);
        }
        cajerosDisponibles++;
        for (int i = 0; i < clientesMax; i++) {
            if(clientes[i].id<min_ID&&clientes[i].estado==0&&clientes[i].id>0){
                min_ID = clientes[i].id;
                pos=clientes[i].posicion;
            }
          
          
        }
        printf("soy el cajero %d y he encontrado el id %d con pos= %d\n",id,min_ID,pos);
        if(clientes[pos].estado==0&&clientes[pos].id==min_ID){
         printf("soy la pos %d id %d y mi estado es %d\n",clientes[pos].posicion,min_ID,clientes[pos].estado);
        idImprimir=min_ID;
        clientes[pos].estado = 1;
         printf("soy la pos %d id %d y mi estado ha cambiado a %d\n",clientes[pos].posicion,min_ID,clientes[pos].estado);
     	pthread_cond_broadcast(&cajeroOcupado);
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
        
        pthread_mutex_lock(&mutex_ListaClientes);
        printf("Soy el cliente %d y TERMINE DE comprar y mi pos es %d\n",min_ID,pos);
        clientes[pos].estado = 2;
        printf("Soy el cliente %d mi pos es %d y mi estado cambia a %d \n",min_ID,pos,clientes[pos].estado);
        pthread_cond_broadcast(&clienteAtendido);
        while(clientes[pos].estado!=0){
		pthread_cond_wait(&clienteAtendido2, &mutex_ListaClientes);		
	}
	cajeros[id-1].atendidos++;
	numAtenciones ++;
	cajerosDisponibles--;
        pthread_mutex_unlock(&mutex_ListaClientes);
        
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
    
    while(1){
    	pthread_mutex_lock(&mutex_Reponedor);
    	while (estadoReponedor==0){
        	pthread_cond_wait(&reponedorAcceso, &mutex_Reponedor);
    	}
    	int tiempo_trabajo = calculaNumRandom(1,5);
    	sleep(tiempo_trabajo);
    	estadoReponedor=0;
    	pthread_cond_broadcast(&reponedorAcceso);
    	 pthread_mutex_unlock(&mutex_Reponedor);
    }
   
}


void *clienteFuncion(void *clientePos){
	int tiempoEspera;
	int pos=*(int*)clientePos;
	char idString[15];
	int idImprimir;
	char entryString[52];
	char exitString[65];
	time_t hour=time(0);
	pthread_mutex_lock(&mutex_ListaClientes);
	int id=clientes[pos].id;
	idImprimir=id;
	printf("Hay estoscajeros %d para %d\n",cajerosDisponibles,id);
	
	
	sprintf(idString, "cliente_%02d", idImprimir);
	strftime(entryString, sizeof(entryString), "La hora de entrada es: %Y-%m-%d %H:%M:%S", localtime(&hour));
	pthread_mutex_lock(&mutex_logs);
    	writeLogMessage(idString, entryString);
    	pthread_mutex_unlock(&mutex_logs);
    	if(contadorClientes>3){
    		pthread_mutex_unlock(&mutex_ListaClientes);
    		tiempoEspera=calculaNumRandom(1,100);
		sleep(10);
		tiempoEspera=calculaNumRandom(1,100);
		pthread_mutex_lock(&mutex_ListaClientes);
		if(tiempoEspera<90){
    			while(cajerosDisponibles==3){
				pthread_mutex_unlock(&mutex_ListaClientes);
				sleep(1);
			}
		}
	}
    	pthread_mutex_unlock(&mutex_ListaClientes);
	if(tiempoEspera<90){
		pthread_mutex_lock(&mutex_ListaClientes);
		estadoHilo=1;
		pthread_cond_signal(&clienteCreado);
		printf("llego hasta aqui %d y mi pos es %d\n",id,pos);
		
		while(clientes[pos].estado==0){
			pthread_cond_wait(&cajeroOcupado, &mutex_ListaClientes);
			
		}
		
		printf("Soy atendido %d y mi pos es %d\n",id, pos);
		pthread_mutex_unlock(&mutex_ListaClientes);
		pthread_mutex_lock(&mutex_ListaClientes);
		while(clientes[pos].estado!=2){
			pthread_cond_wait(&clienteAtendido, &mutex_ListaClientes);
			
		}
		printf("HE TERMINADO DE SER atendido %d y mi pos es %d\n",id,pos);
		pthread_mutex_unlock(&mutex_ListaClientes);
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente termina de ser atendido a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
		pthread_mutex_lock(&mutex_logs);
    		writeLogMessage(idString, exitString);
    		pthread_mutex_unlock(&mutex_logs);
		
	}else{
		time_t hour=time(0);
		strftime(exitString, sizeof(exitString), "El cliente no es atendido y se va a las: %Y-%m-%d %H:%M:%S", localtime(&hour));
		pthread_mutex_lock(&mutex_logs);
    		writeLogMessage(idString, exitString);
    		pthread_mutex_unlock(&mutex_logs);
	}
	pthread_mutex_lock(&mutex_ListaClientes);
	estadoHilo=0;
	clientes[pos].estado=0;
	clientes[pos].id=0;
	pthread_cond_signal(&clienteAtendido2);
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
void finPrograma(int sig){
	for(int i=0;i<cajerosMax;i++){
		char cajeroN[10];
		char mensajeN[40];
		sprintf(cajeroN, "cajero_%02d", i+1);
		sprintf(mensajeN, "He atendido a %d clientes", cajeros[i].atendidos);
		pthread_mutex_lock(&mutex_logs);
    		writeLogMessage(cajeroN, mensajeN);
		pthread_mutex_unlock(&mutex_logs);
	}	

	free(cajeros);
	free(clientes);
	printf("Finalizando\n");
	exit(0);
}
