/*
 * Copyright (C) Sun Yuhang
 */

#include "hp_palloc.h"

#include <stdio.h>
#include <stdlib.h>

static inline void *hp_memalign(size_t alignment, size_t size) {
  void *base;
  void **aligned;
  size_t total;
  uintptr_t addr;

  if ((alignment & (alignment - 1)) != 0) {
    return NULL;
  }

  if (alignment < sizeof(void *)) {
    alignment = sizeof(void *);
  }

  total = size + alignment - 1 + sizeof(void *);
  base = malloc(total);
  if (base == NULL) return NULL;

  addr = (uintptr_t) base + sizeof(void *);
  addr = (addr + alignment - 1) & ~((uintptr_t) alignment - 1);
  aligned = (void **) addr;
  aligned[-1] = base;

  return aligned;
}

static inline void hp_free(void *p) {
  if (p == NULL) return;

  free(((void **) p)[-1]);
}

hp_pool_t *hp_create_pool(size_t size) {
  hp_pool_t *p;

  p = hp_memalign(HP_POOL_ALIGNMENT, size);
  if (p == NULL) return NULL;

  p->d.last = (u_char *)p + sizeof(hp_pool_t);
  p->d.end = (u_char *)p + size;
  p->d.next = NULL;
  p->d.failed = 0;

  size = size - sizeof(hp_pool_t);
  p->max = (size < HP_MAX_ALLOC_FROM_POOL) ? size : HP_MAX_ALLOC_FROM_POOL;

  p->current = p;
  p->large = NULL;
  p->cleanup = NULL;

  return p;
}



void hp_destroy_pool(hp_pool_t *pool) {
  hp_pool_t *p, *n;
  hp_pool_large_t *l;
  hp_pool_cleanup_t *c;

  for (c = pool->cleanup; c; c = c->next) {
    if (c->handler) {
      c->handler(c->data);
    }
  }

  for (l = pool->large; l; l = l->next) {
    if (l->alloc) {
      hp_free(l->alloc);
    }
  }

  for (p = pool, n = pool->d.next; ; p = n, n = n->d.next) {
    hp_free(p);
    if (n == NULL) break;
  }
}

void hp_reset_pool(hp_pool_t *pool) {
  hp_pool_t *p;
  hp_pool_large_t *l;

  pool->cleanup = NULL;

  for (l = pool->large; l; l = l->next) {
    if (l->alloc) {
      hp_free(l->alloc);
    }
  }

  for (p = pool; p; p = p->d.next) {
    p->d.last = (u_char *)p + sizeof(hp_pool_t);
    p->d.failed = 0;
  }

  pool->current = pool;
  pool->large = NULL;
}

static void *hp_palloc_block(hp_pool_t *pool, size_t size) {
  u_char *m;
  size_t psize;
  hp_pool_t *p, *new;

  psize = (size_t) (pool->d.end - (u_char *)pool);
  m = hp_memalign(HP_POOL_ALIGNMENT, psize);
  if (m == NULL) {
    return NULL;
  }

  new = (hp_pool_t *)m;
  new->d.end = m + psize;
  new->d.next = NULL;
  new->d.failed = 0;

  m += sizeof(hp_pool_data_t);
  m = hp_align_ptr(m, HP_ALIGNMENT);
  new->d.last = m + size;

  for (p = pool->current; p->d.next; p = p->d.next) {
    if (p->d.failed++ > 4) {
      pool->current = p->d.next;
    }
  }
  p->d.next = new;
  return m;
}

static inline void *hp_palloc_small(hp_pool_t *pool, size_t size, hp_uint_t align) {
  u_char *m;
  hp_pool_t *p;

  p = pool->current;

  do {
    m = p->d.last;
    if (align) {
      m = hp_align_ptr(m, HP_ALIGNMENT);
    }

    if ((size_t)(p->d.end - m) >= size) {
      p->d.last = m + size;
      return m;
    }

    p = p->d.next;
  } while (p);

  return hp_palloc_block(pool, size);
}

