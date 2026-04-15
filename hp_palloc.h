/*
 * Copyright (C) Sun Yuhang
 */
 
#ifndef _HP_PALLOC_H_INCLUDED_
#define _HP_PALLOC_H_INCLUDED_

#include <stddef.h>


typedef struct hp_pool_large_s hp_pool_large_t;
struct hp_pool_large_s {
  hp_pool_large_t *next;
  void *alloc;
};

typedef struct hp_pool_s hp_pool_t;
struct hp_pool_s{
  
  size_t max;
  hp_pool_t *current;
  hp_pool_large_t *large;
  
   
};

#endif
 

