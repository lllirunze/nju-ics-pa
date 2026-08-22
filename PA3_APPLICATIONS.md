# PA3：精彩纷呈的应用程序

本文件对应课程文档 `docs/3.5.html` 的“精彩纷呈的应用程序”章节，只记录代码功能、可执行的手动测试和外部资源依赖；不包含课程思考题。

## 通用构建与启动

在项目根目录执行以下命令，确保 ramdisk 和 Nanos-lite 镜像使用的是当前应用与资源：

```bash
NEMU_HOME=$PWD/nemu AM_HOME=$PWD/abstract-machine NAVY_HOME=$PWD/navy-apps \
  make -C nanos-lite ARCH=riscv32-nemu update
NEMU_HOME=$PWD/nemu AM_HOME=$PWD/abstract-machine NAVY_HOME=$PWD/navy-apps \
  make -C nanos-lite ARCH=riscv32-nemu
cd nemu
./build/riscv32-nemu-interpreter -b ../nanos-lite/build/nanos-lite-riscv32-nemu.bin
```

NEMU 启动并进入 NTerm 后，可直接输入下列各项给出的程序名。图形程序需要在 NEMU 打开的窗口中操作；关闭窗口或按程序自身的退出键后返回 NTerm。

## NSlider

- [x] miniSDL 已实现 `SDL_BlitSurface()`、`SDL_UpdateRect()`、`SDL_WaitEvent()`、`SDL_PollEvent()`、`SDL_GetTicks()` 和 `SDL_FillRect()`，满足 NSlider 的代码接口。
- [x] 已将用户提供的 25 页 16:9 PDF 等比置入 4:3 画布，生成 25 张 400×300 BMP，设定 `N = 25`，并将 NSlider 加入 ramdisk 构建。
  - 原始 PDF 保存在 `navy-apps/apps/nslider/slides/slides-original-16x9.pdf`；转换后的 4:3 PDF 为同目录的 `slides.pdf`。两份 PDF 已显式纳入 Git；生成的 BMP 仍被忽略并可由转换脚本再生。
- 手动验收：在 NTerm 输入 `nslider`，应显示第一页；按 NSlider 源码规定的翻页键，画面应切换到相邻页面。

## MENU 与 NTerm

- [x] MENU 已加入 ramdisk；miniSDL 的绘图与事件接口、Nanos-lite 的 `execve()` 均已实现。
- [x] NTerm 已加入 ramdisk，作为当前系统启动入口；内建 Shell 已支持命令解析、`PATH=/bin` 和 `execvp()`。
- [ ] 人工验证 MENU 的按键选项、程序启动和程序退出后返回入口。
  - 手动验收：在 NTerm 输入 `menu`，用窗口中的方向键选择一个条目并确认；所选程序退出后，应回到 NTerm。
- [ ] 人工验证 NTerm 的输入、退格、光标闪烁和外部程序启动。
  - 手动验收：在 NTerm 输入 `hello`、`file-test` 或 `cpp-test`。应看到程序输出，退出后重新出现 NTerm 提示符。
- [x] 已实现内建 Shell 的选做 `echo` 命令，支持输出命令后的文本和无参数的空行。
  - 手动验收：在 NTerm 输入 `echo hello, NEMU`，应输出 `hello, NEMU` 并返回提示符；输入 `echo` 时应只输出空行并返回提示符。

## Flappy Bird

- [x] Bird 源码、图像和音效资源已加入项目与 ramdisk；`IMG_Load()`、图像像素格式转换、8 位调色板、WAV 加载和 16 位饱和混音均已实现。
- [x] 已将 Bird 的目标屏幕高度适配为 NEMU 的 300 像素高度。
- [ ] 在 NEMU 图形窗口中人工验证画面、按键和音效。
  - 手动验收：在 NTerm 输入 `bird`。应出现游戏画面；按空格键开始/控制飞行，确认画面持续刷新。调高宿主机音量后，确认开始或碰撞时有音效；退出后应返回 NTerm。

## PAL（仙剑奇侠传）

