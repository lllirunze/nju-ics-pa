#include <am.h>
#include <nemu.h>
#include <klib.h>

#define SEG_KCODE 1

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0xc0000000)

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  memset(kas.ptr, 0, PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_cr3(kas.ptr);
  set_cr0(get_cr0() | CR0_PG);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  memset(updir, 0, PGSIZE);
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
  PTE *pdir = (PTE *)as->ptr;
  for (int i = 0; i < 1024; i ++) {
    if ((pdir[i] & (PTE_P | PTE_U)) == (PTE_P | PTE_U)) {
      pgfree_usr((void *)(uintptr_t)(pdir[i] & ~0xfff));
    }
  }
  pgfree_usr(pdir);
  as->ptr = NULL;
}

void __am_get_cur_as(Context *c) {
  c->cr3 = (vme_enable ? (void *)get_cr3() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->cr3 != NULL) {
    set_cr3(c->cr3);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  uintptr_t vaddr = (uintptr_t)va;
  uintptr_t paddr = (uintptr_t)pa;
  assert((vaddr & (PGSIZE - 1)) == 0);
  assert((paddr & (PGSIZE - 1)) == 0);

  PTE *pdir = (PTE *)as->ptr;
  int pde = (vaddr >> 22) & 0x3ff;
  int pte = (vaddr >> 12) & 0x3ff;
  bool user = as != &kas;
  if (!(pdir[pde] & PTE_P)) {
    PTE *ptab = (PTE *)pgalloc_usr(PGSIZE);
    memset(ptab, 0, PGSIZE);
    pdir[pde] = (uintptr_t)ptab | PTE_P | PTE_W | (user ? PTE_U : 0);
  }

  PTE *ptab = (PTE *)(uintptr_t)(pdir[pde] & ~0xfff);
  assert(!(ptab[pte] & PTE_P));
  if (prot == MMAP_NONE) return;
  ptab[pte] = paddr | PTE_P | (user ? PTE_U : 0) |
      ((prot & MMAP_WRITE) ? PTE_W : 0);
}

Context* ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context *)((uintptr_t)kstack.end - sizeof(Context));
  memset(c, 0, sizeof(*c));
  c->eip = (uintptr_t)entry;
  c->eflags = FL_IF;
  c->cs = USEL(SEG_KCODE);
  c->esp = (uintptr_t)as->area.end;
  c->cr3 = as->ptr;
  return c;
}
