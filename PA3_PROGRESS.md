# PA3 代码进度

- [x] 实现 RISC-V 32 异常响应、CSR 和 `ecall`/`mret` 指令。
- [x] 重新组织 RISC-V `Context` 并实现 CTE 的上下文保存与恢复接口。
- [x] 实现 `yield()` 的事件识别与返回。
- [x] 实现选做的异常轨迹 etrace。
- [x] 初始化 Nanos-lite 和 Navy-apps，并在 Nanos-lite 中处理 `EVENT_YIELD`。
- [x] 初始化并实现 Nanos-lite 的用户程序加载与系统调用。
- [x] 实现 `SYS_brk` 和用户态 `_sbrk()` 堆空间管理。
- [x] 实现 Nanos-lite 文件系统及设备文件。
- [x] 实现 NDL 的计时、按键事件、显示信息、framebuffer 和居中画布支持。
- [x] 实现 `SYS_execve` 并接通用户态 `execve()` 系统调用接口。
- [x] 将开机 Menu 纳入 ramdisk，并在程序退出后重新启动 Menu。
- [ ] 完成 miniSDL 的事件和显示支持，并通过 Menu 按键启动用户程序。
- [ ] 通过 NTerm 的内建 Shell 支持按命令运行用户程序。
