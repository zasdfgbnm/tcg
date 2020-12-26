#!/bin/bash

export PATH=$PATH:$PWD/build
export TCG_LOG_LEVEL=debug

mkdir -p build
cd build
cmake ..
make -j
cd ..
py.test --json-report --json-report-file=test_result.json --timeout=60 -v
