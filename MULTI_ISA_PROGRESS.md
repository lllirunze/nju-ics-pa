# 多架构完善进度

- [x] 为 mips32 NEMU 增加独立 native defconfig，不改变默认 riscv32 配置。
- [x] 实现 mips32 基础整数运算、逻辑运算、移位、比较、跳转与分支延迟槽。
- [x] 实现 mips32 字节、半字与字的访存，以及 HI/LO 乘除法路径。
- [x] 实现 mips32 CP0 基础访问、`syscall`/`break` 异常入口和外设中断查询。
- [x] 实现 mips32 寄存器显示、寄存器表达式读取与基础差分寄存器比对。
- [ ] 实现 mips32 TLB/MMU、地址异常和完整的特权态语义。
- [ ] 为 mips32 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
- [ ] 补齐 x86 NEMU 的 ISA 任务与可复现的独立构建配置。
- [ ] 为 x86 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
