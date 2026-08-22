#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  // Matches the stack built by pushal, the vector number, and the iret frame.
  uintptr_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uintptr_t irq, eip, cs, eflags;
  void *cr3;
};

#define GPR1 eax
#define GPR2 ebx
#define GPR3 ecx
#define GPR4 edx
#define GPRx eax

#endif
