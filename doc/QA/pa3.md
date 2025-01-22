## PA3 - Journey through time: batch system

### Unable to convert PDF

```bash
ubuntu@ubuntu-vm:~/Desktop/ics2024/navy-apps/apps/nslider/slides$ ./convert.sh
convert-im6.q16: attempt to perform an operation not allowed by the security policy `PDF' @ error/constitute.c/IsCoderAuthorized/426.
convert-im6.q16: no images defined `slides.bmp' @ error/convert.c/ConvertImageCommand/3229.
rm: cannot remove '/home/ubuntu/Desktop/ics2024/navy-apps/fsimg/share/slides/*': No such file or directory
mv: cannot stat '*.bmp': No such file or directory
```

Run this command:

```bash
sudo vim /etc/ImageMagick-6/policy.xml 

# Find this row: <policy domain="coder" rights="none" pattern="PDF" />
# Modify this row: <policy domain="coder" rights="read | write" pattern="PDF" />
```

After run `./convert.sh`, we may find that issue: `rm: cannot remove '/home/ubuntu/Desktop/ics2024/navy-apps/fsimg/share/slides/*': No such file or directory`. This issue doesn't matter. Just ignore it.

### How to run Flappy Bird in Navy?

First of all, we need to create a symbolic link.

```make
# navy-apps/apps/bird/Makefile
install-file:
	ln -sf -T $(abspath $(REPO_PATH)/res) $(NAVY_HOME)/fsimg/share/games/bird
```

Because in `navy-apps/apps/bird/repo/include/BirdGame.h`, there are some code:

```c
#ifdef __NAVY__
#define RES_PREFIX "/share/games/bird/"
#else
#define RES_PREFIX "res/"
#endif
```

It means that if we use Navy (native or riscv32) to run Flappy Bird, this game will use the images in "/share/games/bird/" instead of "res/", which is run in Linux native.

After running `make ISA=native install-file`, we can play this game using `make ISA=native run`.

### How to get the data of PAL?

[link](https://blog.csdn.net/weixin_63603830/article/details/134065932)
