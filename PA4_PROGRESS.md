# PA4：虚实交错的魔法——分时多任务

目标架构：RISC-V32。

## 阶段 1：多道程序

- [x] 实现 CTE `kcontext()`，构造可运行的内核线程上下文。
- [x] 让陷阱处理按 `__am_irq_handle()` 的返回 `Context *` 恢复上下文。
- [x] 按 RISC-V32 ABI 向内核线程入口传递 `arg`。
- [x] 在 `yield-os` 中验证两个内核线程交替运行。
- [x] 在 Nanos-lite 中实现 `context_kload()`、PCB 和轮转调度。
- [x] 实现 VME `ucontext()`、Nanos-lite `context_uload()` 和 Navy `_start` 的用户栈切换。
- [x] 在串口、键盘和帧缓冲设备访问时主动让出 CPU，运行 PAL 与 hello 内核线程。
- [x] 为用户栈分配独立页，传递 `argc`、`argv`、`envp` 及其字符串。
- [x] 实现带参数的 `execve()`，并将 `exec-test` 加入 ramdisk 验证程序替换。
- [x] 支持 `pal --skip`，跳过商标和开场动画。
- [ ] 通过 NTerm 运行 Busybox 的 `cat`、`printenv`、`wc` 等命令。

## 阶段 2：虚存管理

- [ ] 阅读并按 RISC-V Sv32 机制实现 NEMU 地址转换。
- [ ] 实现 AM 的页分配、地址空间保护、`map()`、`protect()`、`unprotect()` 和 `ucontext()`。
- [ ] 实现 Nanos-lite `pg_alloc()`，在分页机制上运行内核。
- [ ] 让用户进程在独立地址空间中运行。
- [ ] 实现 `mm_brk()`，让仙剑奇侠传在分页机制上运行。
- [ ] 让仙剑奇侠传与 hello 内核线程在分页机制上多道运行。

## 阶段 3：抢占式分时多任务

- [ ] 通过时钟中断实现抢占式调度。
- [ ] 在 RISC-V32 CTE 中实现内核栈和用户栈之间的切换。
- [ ] 验证仙剑奇侠传和 hello 以内核时钟中断分时运行。
- [ ] 完成课程要求的实验报告必答题。

## 展示与选做代码任务

- [ ] 添加前台程序切换功能，展示多任务系统。
- [ ] 在独立分支尝试 ONScripter：补全 SDL_mixer 的 BGM、音效、混声及音频格式转换。
- [ ] 在独立分支实现 AM native 与 Nanos-lite 的磁盘抽象和块读写。
