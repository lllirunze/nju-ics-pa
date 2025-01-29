# PA3 - Journey through time: batch system

## Exception response mechanism

First, we need to know that the yield test is in `am-kernels/tests/am-tests/src/tests/intr.c`. In the code, we can find that `yield()` will perform the following content, which indicates that yield function will set $a7 as -1 and trigger `ecall`.

```c
// abstract-machine/am/src/riscv/nemu/cte.c
void yield() {
  asm volatile("li a7, -1; ecall");
}
```

So when ecall is triggered, we execute the following command:

```c
// nemu/src/isa/riscv32/inst.c
s->dnpc = isa_raise_intr(R(17), s->pc) // R(17) is $a7
```

## Reorganising `Context` structure

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

## Understand the antecedents of `Context`

You will see in `__am_irq_handle()` that there is a Context pointer `c`, where exactly is the context structure that `c` points to? Where did Context come from? 

* The context structure pointed to by `c` is `__am_asm_trap` in `trap.S'`.

Specifically, Context has many members, where exactly is each member assigned? 

* In `trap.S`, `__am_asm_trap` will open the space the size of `CONTEXT_SIZE`, push `gpr[NR_REGS]` onto the stack, then read `mcause, mstatus, and mepc` into general registers, and then push them into memory.

What is the connection among `$ISA-nemu.h`, `trap.S`, `__am_irq_handle` and NEMU?

* In exception, we set up control and status registers via `isa_raise_intr` of NEMU, and then go to `__am_asm_trap` function in `trap.S`, which will stack all the registers to form a Context structure. Finally, jump to `__am_irq_handle` to handle system calls.

## Recognize EVENT_YIELD

Earlier, we found that yield event sets `$a7` to -1. So we need to write the code:

```c
// abstract-machine/am/src/riscv/nemu/cte.c
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case -1: ev.event = EVENT_YIELD; break;
      case ...:
    }
  }
	// ...
}
```

## Plus 4 for CISC and RISC

When the event is yield, we will add 4 to `mepc`.

```c
// abstract-machine/am/src/riscv/nemu/cte.c
Context* __am_irq_handle(Context *c) {
	if (user_handler) {
    // ...
    if (ev.event == EVENT_YIELD) c->mepc += 4;
		// ...
  }
}
```

## Understand the journey through time

From calling `yield()` to return, what happened on the journey? How do the software (AM) and hardware (NEMU) assist each other to complete this journey?

* The main function (`am-kernels/tests/am-tests/src/main.c`) defines the exception entry via:  
  * `asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));`
* Then use `simple_trap` as the function to execute after the exception.
* After trapping `yield()` in `hello_intr()` function, yield function will execute assembly instructions: 
  * `asm volatile("li a7, -1; ecall");`
* `li` instruction records -1 in the `$a7` register, which is `mcause`. `ecall` will initiate a yield instruction, i.e. call `isa_raise_intr`.
  * `s->dnpc = isa_raise_intr(R(17), s->pc);`
* `isa_raise_intr` will record `mcause` and `mepc`, then jump to `mtvec`, which is the exception entry `__am_asm_trap` defined by `trap.S`.
* `__am_asm_trap` will save the site and call `__am_irq_handle` in `cte.c`.
* `__am_irq_handle` identifies the type of exception and set `ev.event` with the help of `c->mcause`. Because this is a yield instruction, the software will add 4 to `c->mepc`, which will point to the next instruction of PC. This is followed by `c = user_handler(ev, c);`, in this case `simple_trap`, which specifically outputs 'y'.
* When execution is complete, it returns to `trap.S` for recover the site and eventually returns via `mret`.

## Implement event distribution for Nanos-lite

In `nanos-lite/`, run `make ARCH=$ISA-nemu run` to launch Nanos-lite.

When the execution reaches `yield()`, we need to determine the event type in `do_event()`.

```c
// nanos-lite/src/irq.c
static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      // Log a message, and no other action is required at this time.
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  // ...
}
```

## How to run Navy-apps in Nanos?

Take `navy-apps/tests/dummy/` for example. Run the following commands:

```shell
# In `navy-apps/tests/dummy/`
$ make ISA=riscv32
# After compilation, manually copy `navy-apps/tests/dummy/build/dummy-riscv32`.
# Paste it and rename it as `nanos-lite/build/ramdisk.img`.
# In `nanos-lite`
$ make ARCH=riscv32-nemu
```

## Where is heap and stack?

`abstract-machine/script/linker.ld`

## How to recognize executable files in different formats?

Files have headers, and file parser will parse them based on the headers.

## Duplicated attibutes

What are `FileSiz` and `MemSiz`?

* `FileSiz` is the size of the data in the file, which is the size of the initialized data.
* `MemSiz` is the size of the data in the memory, which contains the uninitialized data.

## Why do we need to zero out [VirtAddr+FileSiz, VirtAddr+MemSiz)?

This memory may have malicious code or sensitive data from the previous program.

## Check magic number of ELF file

In `nanos-lite/build/`, run the following command and get the result `0x464c457f`.

```shell
$ readelf -h nanos-lite-riscv32-nemu.elf

# result
ELF Header:
  Magic:   7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF32
  Data:                              2's complement, little endian
  Version:                           1 (current)
  ...
```

