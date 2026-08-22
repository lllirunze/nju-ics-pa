# 多架构完善进度

- [x] 为 mips32 NEMU 增加独立 native defconfig，不改变默认 riscv32 配置。
- [x] 实现 mips32 基础整数运算、逻辑运算、移位、比较、跳转与分支延迟槽。
- [x] 实现 mips32 字节、半字与字的访存，以及 HI/LO 乘除法路径。
- [x] 实现 mips32 CP0 基础访问、`syscall`/`break` 异常入口和外设中断查询。
- [x] 实现 mips32 寄存器显示、寄存器表达式读取与基础差分寄存器比对。
- [x] 实现 mips32 CP0 TLB 寄存器、TLB 读写/探测指令、成对页翻译和启动期 TLB 自检。
- [ ] 实现 mips32 可恢复的 TLB/地址异常、完整特权态语义和用户态保护。
- [ ] 为 mips32 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
- [x] 为 x86 NEMU 增加独立 native defconfig，并修正 8/16/32 位寄存器的共享存储布局。
- [x] 实现 x86 基础算术/逻辑、条件码、栈、调用返回、跳转分支和常用双字节扩展指令。
- [x] 实现 x86 基础 IDT 中断入口、寄存器调试接口和基础差分寄存器比对。
- [x] 实现 x86 两级分页、`CR0/CR2/CR3`、Accessed/Dirty 位更新和启动期页表自检。
- [ ] 实现 x86 分段、可恢复的页故障处理和其余特权指令。
- [ ] 为 x86 补齐 Abstract Machine、Nanos-lite 与 Navy-apps 的可运行链路。
