#!/bin/bash

sudo apt-get update
sudo apt-get install gcc-9 g++-9 clang-tidy clang-format
update-alternatives --set gcc "/usr/bin/gcc-9"
update-alternatives --set g++ "/usr/bin/g++-9"
