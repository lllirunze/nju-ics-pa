# NEMU

NEMU(NJU Emulator) is a simple but complete full-system emulator designed for teaching purpose.
Currently it supports x86, mips32, riscv32 and riscv64.
To build programs run above NEMU, refer to the [AM project](https://github.com/NJU-ProjectN/abstract-machine).

The main features of NEMU include
* a small monitor with a simple debugger
  * single step
  * register/memory examination
  * expression evaluation without the support of symbols
  * watch point
  * differential testing with reference design (e.g. QEMU)
  * snapshot
* CPU core with support of most common used instructions
  * x86
    * real mode is not supported
    * x87 floating point instructions are not supported
  * mips32
    * CP1 floating point instructions are not supported
  * riscv32
    * only RV32IM
  * riscv64
    * only RV64IM
* memory
* paging
  * TLB is optional (but necessary for mips32)
  * protection is not supported
* interrupt and exception
  * protection is not supported
* 5 devices
  * serial, timer, keyboard, VGA, audio
  * most of them are simplified and unprogrammable
* 2 types of I/O
  * port-mapped I/O and memory-mapped I/O

## PA0 - The night before the world was born: configuration

### Add ssh key on github

```shell
$ ssh-keygen -t rsa -C "1927553272@qq.com"
# copy the content of ~/.ssh/id_rsa.pub into Github.
```

### Why do I get an error when I run 'make menuconfig' in the terminal?

We are missing the bison and flex libraries, so we run the following commands.

```shell
$ sudo apt-get install bison
$ sudo apt-get install flex
```

## PA1 - The opening chapter: the simplest computer

### Select your role

Here I choose **riscv-32**.

### We cannot get the right end when we first 'make run'

The error is in **/ics2024/nemu/src/monitor/monitor.c**. We can just comment it out.

```c
// ics2024/nemu/src/monitor/monitor.c
static void welcome() {
  ...
  // Log("Exercise: Please remove me in the source code and compile NEMU again.");
  // assert(0);
}
```

### What is cpu_exec(-1) ?

**cpu_exec(uint64_t n)** is used to control the number of instructions executed by CPU. In C, when -1 is converted to **unsigned long int** type, the value becomes a very large number $2^{64} -1$.

Thus, cpu_exec(-1) will allow the program to execute as many instructions as possible until some other condition (e.g., NEMU_END, NEMU_ABORT, NEMU_QUIT) is triggered and the program is stopped.

### Is cpu_exec(-1) an undefined behavior?

Taking -1 as an argument does **not** result in undefined behavior. It causes n to be converted to the maximum value of type uint64_t. This is legal and well defined behavior in the C99 standard.

Reference: [C99 standard - chapter 6.3.1.3](https://www.dii.uchile.cl/~daespino/files/Iso_C_1999_definition.pdf)

### Why do we need nemu_trap?

nemu_trap is used to **catch exceptions or traps** that occur in a program. It simulates the behavior of the hardware when it encounters an error and ensures that the program responds correctly when an exception occurs, such as reporting an error, terminating the program, or entering debug mode.

### Why do we need monitor?

Monitor provides **interactive debugging features** that allow users to control program execution, view program status, set breakpoints, and more.

### Why do we need '\n' in printf() ?

Line breaks '\n' are used to ensure that the output is formatted to display properly, and to maintain clear separation between multiple outputs.

### Where is selector in i386 (just find its location)?

[i386 - chapter 5.1.3](https://css.csail.mit.edu/6.858/2013/readings/i386.pdf)

### What are the command formats for riscv-32?

|--------|--------------------------------------------------------------------------------------|
|   bit  |    31   |30       25|24   21|    20   |19   15|14    12|11       8|    7    |6      0|
|--------|---------------------------------------|-------|--------|--------------------|--------|
| R-type |       funct7        |       rs2       |  rs1  | funct3 |        rd          | opcode |
|--------|---------------------------------------|-------|--------|--------------------|--------|
| I-type |               imm[11:0]               |  rs1  | funct3 |        rd          | opcode |
|--------|---------------------------------------|-------|--------|--------------------|--------|
| S-type |      imm[11:5]      |       rs2       |  rs1  | funct3 |     imm[4:0]       | opcode |
|--------|---------------------------------------|-------|--------|--------------------|--------|
| B-type | imm[12] | imm[10:5] |       rs2       |  rs1  | funct3 | imm[4:1] | imm[11] | opcode |
|--------|--------------------------------------------------------|--------------------|--------|
| U-type |                    imm[31:12]                          |        rd          | opcode |
|--------|--------------------------------------------------------|--------------------|--------|
| J-type | imm[20] |     imm[10:1]     | imm[11] |   imm[19:12]   |        rd          | opcode |
|--------|--------------------------------------------------------------------------------------|

### What is the behavior of the LUI command?

LUI (load upper immediate) is used to build 32-bit constants and uses the U-type format. LUI places the U-immediate value in the top 20 bits of the destination register rd, filling in the lowest 12 bits with zeros.

### What is the structure of the mstatus register?

The mstatus register is a read/write register formatted as shown in the following figures for riscv-32 and riscv-64. The mstatus register keeps track of and controls the current operating state of the hart.

Machine-mode status register (mstatus) for riscv-32:   
|----------|---------------------------------------------------------------------------------------------------------------------------------------------------|    
|    bit   | 31 |30  23|  22 | 21 |  20 |  19 |  18 |  17  |16     15|14     13|12      11|10      9|  8  |   7  |  6  |   5  |   4  |  3  |   2  |  1  |   0  |    
|----------|----|------|-----|----|-----|-----|-----|------|---------|---------|----------|---------|-----|------|-----|------|------|-----|------|-----|------|     
| riscv-32 | SD | WPRI | TSR | TW | TVM | MXR | SUM | MPRV | XS[1:0] | FS[1:0] | MPP[1:0] | VS[1:0] | SPP | MPIE | UBE | SPIE | WPRI | MIE | WPRI | SIE | WPRI |    
|----------|---------------------------------------------------------------------------------------------------------------------------------------------------|    

Machine-mode status register (mstatus) for riscv-64:   
|----------|-------------------------------------------------------------------------------------    
|    bit   | 63 |62  38|  37 |  36 |35      34|33      32|31  23|  22 | 21 |  20 |  19 |  18 |   
|----------|----|------|-----|-----|----------|----------|------|-----|----|-----|-----|-----|---   
| riscv-64 | SD | WPRI | MBE | SBE | SXL[1:0] | UXL[1:0] | WPRI | TSR | TW | TVM | MXR | SUM |    
|----------|-------------------------------------------------------------------------------------   

        --------------------------------------------------------------------------------------------------------------|    
           |  17  |16     15|14     13|12      11|10      9|  8  |   7  |  6  |   5  |   4  |  3  |   2  |  1  |   0  |    
           |------|---------|---------|----------|---------|-----|------|-----|------|------|-----|------|-----|------|    
           | MPRV | XS[1:0] | FS[1:0] | MPP[1:0] | VS[1:0] | SPP | MPIE | UBE | SPIE | WPRI | MIE | WPRI | SIE | WPRI |    
        --------------------------------------------------------------------------------------------------------------|    

Reference: [mstatus](https://docs.amd.com/r/en-US/ug1629-microblaze-v-user-guide/Machine-Status-Register-mstatus)

### How to get the total number of lines of code for all .c and .h and files in the nemu directory?

```shell
cd $NEMU_HOME
find -name "*.c" -o -name "*.h" | xargs wc -l

# If you want to ignore all blank lines, you can run the following command.
find -name "*.c" -o -name "*.h" | xargs grep -v '^\s*$' | wc -l
```

### How many lines of code did you write in PA1 compared to the framework code?

```shell
git diff --stat pa0..pa1
```

### What do -Wall and -Werror do in gcc?

-Wall: turn on **common warnings** to help developers find potential problems.

-Werror: **treat all warnings as errors** and force developers to fix them to ensure code quality.

### Issue

* 
* **cmd_d**: I don't actually return the watchpoints to free_, and I just left it **unused**.
  * I have set it unused when cmd_d.
* **info w**: I output watchpoints in **reverse order**, which doesn't really fit gdb in terms of output style.
* **trace_and_difftest()**: Although I implemented a cyclic scan of watchpoints, I did **not create the macro CONFIG_WATCHPOINT**. I just implemented it as a code definition. I also did not add a switch option in nemu/Kconfig for the watchpoint.
  ```c
  // ics2024/nemu/src/cpu/cpu-exec.c
  /* Scan watchpoints, here we can manually edit CONFIG_WATCHPOINT to open the configuration. */
  static bool CONFIG_WATCHPOINT = false;
  static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
    ...
    /* Scan watchpoints. */
    if (CONFIG_WATCHPOINT) {
      if (scan_wp(_this->pc)) {
        nemu_state.state = NEMU_STOP;
      }
    }
  }
  ```

