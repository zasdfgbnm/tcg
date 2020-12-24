#!/bin/bash

export PATH=$PATH:$PWD/build
cd build
cmake ..
make -j
cd ..
py.test -v
