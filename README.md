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

## Preface

I have written an adaptation of ics-pa for Tianjin University once in 2020, writing the x86 version. 
But for some well known reasons, my experiment was actually no different from the copy, and that experiment only went up to pa3 at that time. 
So I would like to complete an ics experiment in its entirety for the original version from Nanjing University to give me a consolation for [天津大学-计算机系统综合实践](https://github.com/superpung/TJU-CourseSharing/tree/main/2440072_%E8%AE%A1%E7%AE%97%E6%9C%BA%E7%B3%BB%E7%BB%9F%E7%BB%BC%E5%90%88%E5%AE%9E%E8%B7%B5).

## Blog

- [PA0](https://lllirunze.cn/2025/01/27/NJU-ICS-PA0/)
- [PA1-1](https://lllirunze.cn/2025/01/27/NJU-ICS-PA1-1/)
- [PA1-2]

## Issues:

Most of the issues here are optional tasks or little bugs that don't affect the required questions too much.

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
  * [ ] `printf("%p")` may return negative values.
* **PA3**
  * [ ] Exception response mechanism in `DIFFTEST`: In riscv-32, we need to set `mstatus` as 0x1800
  * [x] Detect ISA type in ELF file
  * [ ] Center the canvas in `NDL_DrawRect`
  * [ ] Implement `fixedpt fixedpt_fromfloat(void *p)` which can transfer floating parameter to `fixedpt` type
  * [ ] Built-in `echo` command in NTerm
  * [ ] Implement PAL
    * [x] Get the configuration [data](https://blog.csdn.net/weixin_63603830/article/details/134065932) in order to run PAL.
    * [ ] There are some game operations that need to be implemented in order to control the role.
  * [ ] Implement other apps in Navy
    * [ ] am-kernels
    * [ ] FCEUX
    * [ ] os-lab0
  * [ ] Audio: NJU Player
    * [ ] PAL with soundscape
    * [ ] Flappy Bird with soundscape
  * [ ] Free-switching DiffTest mode
  * [ ] Implement snapshots (save and load)
  * [ ] Add startup music
* **PA4**
  * [ ] DiffTest for Context: set `mstatus` as 0x1800
  * [ ] [RT-Thread](https://nju-projectn.github.io/ics-pa-gitbook/ics2024/4.1.html#rt-thread%E9%80%89%E5%81%9A) (optional)
  * [ ] Implement context switching in Nanos-lite(native)
