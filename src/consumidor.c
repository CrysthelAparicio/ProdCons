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
  int shmid = shmget(key, sizeof(segmento_t), 0644);

  if (shmid == -1) {
    perror("shmget");
    return 1;
  }

  shared_mem = (segmento_t *) shmat(shmid, NULL, 0);

  if (shared_mem == (segmento_t *)(-1)) {
    perror("shmat");
    return 1;
  }

  printf("orden: %d\n", shared_mem->buffer[1023].orden);
  printf("cliente: %d\n", shared_mem->buffer[1023].cliente);
  printf("monto: %f\n", shared_mem->buffer[1023].monto);
  printf("tarjeta: %d\n", shared_mem->buffer[1023].tarjeta);
  printf("fecha: %s\n", shared_mem->buffer[1023].fecha);

  return 0;
}
