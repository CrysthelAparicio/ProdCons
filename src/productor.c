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

void producir(int key, orden_compra_t *);

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

  // agregar una orden de compra cada segundo, hasta 5 ordenes
  for (int agregadas = 0; agregadas < 5; agregadas += 1) {
    printf("agregando orden #%d\n", agregadas);
    sem_wait(&(shared_mem->lleno));

    // meter una nueva orden
    sem_wait(&(shared_mem->mutex_prod));
    int i = shared_mem->pos_prod;
    producir(i + 1, &(shared_mem->buffer[i]));

    // siguiente posicion de productor
    shared_mem->pos_prod = (shared_mem->pos_prod + 1) % BUF_MAX;

    sem_post(&(shared_mem->mutex_prod));

    // cambiar size
    sem_wait(&(shared_mem->mutex_size));
    shared_mem->size += 1;
    sem_post(&(shared_mem->mutex_size));

    sem_post(&(shared_mem->vacio));

    sleep(1);
  }

  return 0;
}

void producir(int key, orden_compra_t *orden) {
  orden->orden = key * 2;
  orden->cliente = key * 45;
  orden->monto = 77.53 * key;
  orden->tarjeta = key % 2;
  strcpy(orden->fecha, "14/11/1990");
}
