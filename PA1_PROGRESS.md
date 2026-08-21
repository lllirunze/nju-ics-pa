# PA1 代码进度

- [x] 移除 `nemu/src/monitor/monitor.c` 中 PA0 的 `welcome()` 练习断言，使 NEMU 可以进入 `(nemu)` 监视器。
- [x] 实现 `si [N]`，支持默认单步与指定正整数步数执行。
- [x] 实现 `info r`，显示 RISC-V 32 个通用寄存器及 `pc`。
- [x] 实现 `x N EXPR`，从表达式给出的地址开始读取 `N` 个 4 字节字。
- [x] 实现 `p EXPR` 的算术表达式求值：十进制、十六进制、括号、`+`、`-`、`*`、`/`。
- [x] 实现表达式差分测试：`tools/gen-expr` 生成结果文件，NEMU 通过 `--expr-test=FILE` 校验表达式结果。
- [x] 扩展表达式：寄存器（`$pc`、`$x0`～`$x31`、ABI 名称）、`==`、`!=`、`&&` 与一元 `*` 解引用。
- [x] 实现监视点：`w EXPR`、`info w`、`d N`，在每条客户指令执行后检查表达式变化并停止 NEMU。
- [x] 在 `nemu/Kconfig` 中加入默认开启的 `CONFIG_WATCHPOINT` 开关。
- [x] 支持一元负号：`1 + -1`、`--1`、`-(-1)` 等表达式按 `uint32_t` 语义计算。
