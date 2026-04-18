# HPool

一个简洁的 C 语言内存池示例项目，用来演示 `pool allocator (内存池分配器)` 的基础实现和常见用法。

## 功能

- `hp_create_pool` / `hp_destroy_pool`
- `hp_palloc` / `hp_pnalloc` / `hp_pcalloc`
- `large allocation (大块分配)`
- `hp_pmemalign` 对齐分配
- `hp_pfree` 释放 large block
- `cleanup hook (清理回调)`
- `hp_reset_pool`

## 构建

```bash
make
```

## 运行

```bash
./hpool
```

## Demo 内容

`hpool.c` 会按教程方式依次演示：

1. 创建 pool
2. small allocations
3. large allocations
4. cleanup hooks
5. reset pool
6. destroy pool

## 文件说明

- `hp_palloc.h`：公开接口和数据结构定义
- `hp_palloc.c`：内存池实现
- `hpool.c`：教程式 demo
- `Makefile`：构建脚本

## 说明

这个项目偏学习和演示用途，重点是帮助理解内存池的分配、清理和生命周期管理。
