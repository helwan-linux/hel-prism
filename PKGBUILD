# Maintainer: Saeed Badreldin <helwanlinux@gmail.com> 
pkgname=hel-prism
pkgver=1.1  # تم تحديث الإصدار لدعم المراحل اللانهائية
pkgrel=1
pkgdesc="A physics-based laser puzzle game for Helwan Linux with infinite levels" 
arch=('x86_64') 
url="https://github.com/helwan-linux/hel-prism" 
license=('GPL') 
depends=('gtk3') 
makedepends=('gcc' 'git') 
source=("git+https://github.com/helwan-linux/hel-prism.git") 
md5sums=('SKIP') 

build() {
    cd "$srcdir/hel-prism/helwan-prism" 
    # إضافة levels.c إلى أمر التجميع 
    gcc helwan-prism.c levels.c -o helwan-prism $(pkg-config --cflags --libs gtk+-3.0) -lm 
}

package() {
    cd "$srcdir/hel-prism/helwan-prism" 

    # 1. تثبيت الملف التنفيذي 
    install -Dm755 helwan-prism "$pkgdir/usr/bin/helwan-prism" 

    # 2. تثبيت ملف الـ Desktop 
    install -Dm644 helwan-prism.desktop "$pkgdir/usr/share/applications/helwan-prism.desktop" 

    # 3. تثبيت الأيقونة [cite: 2]
    install -Dm644 icon.png "$pkgdir/usr/share/pixmaps/helwan-prism.png"
}
