#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define KEY 5504
#define BUF_MAX 1024

typedef struct orden_compra_t {
  unsigned int orden;
  unsigned int cliente;
  int tarjeta; // 1 => tarjeta, 0 => efectivo
  char fecha[11]; // dd/mm/yyyy
  double monto;
} orden_compra_t;

typedef struct segmento_t {
  orden_compra_t buffer[BUF_MAX];
  sem_t lleno;
  sem_t vacio;
  int iniciado;
  int pos_prod;
  int pos_cons;
  sem_t mutex_prod;
  sem_t mutex_cons;
  sem_t mutex_size;
  int size;
} segmento_t;

void consumir(orden_compra_t *);

segmento_t *shared_mem;

int main(int argc, char *argv[]) {
  key_t key = KEY;
  int shmid = shmget(key, sizeof(segmento_t), 0644 | IPC_CREAT);

  if (shmid == -1) {
    perror("shmget");
    return 1;
  }

  shared_mem = (segmento_t *) shmat(shmid, NULL, 0);

  if (shared_mem == (segmento_t *)(-1)) {
    perror("shmat");
    return 1;
  }

  if (!(shared_mem->iniciado)) {
    printf("inicializando memoria compartida...\n");
    shared_mem->iniciado = 1;
    sem_init(&(shared_mem->lleno), 1, BUF_MAX);
    sem_init(&(shared_mem->vacio), 1, 0);
    sem_init(&(shared_mem->mutex_prod), 1, 1);
    sem_init(&(shared_mem->mutex_cons), 1, 1);
    sem_init(&(shared_mem->mutex_size), 1, 1);
  }

  // leer 5 ordenes de compra cada segundo, hasta 5 ordenes
  for (int leidas = 0; leidas < 5; leidas += 1) {
    printf("consumiendo orden #%d\n", leidas);
    sem_wait(&(shared_mem->vacio));

    // leer una nueva orden
    sem_wait(&(shared_mem->mutex_cons));
    orden_compra_t *orden_temp = &shared_mem->buffer[shared_mem->pos_cons];

    // siguiente posicion de consumidor
    shared_mem->pos_cons = (shared_mem->pos_cons + 1) % BUF_MAX;

    sem_post(&(shared_mem->mutex_cons));

    // cambiar size
    // sem_wait(&(shared_mem->mutex_size));
    // shared_mem->size -= 1;
    // sem_post(&(shared_mem->mutex_size));

    sem_post(&(shared_mem->lleno));

    consumir(orden_temp);

    sleep(1);
  }

  return 0;
}

void consumir(orden_compra_t *orden) {
  printf("orden: %d\n", orden->orden);
  printf("cliente: %d\n", orden->cliente);
  printf("monto: %f\n", orden->monto);
  printf("tarjeta: %d\n", orden->tarjeta);
  printf("fecha: %s\n", orden->fecha);
  printf("\n");
}
