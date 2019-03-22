#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define KEY 2211
#define BUF_MAX 1024
#define LIMITE 26843545

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
  unsigned int pk;
} segmento_t;

void salir(int);
void producir(int key, orden_compra_t *);

segmento_t *shared_mem;
static volatile int flag_salir = 0;

int main(int argc, char *argv[]) {
  int modo_sleep = 0;
  if (argc > 1) {
    modo_sleep = strcmp(argv[1], "sleep") == 0;
  }

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

  if (modo_sleep) {
    printf("sleep de 1 segundo activado\n");
  } else {
    printf("sleep desactivado\n");
  }

  if (!(shared_mem->iniciado)) {
    printf("inicializando memoria compartida...\n");
    shared_mem->iniciado = 1;
    shared_mem->pk = 1;
    sem_init(&(shared_mem->lleno), 1, BUF_MAX);
    sem_init(&(shared_mem->vacio), 1, 0);
    sem_init(&(shared_mem->mutex_prod), 1, 1);
    sem_init(&(shared_mem->mutex_cons), 1, 1);
    sem_init(&(shared_mem->mutex_size), 1, 1);
  }

  signal(SIGINT, salir);

  // agregar una orden de compra cada segundo
  for (int agregadas = 0; agregadas < LIMITE; agregadas += 1) {
    sem_wait(&(shared_mem->lleno));

    // meter una nueva orden
    sem_wait(&(shared_mem->mutex_prod));
    int i = shared_mem->pos_prod;
    int key = shared_mem->pk;
    producir(key, &(shared_mem->buffer[i]));

    // siguiente posicion de productor
    shared_mem->pos_prod = (shared_mem->pos_prod + 1) % BUF_MAX;
    shared_mem->pk += 1;

    sem_post(&(shared_mem->mutex_prod));

    // cambiar size
    sem_wait(&(shared_mem->mutex_size));
    shared_mem->size += 1;
    sem_post(&(shared_mem->mutex_size));

    sem_post(&(shared_mem->vacio));

    if (flag_salir) {
      break;
    }

    if (modo_sleep) {
      sleep(1);
    }
  }

  return 0;
}

void producir(int key, orden_compra_t *orden) {
  printf("agregando orden con id %4d\n", key);
  orden->orden = key;
  orden->cliente = key * 45;
  orden->monto = key * 77.53;
  orden->tarjeta = key % 2;
  sprintf(orden->fecha, "%2d/%2d/20%2d", (key * 3) % 28 + 1, key % 12 + 1, key % 10 + 10);
}

void salir(int s) {
  flag_salir = 1;
}
