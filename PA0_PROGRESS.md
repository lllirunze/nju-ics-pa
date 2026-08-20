# PA0 代码进度

本清单只记录 PA0 中与项目源码、配置、构建和运行直接相关的内容。GNU/Linux 使用练习、工具安装、Git 使用练习和实验报告不在此清单范围内。

- [x] 创建 PA0 开发分支 `pa0`。
- [x] 获取 NEMU 源码：`nemu/` 使用课程初始化脚本指定的 `ics2024` 分支。
- [x] 确认 NEMU 默认目标 ISA 为 `riscv32`。
- [x] 初始化 Abstract-Machine 源码：`abstract-machine/` 使用 `ics2024` 分支。
- [x] 设置 `NEMU_HOME` 与 `AM_HOME`：已写入 `~/.bashrc`，指向当前工作区中的两个组件。
- [x] 生成 NEMU 配置：使用 Kconfig 默认项，确认 ISA 为 `riscv32`、执行引擎为解释器、构建目标为宿主机原生 ELF。
- [x] 编译 NEMU：生成 `nemu/build/riscv32-nemu-interpreter`。
- [x] 运行 NEMU：`make run` 已执行，并按 PA0 讲义预期在 `welcome()` 的 `assert(0)` 处终止。
- [x] 验证 GDB：GDB 已成功加载 NEMU 可执行文件；交互式调试可通过 `make gdb` 启动。

## PA1 待办（不计入 PA0）

- [ ] 移除 `nemu/src/monitor/monitor.c` 中 `welcome()` 的练习日志和 `assert(0)`，重新构建并验证 NEMU 可以进入监视器。讲义要求在 PA0 首次运行看到该断言后暂时忽略，PA1 再修复。

## 验证记录

安装构建依赖后，Kconfig 默认配置已确认选择 `riscv32`、解释器和 Native ELF。构建过程额外拉取并编译了 NEMU 所需的 Capstone 依赖。首次 `make run` 的断言是 PA0 规定的预期结果；修复它属于 PA1。

每次新开交互式终端后可执行 `source ~/.bashrc`，或重新登录，使环境变量生效。
