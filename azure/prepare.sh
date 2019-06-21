#!/bin/bash

sudo add-apt-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main"
wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update

sudo apt-get install gcc-9 g++-9
sudo apt-get remove gcc-5 g++-5

sudo apt-get install clang-8 clang-format-8 clang-tidy-8
