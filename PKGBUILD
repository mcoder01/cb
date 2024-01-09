pkgname=cb
pkgver=1.0.1
pkgrel=1

pkgdesc="An easy-to-use tool for converting from one base system to another."
arch=('i686' 'x86_64' 'arm' 'armv6h' 'armv7h' 'aarch64')
url="https://github.com/mcoder01/cb"
license=('GPL3')
provides=("cb=$pkgver-$pkgrel")

builddir="../build"
installdir="usr/bin"

prepare() {
    mkdir -p "${builddir}"
}

build() {
    gcc "${srcdir}/cb.c" -lm -o "${builddir}/cb"
}

package() {
    mkdir -p "${pkgdir}/${installdir}"
    cp "${builddir}/cb" "${pkgdir}/${installdir}"
}