- [x] PAL 移植源码已获取；miniSDL 的 8 位 Surface、调色板更新与最近邻缩放等图形依赖已实现。
- [x] 音频设备文件、NDL 音频接口和 miniSDL 音频回调/重入保护已实现，具备 PAL 音乐与音效的运行时基础。
- [x] 已从你本地官方安装内容的 `PAL_DOS/` 提取一套相互匹配的 DOS 数据，并补入中文 `desc.dat`；课程移植所需的 MKF、消息、字库与初始存档均在本地 `repo/data/`。该目录和根目录 `PAL/` 都被 Git 忽略，不会提交或推送。
- [x] 已在本地数据目录创建课程要求的 `sdlpal.cfg`（11,025 Hz、320×200），并将 PAL 加回默认 ramdisk 构建。
- [ ] 在 NEMU 中完成 PAL 的图像、按键、音乐和音效人工验证。
  - 手动验收：在 NTerm 输入 `pal`。应先出现启动动画和标题画面；进入游戏后检查静态场景、方向键/确认键响应、背景音乐和音效。

## Navy 上的 AM 应用

- [x] `libam` 已实现 TRM 输出/退出，以及计时器、键盘和 GPU 的 IOE 适配。
- [x] `coremark`、`dhrystone` 和 `typing-game` 已作为 RISC-V Navy 应用编译并加入 ramdisk。
- [x] 已启动 CoreMark，确认其输出和计时器链路能够工作。
- [ ] 人工运行 Dhrystone 与 typing-game。
  - 手动验收：在 NTerm 分别输入 `dhrystone` 和 `typing-game`。前者应输出基准测试结果；后者应显示图形界面，并能响应键盘输入。
- [x] 已将 microbench 接入 Navy ramdisk，并运行默认 `ref` 规模。Navy `libam` 中的 `Area heap` 未初始化，导致需要堆内存的九个子项显示 `Ignored (insufficient memory)`；无需堆内存的 queen 仍通过并输出分数。这是课程要求分析的运行受限原因。
  - 手动验收：在 NTerm 输入 `microbench`。默认 `ref` 规模约需十余秒；应看到 queen 通过，其余需要堆内存的项目被跳过，最后输出 `MicroBench PASS` 和很低的分数。

## FCEUX

- [x] FCEUX 已适配为 Navy 的标准 C 入口，`FCEUX_PATH` 已指向本项目的 `fceux-am`。
- [x] FCEUX 已纳入 RISC-V ramdisk；无 ROM 时会安全提示而非崩溃。
- [x] 已将你提供的 `thwaite.nes` 移到 `fceux-am/nes/rom/`，重新构建后已确认它被嵌入 FCEUX 二进制。
- [ ] 在 NEMU 图形窗口中人工启动并操作 Thwaite。
  - 手动验收：按本文件开头重新构建并启动后，在 NTerm 输入 `fceux`。应直接启动 Thwaite；检查画面和按键。ROM 目录被 `.gitignore` 忽略，不会被提交或推送。

## oslab0

- [x] 已获取 OSLab0 游戏集合、移除嵌套 Git 元数据，并将上游源码和 README 由本仓库跟踪。
- [x] 已将全部 14 个 OSLab0 游戏作为 RISC-V Navy 应用加入 ramdisk 默认构建，并完成逐个编译与安装验证。
- [ ] 人工运行 OSLab0 游戏并验证图形、键盘与退出行为。
  - 手动验收：在 NTerm 输入游戏编号，例如 `161220016`（跳一跳：空格蓄力、`R` 重开、`Q` 退出）、`171240502`（推箱子：方向键移动）或 `171860508`（俄罗斯方块：方向键/`WASD` 操作）。其余玩法见 `navy-apps/apps/oslab0/repo/<编号>/README.md`。

## NPlayer 与开机音乐

- [x] NPlayer、`little-star.ogg`、`/dev/sb`、`/dev/sbctl`、NDL 音频 API 和 miniSDL 音频 API 已加入 ramdisk 运行链路。
- [x] 已确认 NPlayer 能打开并开始播放 `/share/music/little-star.ogg`。
- [ ] 在 NEMU 中人工确认能听到 NPlayer 音乐并验证音量调节。
  - 手动验收：在 NTerm 输入 `nplayer`。应开始播放《小星星》并显示播放器画面；按 NPlayer 源码定义的音量键调节音量，确认宿主机音频输出存在变化。
- [x] 已实现课程选做的“进入 NTerm 时自动播放开机音乐”，复用 `little-star.ogg`，进入 NTerm 后后台播放最多 4 秒并自动释放音频资源。
  - 手动验收：重新构建并启动 NEMU；NTerm 出现时应听到一小段《小星星》，同时终端仍可输入命令。约 4 秒后音乐应停止；随后输入 `nplayer`，应仍可正常播放完整音乐。
