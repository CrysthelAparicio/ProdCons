#include <sys/shm.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUF_MAX 1024

typedef struct orden_compra_t {
  unsigned int orden;
  unsigned int cliente;
  int tarjeta; // 1 => tarjeta, 0 => efectivo
  char fecha[11]; // dd/mm/yyyy
  double monto;
} orden_compra_t;

typedef struct segmento_t {
  orden_compra_t buffer[1024];
} segmento_t;

segmento_t *shared_mem;

int main(int argc, char *argv[]) {
  key_t key = 5646;
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

  // agregar 5 ordenes de compra cada segundo, hasta 100 ordenes
  for (int agregadas = 0; agregadas < 100; agregadas += 1) {
    sleep(1);
  }

  for (int i = 0; i < BUF_MAX; i += 1) {
    shared_mem->buffer[i].orden = i * 2;
    shared_mem->buffer[i].cliente = i * 45;
    shared_mem->buffer[i].monto = 77.53 * i;
    shared_mem->buffer[i].tarjeta = i % 2;
    strcpy(shared_mem->buffer[i].fecha, "14/11/1990");
  }

  return 0;
}
