#include <stdio.h>
#include "hp_palloc.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  const char *name;
  int age;
} demo_user_t;

static void demo_cleanup(void *data) {
  const char *label = data;
  printf("generic cleanup: %s\n", label);
}

static void write_text_file(int fd, const char *text) {
  size_t len = strlen(text);

  if (write(fd, text, len) != (ssize_t) len) {
    printf("write failed\n");
  }
}

int main() {
  hp_pool_t *pool = hp_create_pool(HP_DEFAULT_POOL_SIZE);
  if (pool == NULL) {
    printf("create pool failed\n");
    return 1;
  }
  
  printf("[1] create pool ok\n");
  printf("pool first addr: %p\n", pool);

  char *msg1 = hp_palloc(pool, 64);
  printf("addr(msg1): %p\n", msg1);
  printf("addr(pool+sizeof(hp_pool_t)): %p\n", (u_char *)pool+sizeof(hp_pool_t));
  // char *msg2 = hp_pnalloc(pool, 64);
  // int *numbers = hp_palloc(pool, 5 * sizeof(int));
  // demo_user_t *user = hp_palloc(pool, sizeof(demo_user_t));
  // void *aligned = hp_pmemalign(pool, 128, 32);
  
  // if (msg1 == NULL || msg2 == NULL || numbers == NULL || user == NULL || aligned == NULL) {
  //   printf("pool alloc failed\n");
  //   hp_destroy_pool(pool);
  //   return 1;
  // }
  
  snprintf(msg1, 64, "I'am msg1 from hp_palloc");
  
  
  printf("2) hp_palloc -> %s\n", msg1);




  return 0;
}