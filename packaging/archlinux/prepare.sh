#!/bin/bash

set -eux

run () {
    ver=$(python ../../../shells/xonsh/setup.py --version)
    echo $ver
    sed -i "s/pkgver=x/pkgver=$ver/g" PKGBUILD
    cat PKGBUILD
    docker run -v $PWD:/w -w /tmp zasdfgbnm/archlinux-yaourt bash -c '
        cp -r /w .
        cd w
        makepkg --printsrcinfo
    ' | tee .SRCINFO
}

cd tcg-git
run
