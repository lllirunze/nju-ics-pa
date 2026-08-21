#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

typedef struct Block {
  size_t size;
  bool is_free;
  struct Block *next;
} Block;

#define MALLOC_ALIGNMENT (2 * sizeof(uintptr_t))

static Block *block_list = NULL;
static uintptr_t heap_cursor = 0;

static uintptr_t align_up(uintptr_t value) {
  return (value + MALLOC_ALIGNMENT - 1) & ~(MALLOC_ALIGNMENT - 1);
}

static size_t block_header_size(void) {
  return (size_t)align_up(sizeof(Block));
}

static void init_heap(void) {
  if (heap_cursor == 0 && heap.start != NULL && heap.end != NULL) {
    heap_cursor = align_up((uintptr_t)heap.start);
  }
}

static void split_block(Block *block, size_t size) {
  size_t header_size = block_header_size();
  if (block->size < size + header_size + MALLOC_ALIGNMENT) {
    return;
  }

  Block *remaining = (Block *)((uint8_t *)block + header_size + size);
  remaining->size = block->size - size - header_size;
  remaining->is_free = true;
  remaining->next = block->next;
  block->size = size;
  block->next = remaining;
}

static void coalesce_free_blocks(void) {
  for (Block *block = block_list; block != NULL && block->next != NULL; ) {
    Block *next = block->next;
    if (block->is_free && next->is_free) {
      block->size += block_header_size() + next->size;
      block->next = next->next;
    } else {
      block = next;
    }
  }
}

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  if (size == 0 || size > (size_t)-1 - (MALLOC_ALIGNMENT - 1)) {
    return NULL;
  }
  size = (size_t)align_up(size);
  init_heap();
  if (heap_cursor == 0) {
    return NULL;
  }

  for (Block *block = block_list; block != NULL; block = block->next) {
    if (block->is_free && block->size >= size) {
      split_block(block, size);
      block->is_free = false;
      return (uint8_t *)block + block_header_size();
    }
  }

  size_t header_size = block_header_size();
  uintptr_t heap_end = (uintptr_t)heap.end;
  if (heap_cursor > heap_end || size > heap_end - heap_cursor ||
      header_size > heap_end - heap_cursor - size) {
    return NULL;
  }

  Block *block = (Block *)heap_cursor;
  block->size = size;
  block->is_free = false;
  block->next = NULL;
  heap_cursor += header_size + size;

  if (block_list == NULL) {
    block_list = block;
  } else {
    Block *tail = block_list;
    while (tail->next != NULL) tail = tail->next;
    tail->next = block;
  }
  return (uint8_t *)block + header_size;
#endif
  return NULL;
}

void free(void *ptr) {
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  if (ptr == NULL) {
    return;
  }

  for (Block *block = block_list; block != NULL; block = block->next) {
    if ((uint8_t *)block + block_header_size() == ptr) {
      block->is_free = true;
      coalesce_free_blocks();
      return;
    }
  }
#endif
}

#endif
