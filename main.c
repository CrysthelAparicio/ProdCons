#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#define BUFF_SIZE 4
#define FULL 0
#define EMPTY 0
char buffer[BUFF_SIZE];
int nextIn = 0;
int nextOut = 0;

sem_t empty_sem_mutex; //semaforo del productor
sem_t full_sem_mutex; //semaforo del consumidor

void Poner(char item){
    int value;
    sem_wait(&empty_sem_mutex); //obtiene el mutex para llenar el buffer

    buffer[nextIn] = item;
    nextIn = (nextIn + 1) % BUFF_SIZE;
    printf("Produciendo %c...nextIn %d...Elemento=%d\n",item,nextIn,item);
    if(nextIn == FULL){
        sem_post(&full_sem_mutex);
        sleep(1);
    }
    sem_post(&empty_sem_mutex);
}

void * Productor(){
    for(int i = 0; i < 10; i++){
        Poner((char)('A'+ i % 26));
    }
}

void Get(){
    int item;
    sem_wait(&full_sem_mutex); //ganar el mutex para consumir desde el buffer

    item = buffer[nextOut];
    nextOut = (nextOut + 1) % BUFF_SIZE;
    printf("Consumiendo %c...nextOut %d...Elemento=%d\n",item,nextOut,item);
    if(nextOut==EMPTY){ //esta vacio
      sleep(1);
    }

  sem_post(&full_sem_mutex);
}

void * Consumidor(){
    for(int i=0; i<10; i++){
        Get();
    }
}

int main(){
    pthread_t ptid, ctid;
    //Inicializa los semaforos

    sem_init(&empty_sem_mutex,0,1);
    sem_init(&full_sem_mutex,0,0);

    //creando los hilos de produtor y consumidor
    if(pthread_create(&ptid,NULL,Productor,NULL)){
        printf("\n Error creando el hilo 1");
        exit(1);
    }

    if(pthread_create(&ctid,NULL,Consumidor,NULL)){
        printf("\n Error creando el hilo 2");
        exit(1);
    }

    if(pthread_join(ptid,NULL)){ //esperando que el productor termine
        printf("\n Error entrando al hilo");
        exit(1);
    }

    if(pthread_join(ctid,NULL)){ //esperando que el consumidor termine
        printf("\n Error entrando al hilo");
        exit(1);
    }

    sem_destroy(&empty_sem_mutex);
    sem_destroy(&full_sem_mutex);

    pthread_exit(NULL);
    return 1;
}