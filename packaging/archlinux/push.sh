#!/bin/bash

set -eux

push_repo () {
    git clone ssh://aur@aur.archlinux.org/$1.git
    pushd $1
    rm -rf *
    cp ../packaging/archlinux/$1/{PKGBUILD,.SRCINFO} .
    git add .
    git commit -m "version bump"
    git push
    popd
}

push_repo tcg
push_repo tcg-git
