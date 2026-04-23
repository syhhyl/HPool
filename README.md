# HPool
HPool is derived from Nginx’s ngx_pool, with several modifications.

## How to use

1. create pool
`hp_create_pool()`

2. dejstroy pool
`hp_destroy_pool()`

3. reset pool
`hp_reset_pool()` clean controls whether the cleanup chain should be processed.

4. Allocate aligned small memory. 
`hp_palloc()`

5. Allocate aligned small memory.  
`hp_pnalloc()`

6. Allocate a large aligned memory block and create a cleanup node linked into the cleanup chain.
`hp_pmemalign()`

7. Free a large memory block.
`hp_pfree()`

8. Add a node to the cleanup chain.
`hp_pool_cleanup_add()`

9. Free a cleanup node individually. 
`hp_pool_run_cleanup_file()`

10. Close a file.
`hp_pool_cleanup_file()`

11. Delete a file.
`hp_pool_delete_file()`
