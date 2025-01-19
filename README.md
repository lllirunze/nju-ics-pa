# ICS2024 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2024/

To initialize, run
```bash
bash init.sh <subproject-name>
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

## Issues:

* **PA1**
  * [x] cmd_d: I don't actually return the watchpoints to `free_`, and I just left it unused.
  * [x] info w: I output watchpoints in reverse order, which doesn't really fit gdb in terms of output style
  * [x] `trace_and_difftest()`: Although I implemented a cyclic scan of watchpoints, I did not create the macro `CONFIG_WATCHPOINT`
* **PA2**
  * [ ] RISC-V instruction tests: I haven't used those test sets
    * [x] [riscv-tests-am](https://github.com/NJU-ProjectN/riscv-tests-am)
    * [ ] [riscv-arch-test-am](https://github.com/NJU-ProjectN/riscv-arch-test-am)
  * [ ] Spike doesn't support unaligned accesses: Executing an unaligned access instruction in Spike will throw an exception and make PC jump to 0. However, NEMU doesn't implement such a feature, so if you let NEMU and Spike execute such an instruction at the same time, DiffTest will report an error.
  * [ ] Capture dead loop: When the program is trapped in a dead loop, let the user program pause, and output the corresponding prompt message.
  * [x] Implement stdio.h klib fully
  * [ ] Real Time Clock: I haven't implement `AM_TIMER_RTC`.
  * [ ] Detect multiple keys being pressed at the same time: I have an idea to define `static int key_states[128] = {0};` to store the key status.
  * [x] Audio (optional)
* **PA3**
  * [ ] Exception response mechanism in `DIFFTEST`: In riscv-32, we need to set `mstatus` as 0x1800
  * [x] Detect ISA type in ELF file
  * [x] `SYS_write`
  * [ ] Center the canvas in `NDL_DrawRect`
  * [ ] Implement `fixedpt fixedpt_fromfloat(void *p)` which can transfer floating parameter to `fixedpt` type
