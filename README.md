# tcg: cgroups for terminals

![sanity-check](https://github.com/zasdfgbnm/tcg/workflows/sanity-check/badge.svg)
![archlinux](https://github.com/zasdfgbnm/tcg/workflows/archlinux/badge.svg)

Terminal cgroup, short for `tcg`, is a tool to help people manage system's resource usage through cgroup. To use `tcg`, you should first do the following:
1. [Prepare your system to use cgroup v2](https://wiki.archlinux.org/index.php/Cgroups#Switching_to_cgroups_v2)
2. Install `tcg`.
3. Add `tcg create` to your `.bashrc`, `.zshrc`, or `.xonshrc`, so that it runs everytime you open a new shell
4. Read the example use case below.

# Install

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


