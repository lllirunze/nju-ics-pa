#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *p = pf;
  pf = (void *)((uintptr_t)pf + nr_page * PGSIZE);
  assert(pf <= heap.end);
  return p;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *p = new_page(n / PGSIZE);
  memset(p, 0, n);
  return p;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  assert(current != NULL);
  assert(brk <= (uintptr_t)current->as.area.end - STACK_SIZE);

  if (brk > current->max_brk) {
    uintptr_t va = ROUNDUP(current->max_brk, PGSIZE);
    uintptr_t end = ROUNDUP(brk, PGSIZE);
    for (; va < end; va += PGSIZE) {
      map(&current->as, (void *)va, new_page(1), MMAP_READ | MMAP_WRITE);
    }
    current->max_brk = brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