static inline void *hp_alloc(size_t size) {
  return hp_memalign(HP_ALIGNMENT, size);
}

static void *hp_palloc_large(hp_pool_t *pool, size_t size) {
  void *p;
  hp_uint_t n;
  hp_pool_large_t *large;

  p = hp_alloc(size);
  if (p == NULL) return NULL;

  n = 0;
  for (large = pool->large; large; large = large->next) {
    if (large->alloc == NULL) {
      large->alloc = p;
      return p;
    }

    if (n++ > 3) break;
  }

  large = hp_palloc_small(pool, sizeof(hp_pool_large_t), 1);
  if (large == NULL) {
    hp_free(p);
    return NULL;
  }

  large->alloc = p;
  large->next = pool->large;
  pool->large = large;

  return p;
}

void *hp_pmemalign(hp_pool_t *pool, size_t size, size_t alignment) {
  void *p;
  hp_pool_large_t *large;

  p = hp_memalign(alignment, size);
  if (p == NULL) {
    return NULL;
  }

  large = hp_palloc_small(pool, sizeof(hp_pool_large_t), 1);
  if (large == NULL) {
    hp_free(p);
    return NULL;
  }

  large->alloc = p;
  large->next = pool->large;
  pool->large = large;

  return p;
}

void *hp_palloc(hp_pool_t *pool, size_t size) {
  if (size <= pool->max) {
    return hp_palloc_small(pool, size, 1);
  }

  return hp_palloc_large(pool, size);
}

void *hp_pnalloc(hp_pool_t *pool, size_t size) {
  if (size <= pool->max) {
    return hp_palloc_small(pool, size, 0);
  }

  return hp_palloc_large(pool, size);
}

hp_int_t hp_pfree(hp_pool_t *pool, void *p) {
  hp_pool_large_t *l;

  for (l = pool->large; l; l = l->next) {
    if (p == l->alloc) {
      hp_free(l->alloc);
      l->alloc = NULL;

      return HP_OK;
    }
  }

  return HP_DECLINED;
}

void *hp_pcalloc(hp_pool_t *pool, size_t size) {
  void *p;

  p = hp_palloc(pool, size);

  if (p) {
    hp_memzero(p, size);
  }

  return p;
}

hp_pool_cleanup_t *hp_pool_cleanup_add(hp_pool_t *p, size_t size) {
  hp_pool_cleanup_t *c;

  c = hp_palloc(p, sizeof(hp_pool_cleanup_t));
  if (c == NULL) {
    return NULL;
  }

  if (size) {
    c->data = hp_palloc(p, size);
    if (c->data == NULL) {
      return NULL;
    }
  } else {
    c->data = NULL;
  }

  c->handler = NULL;
  c->next = p->cleanup;

  p->cleanup = c;

  return c;
}

void hp_pool_run_cleanup_file(hp_pool_t *p, hp_fd_t fd) {
  hp_pool_cleanup_t *c;
  hp_pool_cleanup_file_t *cf;

  for (c = p->cleanup; c; c = c->next) {
    if (c->handler == hp_pool_cleanup_file) {
      cf = c->data;

      if (cf->fd == fd) {
        c->handler(cf);
        c->handler = NULL;
        return;
      }
    }
  }
}

void hp_pool_cleanup_file(void *data) {
  hp_pool_cleanup_file_t *c = data;

  if (hp_close_file(c->fd) == HP_FILE_ERROR) {
    fprintf(stderr, "hp_close_file failed\n");
  }
}

void hp_pool_delete_file(void *data) {
  hp_pool_cleanup_file_t *c = data;

  if (hp_close_file(c->fd) == HP_FILE_ERROR) {
    fprintf(stderr, "hp_close_file failed\n");
  }

  if (hp_delete_file(c->name) == HP_FILE_ERROR) {
    fprintf(stderr, "hp_delete_file failed\n");
  }
}
