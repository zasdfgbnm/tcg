#!/usr/bin/xonsh

$RAISE_SUBPROC_ERROR = True
$XONSH_TRACE_SUBPROC = True
trace on


ver = $(python ../../shells/xonsh/setup.py --version)
pkgname = f"tcg_{ver}-1"

mkdir -p $pkgname/usr/bin
cp ../../build/tcg $pkgname/usr/bin

mkdir -p $pkgname/DEBIAN
cp control $pkgname/DEBIAN
sed -i f"s/pkgver/{ver}/g" $pkgname/DEBIAN/control
sed -i f"s/libboostver/{getver('libboost-all')}/g" $pkgname/DEBIAN/control
sed -i f"s/libfmtver/{getver('libfmt')}/g" $pkgname/DEBIAN/control
sed -i f"s/libspdlogver/{getver('libspdlog')}/g" $pkgname/DEBIAN/control

def getver(package):
    pkgver = $(apt list --installed | grep $package).strip()
    pkgver = pkgver.splitlines()
    assert len(pkgver) == 1
    pkgver = pkgver[0]