# tcg: cgroups for terminals

![sanity-check](https://github.com/zasdfgbnm/tcg/workflows/sanity-check/badge.svg)
![archlinux](https://github.com/zasdfgbnm/tcg/workflows/archlinux/badge.svg)
![deploy](https://github.com/zasdfgbnm/tcg/workflows/deploy/badge.svg)

Terminal cgroup, short for `tcg`, is a tool to help people manage system's resource usage through cgroup. To use `tcg`, you should first do the following:
1. [Prepare your system to use cgroup v2](https://wiki.archlinux.org/index.php/Cgroups#Switching_to_cgroups_v2)
2. Install `tcg`.
3. Add something like `echo Your cgroup is $(tcg create).` to your `.bashrc`, `.zshrc`, or `.xonshrc`, so that it runs everytime you open a new shell
4. Read the example use case below.

# Install

`tcg` is available on [Archlinux AUR](https://aur.archlinux.org/packages/tcg-git/), you can install from there.

If you use [Xonsh](https://xon.sh), we have a xontrib, see [shells/xonsh/README.md](shells/xonsh/README.md) for more information.

## Install from source

`tcg` requires boost, you need to install boost on your system first.


To clone `tcg` from github, run the following command:
```
git clone --recursive https://github.com/zasdfgbnm/tcg.git
```
Make sure you use the `--recursive` in your command line, so that submodules will be available when you build. If you have already cloned without `--recursive`, you can go to your repository and do
```
git submodule update --init --recursive
```


After you cloned `tcg`, go to `tcg`'s directory and simply do
```bash
mkdir build
cd build
cmake ..
make -j
```
You will get a binary called `tcg` in the `build` directory.

The last step, you need to make sure `tcg` is owned by `root` and has [`suid` permission](https://www.howtogeek.com/656646/how-to-use-suid-sgid-and-sticky-bits-on-linux/):
```
sudo chown root tcg
sudo chmod +s tcg
```

# Example use case

In my daily job, I often need to compile [PyTorch](https://github.com/pytorch/pytorch), which will take 100% of my CPU for a long time.

## Automatically create cgroup for each shell

Since I added `tcg create` to my `.xonshrc`, everytime when I open a shell, I will get a cgroup containing the shell together with all the commands I run in that shell. The name will be displayed on the shell:
```
gaoxiang@XiangdeMacBook-Pro ~ $ ssh xxxxxxxx
Last login: Mon Dec 28 19:05:50 2020 from 2600:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
Your cgroup is poincare.
gaoxiang@sunnyvale ~ $
```

The above example is a shell from ssh, and is assigned a cgroup named `poincare`.

## Freeze and unfreeze

I start to compile PyTorch in the shell, and the CPU usage of my system easily reaches 100%. Before that compilation job finishes, I decided that I want to benchmark something. To be accurate on my benchmark, I want to pause my compilation for a few minutes. To do so, I can easily do:
```
tcg freeze poincare
```

And when I am ready to put my compilation back to work, I just need to:
```
tcg unfreeze poincare
```
and the compilation will continue its work again.

## Limit maximum CPU usage

Letting the compilation job to use 100% of my CPU is not always a good idea. Because this sometimes makes my computer's response time slow. To solve this problem, I can simply do
```
tcg set poincare cpu.max "550000 10000"
```
to limit the CPU usage to about 90% on my computer. *Please note that the two numbers depend on the number of cores in your computer, so don't just copy and paste the number above!* The above `550000 10000` means, for every 10000 periods, the cgroup are allowed to use at most 550000 periods. My computer has 32 cores and 64 threads, so it can use at most 640000 for every 10000 periods. Limiting it to 550000 will allow 90000 spare periods for other tasks.

To relax this limit, simply do:
```
tcg set poincare cpu.max max
```
this will allow the compilation job to run at 100% CPU usage

For detailed information, please check [Linux kernel's cgroup v2 documentation](https://www.kernel.org/doc/Documentation/cgroup-v2.txt).

## Setting the priority of tasks

Let's say I have two compilation jobs, one at cgroup `poincare`, another at cgroup `pauli`. I want these two jobs togeter use as much CPU as possible, but I think `poincare` is 10x more important than `pauli`. To let `poincare` get 10x more CPU time than `pauli`, I can do:
```
tcg set pauli cpu.weight 100
tcg set poincare cpu.weight 1000
```
or even simpler:
```
tcg set poincare cpu.weight 1000
```
The `tcg set pauli cpu.weight 100` can be omitted because the default `cpu.weight` value for cgroups are 100.

Setting the `cpu.weight` value tells Linux kernel to control the number of CPU cycles taskes inside each cgroup when scheduling.

For detailed information, please check [Linux kernel's cgroup v2 documentation](https://www.kernel.org/doc/Documentation/cgroup-v2.txt).
