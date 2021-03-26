#!/bin/bash

set -eux

push_repo () {
    git clone ssh://aur@aur.archlinux.org/$1.git publish
    pushd publish
    rm -rf ./*
    cp ../packaging/archlinux/$1/{PKGBUILD,.SRCINFO} .
    git add .
    git commit -m "version bump"
    git push
    popd
    rm -rf publish
}

push_repo tcg
push_repo tcg-git
