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
- [x] 实现 miniSDL 的 Surface 填充、拷贝、framebuffer 更新和 8 位调色板显示。
- [x] 实现 miniSDL 的轮询、等待、事件队列和按键状态接口。
- [x] 实现 miniSDL 相对计时与延时接口。
- [x] 实现 SDL RWops 文件/内存流和 `IMG_Load()` 图像解码接口。
- [x] 支持不同 32 位像素格式之间的颜色转换，并添加图像解码回归测试。
- [x] 实现 `fixedptc` 的整数、乘除、绝对值及向上/下取整运算。
- [x] 实现 8 位 Surface 的最近邻缩放和局部调色板更新。
- [ ] 通过 Menu 按键启动用户程序，并验证程序退出后重新进入 Menu。
- [x] 将 NTerm 纳入 ramdisk，并将其设为系统启动和程序退出后的入口。
- [x] 实现 NTerm 内建 Shell 的命令解析、`PATH=/bin` 和 `execvp()` 启动接口。
- [x] 实现 NTerm 内建 Shell 的选做 `echo` 命令。
- [ ] 通过 NTerm 手动运行用户程序，并验证程序退出后重新进入 NTerm。
- [x] 获取 Flappy Bird 应用源码和资源，并将其加入 ramdisk。
- [ ] 在 NEMU 上运行 Flappy Bird 并验证图像与按键操作。
- [x] 获取 PAL 应用源码并完成其 8 位调色板图形依赖。
- [x] 已从用户本地的官方 PAL 安装内容中提取与课程移植匹配的 DOS 数据，加入本地 `repo/data/`；数据目录和根目录安装文件均被忽略，不会提交或推送。
- [ ] 在 NEMU 上人工运行 PAL，验证图像、按键、音乐和音效。
- [x] 实现 Nanos-lite 的 `/dev/sb` 与 `/dev/sbctl` 声卡设备文件。
- [x] 实现 NDL 声卡初始化、播放和空闲缓冲区查询接口。
- [x] 实现 miniSDL 音频回调轮询、暂停、关闭与重入保护。
- [x] 实现 miniSDL 的 PCM WAV 加载、释放和 16 位饱和混音接口。
- [x] 将 NPlayer 与 `little-star.ogg` 纳入 ramdisk 并完成启动验证。
- [ ] 手动在 NEMU 中聆听 NPlayer 音乐和 Flappy Bird 音效。
- [x] 在 Navy 启动代码中调用 `__libc_init_array()`，支持 C++ 全局对象构造。
- [x] 将 `cpp-test` 纳入 ramdisk，并验证构造函数先于 `main()` 执行。
- [x] 实现 Navy `libam` 的 TRM 输出/退出及计时器、键盘、GPU IOE 适配。
- [x] 将 coremark、dhrystone 和 typing-game 作为 RISC-V AM 应用纳入 ramdisk 构建。
- [x] 在 NEMU 中启动 CoreMark，验证 AM 程序的输出与计时器链路。
- [ ] 手动运行 typing-game 并验证 AM 键盘和图形输出。
- [x] 获取 FCEUX AM 兼容源码，并将其适配为 Navy 的标准 C 入口。
- [x] 将 FCEUX 作为 RISC-V Navy 应用纳入 ramdisk，并完成无 ROM 安全启动验证。
- [x] 将用户提供的 `thwaite.nes` 放入 `fceux-am/nes/rom/` 并重新构建 FCEUX。
- [ ] 手动在 NEMU 中启动并操作 FCEUX 的 Thwaite 游戏。
- [x] 获取 OSLab0 游戏集合并将全部 14 个游戏加入 RISC-V Navy ramdisk 构建。
- [ ] 手动运行 OSLab0 游戏并验证图形、键盘与退出行为。
- [x] 实现进入 NTerm 时自动播放短开机音乐的选做功能。
- [x] 实现 SDB 的 `detach` / `attach` 命令：可暂停 DiffTest，并将 RISC-V32 DUT 的完整物理内存和寄存器同步到 REF 后恢复比对。
- [x] 实现 SDB 的 `save` / `load` 快照命令，保存和恢复 RISC-V32 CPU 状态与完整物理内存，并校验快照格式和大小。
- [x] 将 microbench 接入 Navy ramdisk 并运行默认 `ref` 规模；Navy `libam` 未初始化 `Area heap`，故九个需要堆内存的子项被判定为内存不足并跳过，只有无需堆的 queen 可运行。

## 应用程序

### 通用构建与启动

```bash
NEMU_HOME=$PWD/nemu AM_HOME=$PWD/abstract-machine NAVY_HOME=$PWD/navy-apps \
  make -C nanos-lite ARCH=riscv32-nemu update
NEMU_HOME=$PWD/nemu AM_HOME=$PWD/abstract-machine NAVY_HOME=$PWD/navy-apps \
  make -C nanos-lite ARCH=riscv32-nemu
cd nemu
./build/riscv32-nemu-interpreter -b ../nanos-lite/build/nanos-lite-riscv32-nemu.bin
```

NEMU 启动并进入 NTerm 后，可直接输入下列程序名。图形程序需要在 NEMU 打开的窗口中操作；关闭窗口或按程序自身的退出键后返回 NTerm。

### NSlider

- [x] miniSDL 已实现 `SDL_BlitSurface()`、`SDL_UpdateRect()`、`SDL_WaitEvent()`、`SDL_PollEvent()`、`SDL_GetTicks()` 和 `SDL_FillRect()`，满足 NSlider 的代码接口。
- [x] 已将用户提供的 25 页 16:9 PDF 等比置入 4:3 画布，生成 25 张 400×300 BMP，设定 `N = 25`，并将 NSlider 加入 ramdisk 构建。
  - 原始 PDF 保存在 `navy-apps/apps/nslider/slides/slides-original-16x9.pdf`；转换后的 4:3 PDF 为同目录的 `slides.pdf`。两份 PDF 已显式纳入 Git；生成的 BMP 仍被忽略并可由转换脚本再生。
