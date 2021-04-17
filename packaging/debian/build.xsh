#!/usr/bin/xonsh

$RAISE_SUBPROC_ERROR = True
$XONSH_TRACE_SUBPROC = True
trace on

ver = $(python ../../shells/xonsh/setup.py --version).strip()
pkgname = f"tcg_{ver}-1"

mkdir -p @(pkgname)/usr/bin
cp ../../build/tcg @(pkgname)/usr/bin

chown root @(pkgname)/usr/bin/tcg
chmod +s @(pkgname)/usr/bin/tcg

mkdir -p @(pkgname)/DEBIAN
cp control @(pkgname)/DEBIAN

sed -i f"s/pkgver/{ver}/g" @(pkgname)/DEBIAN/control

cat @(pkgname)/DEBIAN/control

dpkg-deb --build @(pkgname)
