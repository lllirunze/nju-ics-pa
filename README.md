# ICS2024 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2024/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)


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

* ~~cmd_d: I don't actually return the watchpoints to free_, and I just left it **unused**.~~
  * I have recycled watchpoints when cmd_d.
  * But I haven't sorted free_ yet.
* ~~info w: I output watchpoints in reverse order, which doesn't really fit gdb in terms of output style.~~
  * I have sorted the display information in set_wp.
* ~~trace_and_difftest(): Although I implemented a cyclic scan of watchpoints, I did not create the macro CONFIG_WATCHPOINT.~~
  * I have created the macro CONFIG_WATCHPOINT..

## PA2 - Simple complex machines: Von Neumann computer systems

### Where is "Hello World!" in the string table?

Write a "Hello-World" program under Linux, compile it and find the string table of the ELF file by the above method, where do you find the "Hello World!" string in the string table? Why is it there?

After writing hello.c:
```c
#include <stdio.h>

int main() {
  printf("Hello World!\n");
  return 0;
}
```

Run the following commands and we can get the result:

```shell
gcc -g -o hello hello.c
readelf -x .rodata hello

# result
Hex dump of section '.rodata':
  0x00002000 01000200 48656c6c 6f20576f 726c6421 ....Hello World!
  0x00002010 00  
```

In C, string constants (such as “Hello, World!”) are stored as read-only global data. The compiler puts string constants into .rodata sections to save memory and to ensure that they are not modifiable (read-only property). 

.strtab is specialized for storing symbol-related names (e.g., function names, variable names). "Hello, World!" is a string constant not directly related to a symbol, so it is not stored in .strtab.

### Duplicated symbol table

在Linux下编写一个Hello World程序, 然后使用strip命令丢弃可执行文件中的符号表:
```shell
gcc -o hello hello.c
strip -s hello
```

用readelf查看hello的信息:
```shell
readelf -a hello
```

你会发现符号表被丢弃了, 此时的hello程序能成功运行吗?
Answer: Yes

目标文件中也有符号表, 我们同样可以丢弃它:
```shell
gcc -c hello.c
strip -s hello.o
```

用readelf查看hello.o的信息, 你会发现符号表被丢弃了. 尝试对hello.o进行链接:
```shell
gcc -o hello hello.o

# result
/usr/bin/ld: error in hello.o(.eh_frame); no .eh_frame_hdr table will be created
/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/11/../../../x86_64-linux-gnu/Scrt1.o: in function `_start':
(.text+0x1b): undefined reference to `main'
collect2: error: ld returned 1 exit status
```

你发现了什么问题? 尝试对比上述两种情况, 并分析其中的原因.

### Let's see how fast NEMU goes.

* RISC-V 32
  * dhrystone:     119 Marks
  * coremark:      319 Marks
  * microbench:    397 Marks
  *         vs. 100000 Marks (i9-9900K @ 3.60GHz)
* Native
  * dhrystone:   58726 Marks
  * coremark:    48690 Marks
  * microbench:  48162 Marks
  *         vs. 100000 Marks (i9-9900K @ 3.60GHz)

### Issue

* RISC-V instruction tests: I haven't used those test sets
  * ~~[riscv-tests-am](https://github.com/NJU-ProjectN/riscv-tests-am)~~
  * [riscv-arch-test-am](https://github.com/NJU-ProjectN/riscv-arch-test-am)
* Spike doesn't support unaligned accesses
  * Executing an unaligned access instruction in Spike will throw an exception and make PC jump to 0. However, NEMU doesn't implement such a feature, so if you let NEMU and Spike execute such an instruction at the same time, DiffTest will report an error.
* Capture dead loop
  * When the program is trapped in a dead loop, let the user program pause, and output the corresponding prompt message.
* Implement stdio.h klib
  * I haven't implement stdio.h function fully.
* Detect multiple keys being pressed at the same time
  * I have an idea to define `static int key_states[128] = {0};` to store the key status.
