# PA0 代码进度

- [x] 创建 PA0 开发分支 `pa0`。
- [x] 获取 NEMU 源码：`nemu/` 使用课程初始化脚本指定的 `ics2024` 分支。
- [x] 确认 NEMU 默认目标 ISA 为 `riscv32`。
- [x] 初始化 Abstract-Machine 源码：`abstract-machine/` 使用 `ics2024` 分支。
- [x] 设置 `NEMU_HOME` 与 `AM_HOME`：已写入 `~/.bashrc`，指向当前工作区中的两个组件。
- [x] 生成 NEMU 配置：使用 Kconfig 默认项，确认 ISA 为 `riscv32`、执行引擎为解释器、构建目标为宿主机原生 ELF。
- [x] 编译 NEMU：生成 `nemu/build/riscv32-nemu-interpreter`。
- [x] 运行 NEMU：`make run` 已执行，并按 PA0 讲义预期在 `welcome()` 的 `assert(0)` 处终止。
- [x] 验证 GDB：GDB 已成功加载 NEMU 可执行文件；交互式调试可通过 `make gdb` 启动。
