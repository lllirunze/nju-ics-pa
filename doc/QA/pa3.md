## PA3 - Journey through time: batch system

### Exception response mechanism

First, we need to know that the yield test is in `am-kernels/tests/am-tests/src/tests/intr.c`. In the code, we can find that `yield()` will perform the following content, which indicates that yield function will set $a7 as -1 and trigger `ecall`.

```c
// abstract-machine/am/src/riscv/nemu/cte.c
void yield() {
  asm volatile("li a7, -1; ecall");
#endif
}
```

So when ecall is triggered, we execute the following command:

```c
// nemu/src/isa/riscv32/inst.c
s->dnpc = isa_raise_intr(R(17), s->pc) // R(17) is $a7
```

### Reorganising `Context` structure

We need to reorder the sequence of `Context` in order to get the correct values for each register. This order is determined by `abstract-machine/am/src/riscv/nemu/trap.S`.

```asm
.align 3
.globl __am_asm_trap
__am_asm_trap:
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
```

Here we can get the order of registers.

```c
// abstract-machine/am/include/arch/riscv.h
struct Context {
  uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
  void *pdir;
};
```

### Understand the antecedents of `Context`

You will see in `__am_irq_handle()` that there is a Context pointer `c`, where exactly is the context structure that `c` points to? Where did Context come from? 

- The context structure pointed to by `c` is `__am_asm_trap` in `trap.S'`.

Specifically, Context has many members, where exactly is each member assigned? 

- In `trap.S`, `__am_asm_trap` will open the space the size of `CONTEXT_SIZE`, push `gpr[NR_REGS]` onto the stack, then read `mcause, mstatus, and mepc` into general registers, and then push them into memory.

What is the connection among `$ISA-nemu.h`, `trap.S`, `__am_irq_handle` and NEMU?

- In exception, we set up control and status registers via `isa_raise_intr` of NEMU, and then go to `__am_asm_trap` function in `trap.S`, which will stack all the registers to form a Context structure. Finally, jump to `__am_irq_handle` to handle system calls.

### Recognize EVENT_YIELD

Earlier, we found that yield event sets `$a7` to -1. So we need to write the code:

```c
// abstract-machine/am/src/riscv/nemu/cte.c
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case xxx:
    }
  }
	// xxx
}
```

### Plus 4 for CISC and RISC

When the event is yield, we will add 4 to `mepc`.

```c
// abstract-machine/am/src/riscv/nemu/cte.c
Context* __am_irq_handle(Context *c) {
	if (user_handler) {
    // xxx
    if (ev.event == EVENT_YIELD) c->mepc += 4;
		// xxx
  }
}
```

### Understand the journey through time

From calling `yield()` to return, what happened on the journey? How do the software (AM) and hardware (NEMU) assist each other to complete this journey?

- The main function (`am-kernels/tests/am-tests/src/main.c`) defines the exception entry via:  
  - `asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));`
- Then use `simple_trap` as the function to execute after the exception.
- After trapping `yield()` in `hello_intr()` function, yield function will execute assembly instructions: 
  - `asm volatile("li a7, -1; ecall");`
- `li` instruction records -1 in the `$a7` register, which is `mcause`. `ecall` will initiate a yield instruction, i.e. call `isa_raise_intr`.
  - `s->dnpc = isa_raise_intr(R(17), s->pc);`
- `isa_raise_intr` will record `mcause` and `mepc`, then jump to `mtvec`, which is the exception entry `__am_asm_trap` defined by `trap.S`.
- `__am_asm_trap` will save the site and call `__am_irq_handle` in `cte.c`.
- `__am_irq_handle` identifies the type of exception and set `ev.event` with the help of `c->mcause`. Because this is a yield instruction, the software will add 4 to `c->mepc`, which will point to the next instruction of PC. This is followed by `c = user_handler(ev, c);`, in this case `simple_trap`, which specifically outputs 'y'.
- When execution is complete, it returns to `trap.S` for recover the site and eventually returns via `mret`.
























### Unable to convert PDF

```bash
ubuntu@ubuntu-vm:~/Desktop/ics2024/navy-apps/apps/nslider/slides$ ./convert.sh
convert-im6.q16: attempt to perform an operation not allowed by the security policy `PDF' @ error/constitute.c/IsCoderAuthorized/426.
convert-im6.q16: no images defined `slides.bmp' @ error/convert.c/ConvertImageCommand/3229.
rm: cannot remove '/home/ubuntu/Desktop/ics2024/navy-apps/fsimg/share/slides/*': No such file or directory
mv: cannot stat '*.bmp': No such file or directory
```

Run this command:

```bash
sudo vim /etc/ImageMagick-6/policy.xml 

# Find this row: <policy domain="coder" rights="none" pattern="PDF" />
# Modify this row: <policy domain="coder" rights="read | write" pattern="PDF" />
```

After run `./convert.sh`, we may find that issue: `rm: cannot remove '/home/ubuntu/Desktop/ics2024/navy-apps/fsimg/share/slides/*': No such file or directory`. This issue doesn't matter. Just ignore it.

### How to run Flappy Bird in Navy?

First of all, we need to create a symbolic link.

```make
# navy-apps/apps/bird/Makefile
install-file:
	ln -sf -T $(abspath $(REPO_PATH)/res) $(NAVY_HOME)/fsimg/share/games/bird
```

Because in `navy-apps/apps/bird/repo/include/BirdGame.h`, there are some code:

```c
#ifdef __NAVY__
#define RES_PREFIX "/share/games/bird/"
#else
#define RES_PREFIX "res/"
#endif
```

It means that if we use Navy (native or riscv32) to run Flappy Bird, this game will use the images in "/share/games/bird/" instead of "res/", which is run in Linux native.

After running `make ISA=native install-file`, we can play this game using `make ISA=native run`.

### How to get the data of PAL?

[link](https://blog.csdn.net/weixin_63603830/article/details/134065932)
