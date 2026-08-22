# 多架构完善进度

- [x] 为 mips32 NEMU 增加独立 native defconfig，不改变默认 riscv32 配置。
- [x] 实现 mips32 基础整数运算、逻辑运算、移位、比较、跳转与分支延迟槽。
- [x] 实现 mips32 字节、半字与字的访存，以及 HI/LO 乘除法路径。
- [x] 实现 mips32 CP0 基础访问、`syscall`/`break` 异常入口和外设中断查询。
- [x] 实现 mips32 寄存器显示、寄存器表达式读取与基础差分寄存器比对。
- [ ] 实现 mips32 TLB/MMU、地址异常和完整的特权态语义。
- [ ] 为 mips32 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
- [x] 为 x86 NEMU 增加独立 native defconfig，并修正 8/16/32 位寄存器的共享存储布局。
- [x] 实现 x86 基础算术/逻辑、条件码、栈、调用返回、跳转分支和常用双字节扩展指令。
- [x] 实现 x86 基础 IDT 中断入口、寄存器调试接口和基础差分寄存器比对。
- [ ] 实现 x86 分页、分段、完整异常模型和其余特权指令。
- [ ] 为 x86 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