- [ ] 在 NTerm 输入 `nslider`，应显示第一页；按 NSlider 源码规定的翻页键，画面应切换到相邻页面。

### MENU 与 NTerm

- [x] MENU 已加入 ramdisk；miniSDL 的绘图与事件接口、Nanos-lite 的 `execve()` 均已实现。
- [x] NTerm 已加入 ramdisk，作为当前系统启动入口；内建 Shell 已支持命令解析、`PATH=/bin` 和 `execvp()`。
- [ ] 在 NTerm 输入 `menu`，用窗口中的方向键选择一个条目并确认；所选程序退出后，应回到 NTerm。
- [ ] 在 NTerm 输入 `hello`、`file-test` 或 `cpp-test`，应看到程序输出，退出后重新出现 NTerm 提示符。
- [x] 已实现内建 Shell 的选做 `echo` 命令，支持输出命令后的文本和无参数的空行。
  - 手动验收：`echo hello, NEMU` 应输出 `hello, NEMU`；`echo` 应只输出空行。

### Flappy Bird

- [x] Bird 源码、图像和音效资源已加入项目与 ramdisk；`IMG_Load()`、图像像素格式转换、8 位调色板、WAV 加载和 16 位饱和混音均已实现。
- [x] 已将 Bird 的目标屏幕高度适配为 NEMU 的 300 像素高度。
- [ ] 在 NTerm 输入 `bird`，应出现游戏画面；按空格键开始/控制飞行，检查持续刷新、音效和退出后的 NTerm 返回。

### PAL（仙剑奇侠传）

- [x] PAL 移植源码已获取；miniSDL 的 8 位 Surface、调色板更新与最近邻缩放等图形依赖已实现。
- [x] 音频设备文件、NDL 音频接口和 miniSDL 音频回调/重入保护已实现，具备 PAL 音乐与音效的运行时基础。
- [x] 已从本地官方安装内容提取与课程移植匹配的 DOS 数据，并补入中文 `desc.dat`；课程移植所需的 MKF、消息、字库与初始存档均在本地 `repo/data/`。该目录和根目录 `PAL/` 都被 Git 忽略，不会提交或推送。
- [x] 已在本地数据目录创建课程要求的 `sdlpal.cfg`（11,025 Hz、320×200），并将 PAL 加回默认 ramdisk 构建。
- [ ] 在 NTerm 输入 `pal`，应出现启动动画和标题画面；进入游戏后检查静态场景、方向键/确认键响应、背景音乐和音效。

### Navy 上的 AM 应用

- [x] `libam` 已实现 TRM 输出/退出，以及计时器、键盘和 GPU 的 IOE 适配。
- [x] `coremark`、`dhrystone` 和 `typing-game` 已作为 RISC-V Navy 应用编译并加入 ramdisk。
- [x] 已启动 CoreMark，确认其输出和计时器链路能够工作。
- [ ] 在 NTerm 分别输入 `dhrystone` 和 `typing-game`；前者应输出基准测试结果，后者应显示图形界面并响应键盘输入。
- [x] 已将 microbench 接入 Navy ramdisk，并运行默认 `ref` 规模。Navy `libam` 中的 `Area heap` 未初始化，导致需要堆内存的九个子项显示 `Ignored (insufficient memory)`；无需堆内存的 queen 仍通过并输出分数。
  - 手动验收：`microbench` 默认规模约需十余秒；应输出 `MicroBench PASS` 和很低的分数。

### FCEUX

- [x] FCEUX 已适配为 Navy 的标准 C 入口，`FCEUX_PATH` 已指向本项目的 `fceux-am`。
- [x] FCEUX 已纳入 RISC-V ramdisk；无 ROM 时会安全提示而非崩溃。
- [x] 已将提供的 `thwaite.nes` 移到 `fceux-am/nes/rom/`，重新构建后已确认它被嵌入 FCEUX 二进制。
- [ ] 在 NTerm 输入 `fceux`，应直接启动 Thwaite；检查画面和按键。ROM 目录被 `.gitignore` 忽略，不会被提交或推送。

### OSLab0

- [x] 已获取 OSLab0 游戏集合、移除嵌套 Git 元数据，并将上游源码和 README 由本仓库跟踪。
- [x] 已将全部 14 个 OSLab0 游戏作为 RISC-V Navy 应用加入 ramdisk 默认构建，并完成逐个编译与安装验证。
- [ ] 在 NTerm 输入游戏编号，例如 `161220016`（跳一跳：空格蓄力、`R` 重开、`Q` 退出）或 `171240502`（推箱子：方向键移动），检查图形、键盘与退出行为。

### NPlayer 与开机音乐

- [x] NPlayer、`little-star.ogg`、`/dev/sb`、`/dev/sbctl`、NDL 音频 API 和 miniSDL 音频 API 已加入 ramdisk 运行链路。
- [x] 已确认 NPlayer 能打开并开始播放 `/share/music/little-star.ogg`。
- [ ] 在 NTerm 输入 `nplayer`，应开始播放《小星星》并显示播放器画面；按源码定义的音量键调节音量，确认宿主机音频输出存在变化。
- [x] 已实现课程选做的“进入 NTerm 时自动播放开机音乐”，复用 `little-star.ogg`，进入 NTerm 后后台播放最多 4 秒并自动释放音频资源。
  - 手动验收：重新构建并启动 NEMU；NTerm 出现时应听到一小段《小星星》，约 4 秒后停止；随后 `nplayer` 仍可正常播放完整音乐。