So we edit the code:

```c
// nanos-lite/src/loader.c
static uintptr_t loader(PCB *pcb, const char *filename) {
  // ...
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
  // ...
}
```

## Check ISA type of ELF type

```c
// nanos-lite/src/loader.c
#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_386
#elif defined(__ISA_RISCV32__)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#else
# error Unsupported ISA
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  // ...
  assert(ehdr.e_machine == EXPECT_TYPE);
  // ...
}
```

## Are system calls necessary for batch systems?

It's not necessary, because programs in the batch system are executed serially and there is no resource grabbing.

## Implement proper `GPR?` macro

In `navy-apps/libs/libos/src/syscall.c` (code as below), we can find that

```c
// navy-apps/libs/libos/src/syscall.c
// extract an argument from the macro array
#define SYSCALL  _args(0, ARGS_ARRAY)
#define GPR1 _args(1, ARGS_ARRAY)
#define GPR2 _args(2, ARGS_ARRAY)
#define GPR3 _args(3, ARGS_ARRAY)
#define GPR4 _args(4, ARGS_ARRAY)
#define GPRx _args(5, ARGS_ARRAY)

// ISA-depedent definitions
#if ...
#elif defined(__riscv)
#ifdef ...
#else
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#endif
...
#endif

intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
  register intptr_t _gpr1 asm (GPR1) = type;
  register intptr_t _gpr2 asm (GPR2) = a0;
  register intptr_t _gpr3 asm (GPR3) = a1;
  register intptr_t _gpr4 asm (GPR4) = a2;
  register intptr_t ret asm (GPRx);
  asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3), "r"(_gpr4));
  return ret;
}
```

We can find the assembly instruction executed in `_syscall_()`: `ecall, $a7, $a0, $a1, $a2, $a0`.  According to ``, the indexes of registers are:

```c
// nemu/src/isa/riscv32/reg.c
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
```

So edit `GPR?` macro in `riscv.h`.

```c
// abstract-machine/am/include/arch/riscv.h
#define GPR1 gpr[17] // $a7
#define GPR2 gpr[10] // $a0
#define GPR3 gpr[11] // $a1
#define GPR4 gpr[12] // $a2
#define GPRx gpr[10] // $a0
```

## What is the hello.c?  Where does it come from, and where does it go? 

First, `hello.c` is compiled into an ELF file under the `navy-apps/tests/hello/build`. Then the executable file is renamed and placed in `nanos-lite/build` and placed at [ramdisk_start, ramdisk_end]. Finally, nanos-lite and abstract-machine are compiled into ELF files and sent to nemu.

## Implement complete file system

Here we need to know where we should write code:

```c
// navy-apps/libs/libos/src/syscall.c
int _open(const char *path, int flags, mode_t mode) {...}
int _write(int fd, void *buf, size_t count) {...}
void *_sbrk(intptr_t increment) {...}
int _read(int fd, void *buf, size_t count) {...}
int _close(int fd) {...}
off_t _lseek(int fd, off_t offset, int whence) {...}
int _gettimeofday(struct timeval *tv, struct timezone *tz) {...}
int _execve(const char *fname, char * const argv[], char *const envp[]) {...}
```

```c
// abstract-machine/am/src/riscv/nemu/cte.c
Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case SYS_...: ev.event = EVENT_SYSCALL; break;
      default:      ev.event = EVENT_ERROR;   break;
    }
  return c;
}
```

```c
// nanos-lite/src/syscall.c
void do_syscall(Context *c) {
  uintptr_t a[4];
  switch (a[0]) {
    case SYS_...: c->GPRx = ...; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
```

Writing the code for these three parts and the corresponding functions enables the syscall.

## Implement NDL_GetTicks()

Here we need to notice that `NDL_GetTicks` will return **microseconds**. So we write the code:

```c
uint32_t NDL_GetTicks() {
  gettimeofday(&tv, &tz);
  current_usec = tv.tv_sec * 1000000 + tv.tv_usec;
  return (current_usec-start_usec)/1000;
}
```

Attention: In order to compile the `navy-apps/tests` successfully, we need to add NDL lib to Makefile.

```make
# navy-apps/Makefile
LIBS += libc libos libndl libminiSDL
```

## Use fopen() or open()?

Use `open()`. `open()` can operate character devices, `fopen()` is usually operating *regular file*, read will take a buffer (i.e. read and write a character is not 100% read and write to disk).

## fixedpt

Let `fixedpt A = a << FIXEDPT_FBITS, B = b << FIXEDPT_FBITS;`, We get the following equation:

```
A + B = (a + b) << FIXEDPT_FBITS;
A - B = (a - b) << FIXEDPT_FBITS;
A * B = (a * b) >> FIXEDPT_FBITS;
A / B = (a << FIXEDPT_FBITS) / b;
floor(A) = A & 0xffffff00;
ceil(A) = (floor(A) == A) ? A : A + FIXEDPT_ONE;
```

Note that multiplication and division need to extend the range of data types, i.e. `uint32_t` temporarily needs to be extended to `uint64_t`.

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
