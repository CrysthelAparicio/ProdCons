#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
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
} segmento_t;

void salir(int);
void consumir(orden_compra_t *);

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
    sem_init(&(shared_mem->lleno), 1, BUF_MAX);
    sem_init(&(shared_mem->vacio), 1, 0);
    sem_init(&(shared_mem->mutex_prod), 1, 1);
    sem_init(&(shared_mem->mutex_cons), 1, 1);
    sem_init(&(shared_mem->mutex_size), 1, 1);
  }

  signal(SIGINT, salir);

  // consumir una orden de compra cada segundo
  for (int leidas = 0; leidas < LIMITE; leidas += 1) {
    sem_wait(&(shared_mem->vacio));

    // leer una nueva orden
    sem_wait(&(shared_mem->mutex_cons));
    orden_compra_t *orden_temp = malloc(sizeof(*orden_temp));
    *orden_temp = shared_mem->buffer[shared_mem->pos_cons];

    // siguiente posicion de consumidor
    shared_mem->pos_cons = (shared_mem->pos_cons + 1) % BUF_MAX;

    sem_post(&(shared_mem->mutex_cons));

    // cambiar size
    sem_wait(&(shared_mem->mutex_size));
    shared_mem->size -= 1;
    printf("restantes: %12d\n", shared_mem->size);
    sem_post(&(shared_mem->mutex_size));

    sem_post(&(shared_mem->lleno));

    consumir(orden_temp);
    free(orden_temp);

    if (flag_salir) {
      break;
    }

    if (modo_sleep) {
      sleep(1);
    }
  }

  return 0;
}

void consumir(orden_compra_t *orden) {
  char *tarjeta;
  if (orden->tarjeta) {
    tarjeta = "tarjeta\0";
  } else {
    tarjeta = "efectivo\0";
  }
  printf("# orden:   %12d\n", orden->orden);
  printf("# cliente: %12d\n", orden->cliente);
  printf("monto:     %12.2f\n", orden->monto);
  printf("tipo de pago: %9s\n", tarjeta);
  printf("fecha:     %12s\n", orden->fecha);
  printf("\n");
}

void salir(int s) {
  flag_salir = 1;
}
