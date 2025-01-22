## PA0 - The night before the world was born: configuration

### Add ssh key on github

```shell
$ ssh-keygen -t rsa -C "1927553272@qq.com"
# copy the content of ~/.ssh/id_rsa.pub into Github.
```

### Why do I get an error when I run 'make menuconfig' in the terminal?

We are missing the bison and flex libraries, so we run the following commands.

```shell
$ sudo apt-get install bison
$ sudo apt-get install flex
```