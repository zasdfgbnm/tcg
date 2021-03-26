#!/usr/bin/xonsh

$RAISE_SUBPROC_ERROR = True
$XONSH_TRACE_SUBPROC = True
trace on


ver = $(python ../../shells/xonsh/setup.py --version).strip()
pkgname = f"tcg_{ver}-1"

mkdir -p @(pkgname)/usr/bin
cp ../../build/tcg @(pkgname)/usr/bin

mkdir -p @(pkgname)/DEBIAN
cp control @(pkgname)/DEBIAN

def getver(package):
    pkgver = $(apt list --installed | grep @(package)).strip()
    pkgver = pkgver.splitlines()
    assert len(pkgver) == 1
    pkgver = pkgver[0]
    pkgver = pkgver.split()
    assert len(pkgver) == 4
    pkgver = pkgver[1]
    pkgver = pkgver.split('.')
    pkgver = '.'.join(pkgver[:2])
    return pkgver

sed -i f"s/pkgver/{ver}/g" @(pkgname)/DEBIAN/control
sed -i f"s/libboostver/{getver('libboost-all-dev')}/g" @(pkgname)/DEBIAN/control
sed -i f"s/libfmtver/{getver('libfmt-dev')}/g" @(pkgname)/DEBIAN/control
sed -i f"s/libspdlogver/{getver('libspdlog-dev')}/g" @(pkgname)/DEBIAN/control

cat @(pkgname)/DEBIAN/control

dpkg-deb --build @(pkgname)
