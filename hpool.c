#include <stdio.h>
#include "hp_palloc.h"


int main() {
  hp_pool_t *p;
  
  p = hp_create_pool(1024);
  char *msg1 = hp_palloc(p, 10);
  msg1 = "hello world yes hejaafs";
  
  printf("msg1: %s\n", msg1);
  return 0;
  
  
}