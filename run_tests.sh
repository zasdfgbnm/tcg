#!/bin/bash

set -eux

export PATH=$PATH:$PWD/build
ls -lah /sys/fs/cgroup/user.slice/
ls -lah /sys/fs/cgroup/user.slice/user-1000.slice/
ls -lah /sys/fs/cgroup/user.slice/user-1000.slice/
ls -lah /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service
ls -lah /sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/terminals.slice
mkdir -p build
cd build
cmake ..
make -j
cd ..
py.test --json-report --json-report-file=test_result.json --timeout=60 -v
