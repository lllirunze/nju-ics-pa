# ICS2025 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

The default target architecture for this workspace is **riscv32**. Follow the
relevant PA document for each component's exact ISA configuration and command.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2025/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.
* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

## 写在完成之后

我的本科就读于天津大学计算机科学与技术专业。大二上学期时，我上过一门名为《计算机系统综合实践》的编码课程。它的项目来自南京大学的 ICS，只是做了一些精简。因为疫情和种种现实原因，那时课程最终只推进到 PA2，随后便不了了之。

彼时的我，对计算机几乎一无所知。面对一层层展开的代码、陌生的指令集、内存、设备、操作系统和程序运行的细节，阅读已经十分吃力，更不要说亲手把它们写出来。ICS 像一台被拆开的机器：每一块零件似乎都能看懂一点，但当它们需要重新扣合、开始转动时，我常常不知道该从哪里下手。

那门课里，真正独自写出全部内容的同学并不多。许多人都曾沿着 GitHub 上前辈留下的足迹前进，也得到过学院同门的帮助。其中有些同学在初高中便已深入接触计算机专业，参加过 ACM 等信息学竞赛；而我还在努力辨认那些最基础的概念。很感谢那些愿意开源代码、分享思路的人，也感谢曾经愿意耐心帮助我的同学。正是这些看似微小的善意，让一个什么都不会的人也能跌跌撞撞地走过那段路。

但这份没有完成的项目，一直留在心里。它像一本读到一半便被合上的书：我知道后面还有更大的世界，也一直想知道，自己能不能有一天把它完整地走完。只是很多年里，面对后续越来越复杂的内容，我确实常常觉得能力有限；即使反复思考，也不知道下一行代码究竟该怎样写下去。

后来我已经就业，成为了一名程序员；但再回头面对这份完整的项目，仍然很难仅凭自己把它一步步写出来。系统软件里那些彼此咬合的细节，并不会因为职位名称而自动变得简单。这份迟疑也让我明白，曾经的挫败并不只是无知所致：计算机系统本就是一座需要长期攀登的山。

如今，借助 AI agent 这样的工具，我终于得以在几个小时里重新走过这条路：从 NEMU 的指令执行，到 Abstract Machine 的上下文和虚存；从 Nanos-lite 的调度，到 Navy 应用程序、磁盘与设备。它并不意味着那些概念突然变得轻飘飘，也不意味着曾经的困难不再真实。恰恰相反，正因为曾经被这些困难困住过，今天才能更清楚地感受到工具带来的改变：它可以陪人阅读、解释、尝试、排错，把遥远而庞大的工程拆成一次次可以抵达的小步。

感谢开源，感谢曾经帮助过我的同学，也感谢技术的发展。多年以后回头看，完成这份项目并不只是补上一门课程的作业；更像是和过去那个面对计算机系统手足无措的自己，认真地说了一声：这一次，我们把它走完了。
