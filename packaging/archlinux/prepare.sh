#!/bin/bash

set -eux

run () {
    set -eux
    ver=$(python ../../../shells/xonsh/setup.py --version)
    commit=$(git rev-parse HEAD)
    echo $ver
    sed -i "s/pkgver=x/pkgver=$ver/g" PKGBUILD
    sed -i "s/commit=x/commit=$commit/g" PKGBUILD
    cat PKGBUILD
    sudo docker run --privileged -v $PWD:/w -w /tmp zasdfgbnm/archlinux-yaourt bash -c '
        cd /w
        makepkg -g
        makepkg --printsrcinfo | sudo tee .SRCINFO
    '
    sudo chown -R $(id -u):$(id -g) .
    cat PKGBUILD
    cat .SRCINFO
}

pushd tcg
run
popd

pushd tcg-git
run
popd
