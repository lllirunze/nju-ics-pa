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
- [x] 通过 NTerm 运行 Busybox 的 `cat`、`printenv`、`wc` 等命令。

## 阶段 2：虚存管理

- [x] 阅读并按 RISC-V Sv32 机制实现 NEMU 地址转换。
- [x] 实现 AM 的页分配、地址空间保护、`map()`、`protect()`、`unprotect()` 和 `ucontext()`。
- [x] 实现 Nanos-lite `pg_alloc()`，在分页机制上运行内核。
- [x] 让用户进程在独立地址空间中运行。
- [x] 实现 `mm_brk()`，让仙剑奇侠传在分页机制上运行。
- [x] 让仙剑奇侠传与 hello 内核线程在分页机制上多道运行。

## 阶段 3：抢占式分时多任务

- [x] 通过时钟中断实现抢占式调度。
- [x] 在 RISC-V32 CTE 中实现内核栈和用户栈之间的切换。
- [x] 配置 hello 与 NTerm 为两个独立用户进程，由时钟中断轮转调度。
- [ ] 在 NTerm 中启动仙剑奇侠传，人工验证其与 hello 以内核时钟中断分时运行。

## 实现说明与思考题

- [x] 时钟抢占链路：NEMU 的定时器置位 `cpu.INTR`；每条指令结束后，CPU 仅在 `mstatus.MIE=1` 时接受该请求并以 `mcause=0x80000007` 陷入。CTE 将其封装为 `EVENT_IRQ_TIMER`，Nanos-lite 调用 `schedule()` 返回下一进程的上下文。
- [x] RISC-V 中断状态：`isa_raise_intr()` 将旧 `MIE` 保存到 `MPIE` 并关闭 `MIE`，以避免 CTE 重入；`mret` 将 `MPIE` 恢复到 `MIE`，并将 `MPIE` 置 1。因此新建的内核和用户上下文都设置 `mstatus=0x1880`，使第一次 `mret` 后可响应时钟中断。
- [x] 内核栈的必要性：用户进程可任意修改 `sp`，也可能接近用户栈边界；在该栈上保存陷阱上下文既不安全，也不能保证另一用户地址空间可访问。陷入时必须先转入内核拥有且被所有地址空间共享映射的 PCB 内核栈。
- [x] RISC-V32 栈切换：`mscratch` 在运行用户进程时保存其内核栈指针、运行内核代码时为 0。陷入入口通过 `csrrw sp, mscratch, sp` 原子交换栈指针；`Context.np` 记录返回目标是否为用户进程。恢复上下文前，若 `np=1`，将当前内核栈指针写回 `mscratch`，最后从 `Context.gpr[2]` 恢复用户栈指针。
- [x] 地址空间切换的循环依赖：若上下文位于用户栈，进程 A 无法访问进程 B 用户栈中的 `pdir`，但切到 B 的地址空间又需要该 `pdir`。现在上下文保存在所有地址空间均可访问的内核栈，`__am_switch()` 可在切换页表前安全读取目标 `Context.pdir`，从而消除循环依赖。
- [x] CTE 重入处理：陷入后将 `mscratch` 清零，表示随后嵌套陷入来自内核栈；同时硬件模型在进入中断后关闭 `MIE`。因此 PA 的时钟中断不会在 CTE 执行过程中再次抢占并覆盖当前上下文。

## 展示与选做代码任务

- [x] 添加前台程序切换功能：F1 选择 hello，F2 选择 NTerm/PAL；键盘事件经内核队列只交付给前台进程。
- [ ] 在独立分支尝试 ONScripter：补全 SDL_mixer 的 BGM、音效、混声及音频格式转换。
- [ ] 在独立分支实现 AM native 与 Nanos-lite 的磁盘抽象和块读写。
