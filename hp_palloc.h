/*
 * Copyright (C) Sun Yuhang
 */

#ifndef _HP_PALLOC_H_INCLUDED_
#define _HP_PALLOC_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define HP_MAX_ALLOC_FROM_POOL (4096 - 1)
#define HP_POOL_ALIGNMENT 16
#define hp_align_ptr(d, a) \
  (u_char *) (((uintptr_t)(d) + ((uintptr_t)(a)-1)) & ~((uintptr_t)(a)-1)) 
#define hp_memzero(buf, n) memset((buf), 0, n)
#define hp_close_file(fd) close(fd)
#define hp_delete_file(name) unlink((const char *) (name))

#define HP_ALIGNMENT sizeof(uintptr_t)
#define HP_OK 0
#define HP_DECLINED -5
#define HP_FILE_ERROR -1
#define HP_DEFAULT_POOL_SIZE 16 * 1024

typedef unsigned char u_char;
typedef void (*hp_pool_cleanup_pt)(void *data);
typedef int hp_fd_t;
typedef intptr_t hp_int_t;
typedef uintptr_t hp_uint_t;
typedef struct hp_pool_s hp_pool_t;
typedef struct hp_pool_large_s hp_pool_large_t;
typedef struct hp_pool_cleanup_s hp_pool_cleanup_t;


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

struct hp_pool_s {
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
void hp_reset_pool(hp_pool_t *pool);

void *hp_palloc(hp_pool_t *pool, size_t size);
void *hp_pnalloc(hp_pool_t *pool, size_t size);
void *hp_pcalloc(hp_pool_t *pool, size_t size);
void *hp_pmemalign(hp_pool_t *pool, size_t size, size_t alignment);
hp_int_t hp_pfree(hp_pool_t *pool, void *p);

hp_pool_cleanup_t *hp_pool_cleanup_add(hp_pool_t *p, size_t size);
void hp_pool_run_cleanup_file(hp_pool_t *p, hp_fd_t fd);
void hp_pool_cleanup_file(void *data);
void hp_pool_delete_file(void *data);

#endif
