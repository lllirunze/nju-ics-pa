# PA1 代码进度

本清单记录 PA1 中直接涉及源码、配置、构建和功能验证的内容。实验报告、阅读材料和思考题不在此清单范围内；文档明确标为选做的代码任务也纳入实现范围。

## 必做功能

- [x] 移除 `nemu/src/monitor/monitor.c` 中 PA0 的 `welcome()` 练习断言，使 NEMU 可以进入 `(nemu)` 监视器。
- [x] 实现 `si [N]`，支持默认单步与指定正整数步数执行。
- [x] 实现 `info r`，显示 RISC-V 32 个通用寄存器及 `pc`。
- [x] 实现 `x N EXPR`，从表达式给出的地址开始读取 `N` 个 4 字节字。
- [x] 实现 `p EXPR` 的算术表达式求值：十进制、十六进制、括号、`+`、`-`、`*`、`/`。
- [x] 实现表达式差分测试：`tools/gen-expr` 生成结果文件，NEMU 通过 `--expr-test=FILE` 校验表达式结果。
- [x] 扩展表达式：寄存器（`$pc`、`$x0`～`$x31`、ABI 名称）、`==`、`!=`、`&&` 与一元 `*` 解引用。
- [x] 实现监视点：`w EXPR`、`info w`、`d N`，在每条客户指令执行后检查表达式变化并停止 NEMU。
- [x] 在 `nemu/Kconfig` 中加入默认开启的 `CONFIG_WATCHPOINT` 开关。

## 选做代码任务

- [x] 支持一元负号：`1 + -1`、`--1`、`-(-1)` 等表达式按 `uint32_t` 语义计算。

## 未完成的 PA1 代码任务

- [x] 无。PA1 文档中明确的必做和选做代码任务均已实现。

## 验证记录

- [x] 在 `nemu/` 目录执行 `make`，成功生成 `build/riscv32-nemu-interpreter`。
- [x] 手动验证寄存器、逻辑比较、内存解引用、监视点触发、监视点删除和编号复用。
- [x] 执行 `./tools/gen-expr/build/gen-expr 1000 > /tmp/expr-input`，再执行 `./build/riscv32-nemu-interpreter --expr-test=/tmp/expr-input`；输出 `Expression tests passed: 1000 cases`。

## 提交记录

- `df31367 implement basic debugger commands`
- `652c1dd add expression differential tests`
- `f6d27c3 implement watchpoints`
- `8996680 support unary negation in expressions`
