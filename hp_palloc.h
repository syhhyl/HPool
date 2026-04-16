/*
 * Copyright (C) Sun Yuhang
 */
 
#ifndef _HP_PALLOC_H_INCLUDED_
#define _HP_PALLOC_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>

#define HP_MAX_ALLOC_FROM_POOL (4096 - 1)
#define HP_POOL_ALIGNMENT 16
#define hp_align_ptr(d, a) \
  (u_char *) (((uintptr_t)(p) + ((uintptr_t)(a)-1)) & ~((uintptr_t)(a)-1)) 

#define HP_ALIGNMENT sizeof(uintptr_t)
#define HP_OK 0
#define HP_DECLINED -5

typedef unsigned char u_char;
typedef struct hp_pool_s hp_pool_t;
typedef uintptr_t hp_uint_t;
typedef intptr_t hp_int_t;
typedef void (*hp_pool_cleanup_pt)(void *data);
typedef struct hp_pool_cleanup_s hp_pool_cleanup_t;
typedef struct hp_pool_large_s hp_pool_large_t;
typedef int hp_fd_t;


struct hp_pool_large_s {
  hp_pool_large_t *next;
  void *alloc;
};

typedef struct {
    u_char *last;
    u_char *end;
    hp_pool_t *next;
    hp_uint_t failed;
} hp_pool_data_t;

struct hp_pool_cleanup_s {
  hp_pool_cleanup_pt handler;
  hp_pool_cleanup_t *next;
  void *data;
};

struct hp_pool_s{
  hp_pool_data_t d;
  size_t max;
  hp_pool_t *current;
  hp_pool_large_t *large;
  hp_pool_cleanup_t *cleanup;
};



typedef struct {
  hp_fd_t fd;
  u_char *name;
} hp_pool_cleanup_file_t;



hp_pool_t *hp_create_pool(size_t size);
void hp_destroy_pool(hp_pool_t *pool);

void *hp_palloc(hp_pool_t *pool, size_t size);
void *hp_pnalloc(hp_pool_t *pool, size_t size);




#endif
 

