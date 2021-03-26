#!/bin/bash

set -eux

ver=$(python ../../shells/xonsh/setup.py --version)
pkgname="tcg_$ver-1"

mkdir -p $pkgname/usr/bin
cp ../../build/tcg $pkgname/usr/bin

mkdir -p $pkgname/DEBIAN
cp control $pkgname/DEBIAN
