#!/bin/bash

echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" | sudo tee /etc/apt/source.list
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update

sudo apt-get install gcc-9 g++-9
sudo apt-get remove gcc-5 g++-5

sudo apt-get install clang-8 lldb-8 lld-8 clang-format-8 clang-tidy-8

which clang-format
which clang-tidy
