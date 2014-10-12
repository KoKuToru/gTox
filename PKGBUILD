# Maintainer: Luca Béla Palkovics <luca.bela.palkovics@gmail.com>

pkgrel=1
pkgver=20141012
pkgname=('gTox-git')
pkgdesc="gTox a GTK-based tox-client"
url="https://github.com/KoKuToru/gTox.git"
license='GPL3'
arch=('i686' 'x86_64')
depends=('gtkmm3' 'libnotifymm' 'libconfig' 'libsodium' 'libvpx' 'opus')
makedepends=('check' 'git' 'cmake')
source=("${pkgname%-git}::git+https://github.com/KoKuToru/gTox.git")
sha256sums=('SKIP')
provides=('gTox')
conflicts=('gTox')

pkgver()
{
     cd ${pkgname%-git}
     git log -1 --format="%cd" --date=short | sed "s|-||g"
}

build()
{
    cd ${pkgname%-git}
    mkdir build
    cd build
    cmake ..
    make
}

package()
{
	cd ${pkgname%-git}
	cd build
	cp gTox /usr/bin/
}
