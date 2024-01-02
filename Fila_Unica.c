#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define ClientesMax 20 
#define Cajeros 3

void cajeroFuncion(void arg);
void reponedorFuncion(void arg);
void clienteFuncion(void arg);
//manejador señales (FALTA)

int main(int argc, char const *argv[]){
    //implmentacion manejador de señales

    pthread_t cajeros, reponedor, clientes;
    //pthread_create();
    return 0;
}
void cajeroFuncion(void arg);
void reponedorFuncion(void arg);
void clienteFuncion(void arg);
