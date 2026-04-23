#include "hp_palloc.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
  const char *name;
  int age;
} demo_user_t;

static void print_section(const char *title) {
  printf("\n== %s ==\n", title);
}

static void demo_cleanup(void *data) {
  const char *label = data;
  printf("generic cleanup: %s\n", label);
}

static int create_temp_file(char *path, size_t path_size, const char *prefix) {
  int written;

  written = snprintf(path, path_size, "/tmp/%sXXXXXX", prefix);
  if (written < 0 || (size_t) written >= path_size) {
    return -1;
  }

  return mkstemp(path);
}

static void write_text_file(int fd, const char *text) {
  hp_int_t written;
  size_t len = strlen(text);

  written = hp_write(fd, text, len);
  if (written < 0 || (size_t) written != len) {
    printf("write failed\n");
  }
}

int main() {
  char *msg1;
  char *msg2;
  int *numbers;
  demo_user_t *user;
  void *large;
  void *aligned;
  hp_pool_cleanup_t *cln;
  hp_pool_cleanup_t *file_cln;
  hp_pool_cleanup_t *delete_cln;
  hp_pool_cleanup_file_t *delete_data;
  char file_template[512];
  char delete_template[512];
  int file_fd;
  int delete_fd;


  hp_pool_t *pool = hp_create_pool(HP_DEFAULT_POOL_SIZE);
  if (pool == NULL) {
    printf("create pool failed\n");
    return 1;
  }

  print_section("1. create pool");
  printf("create pool ok\n");
  printf("addr(pool): %p\n", pool);
  printf("addr(current.last): %p\n", pool->current->d.last);
  printf("addr(current.end): %p\n", pool->current->d.end);

  print_section("2. small allocations");
  msg1 = hp_palloc(pool, 50);
  msg2 = hp_pnalloc(pool, 64);
  numbers = hp_pcalloc(pool, 4 * sizeof(int));
  user = hp_palloc(pool, sizeof(demo_user_t));

  if (msg1 == NULL || msg2 == NULL || numbers == NULL || user == NULL) {
    printf("small allocation failed\n");
    hp_destroy_pool(pool);
    return 1;
  }

  printf("addr(msg1): %p\n", msg1);
  printf("addr(msg2): %p\n", msg2);
  printf("addr(numbers): %p\n", numbers);
  printf("addr(user): %p\n", user);

  snprintf(msg1, 50, "I'am msg1 from hp_palloc");
  snprintf(msg2, 64, "I'am msg2 from hp_pnalloc");
  user->name = "hp_pool";
  user->age = 26;

  printf("hp_palloc  -> %s\n", msg1);
  printf("hp_pnalloc -> %s\n", msg2);
  printf("hp_pcalloc -> %d %d %d %d\n",
         numbers[0], numbers[1], numbers[2], numbers[3]);
  printf("struct alloc -> name=%s age=%d\n", user->name, user->age);

  print_section("3. large allocations");
  large = hp_palloc(pool, HP_MAX_ALLOC_FROM_POOL + 128);
  aligned = hp_pmemalign(pool, 128, 32);
  if (large == NULL || aligned == NULL) {
    printf("large allocation failed\n");
    hp_destroy_pool(pool);
    return 1;
  }

  printf("hp_palloc(large) -> ptr=%p size=%d\n", large, HP_MAX_ALLOC_FROM_POOL + 128);
  printf("hp_pmemalign -> ptr=%p ptr%%32=%lu\n",
         aligned, (unsigned long) ((uintptr_t) aligned % 32));
  printf("hp_pfree(large) -> %ld\n", (long) hp_pfree(pool, large));
  printf("hp_pfree(aligned) -> %ld\n", (long) hp_pfree(pool, aligned));
  printf("hp_pfree(msg1) -> %ld (small block cannot be individually freed)\n",
         (long) hp_pfree(pool, msg1));

  print_section("4. cleanup hooks");
  cln = hp_pool_cleanup_add(pool, 0);
  if (cln != NULL) {
    cln->handler = demo_cleanup;
    cln->data = "cleanup hook runs on destroy_pool | reset_pool";
  }

  file_fd = create_temp_file(file_template, sizeof(file_template), "hpc");
  if (file_fd == -1) {
    printf("create cleanup file failed\n");
    hp_destroy_pool(pool);
    return 1;
  }

  write_text_file(file_fd, "pool cleanup file\n");
  file_cln = hp_pool_cleanup_add(pool, sizeof(hp_pool_cleanup_file_t));
  if (file_cln == NULL) {
    printf("add file cleanup failed\n");
    hp_close_file(file_fd);
    hp_destroy_pool(pool);
    return 1;
  }

  ((hp_pool_cleanup_file_t *)file_cln->data)->fd = file_fd;
  ((hp_pool_cleanup_file_t *)file_cln->data)->name = (u_char *)file_template;
  file_cln->handler = hp_pool_cleanup_file;

  printf("run cleanup for fd=%d, path=%s\n", file_fd, file_template);
  hp_pool_run_cleanup_file(pool, file_fd);
  hp_delete_file((u_char *) file_template);

  delete_fd = create_temp_file(delete_template, sizeof(delete_template), "hpd");
  if (delete_fd == -1) {
    printf("create delete file failed\n");
    hp_destroy_pool(pool);
    return 1;
  }

  write_text_file(delete_fd, "pool delete file\n");
  delete_cln = hp_pool_cleanup_add(pool, sizeof(hp_pool_cleanup_file_t));
  if (delete_cln == NULL) {
    printf("add delete cleanup failed\n");
    hp_close_file(delete_fd);
    hp_delete_file((u_char *) delete_template);
    hp_destroy_pool(pool);
    return 1;
  }

  delete_data = delete_cln->data;
  delete_data->fd = delete_fd;
  delete_data->name = (u_char *) delete_template;
  delete_cln->handler = hp_pool_delete_file;

  printf("registered delete cleanup for path=%s\n", delete_template);

  print_section("5. reset pool");
  printf("reset pool and allocate again\n");
  hp_reset_pool(pool, 0);

  msg1 = hp_palloc(pool, 32);
  
  if (msg1 == NULL) {
    printf("alloc after reset failed\n");
    hp_destroy_pool(pool);
    return 1;
  }
  printf("addr(msg1): %p\n", msg1);

  snprintf(msg1, 32, "I am msg1 from after reset pool");
  printf("after reset -> %s\n", msg1);

  print_section("6. destroy pool");
  printf("destroy pool\n");
  hp_destroy_pool(pool);

  printf("delete file should now be removed: %s\n", delete_template);
  printf("done\n");

  return 0;
}
