#!/bin/bash

sudo echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" >> /etc/apt/source.list
sudo apt-get update
sudo apt-get install gcc-9 g++-9 clang-tidy clang-format
sudo apt-get remove gcc-5 g++-5
