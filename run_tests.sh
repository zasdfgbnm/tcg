#!/bin/bash

export PATH=$PATH:$PWD/build
mkdir -p build
cd build
cmake ..
make -j
cd ..
py.test -v
