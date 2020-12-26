#!/bin/bash

export PATH=$PATH:$PWD/build
ls /sys/fs/cgroup/user.slice/user@1000.service
mkdir -p build
cd build
cmake ..
make -j
cd ..
py.test --json-report --json-report-file=test_result.json --timeout=60 -v
