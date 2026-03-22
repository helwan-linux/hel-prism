# Maintainer: Saeed Badreldin <helwanlinux@gmail.com> 
pkgname=hel-prism
pkgver=1.0
pkgrel=1
pkgdesc="A physics-based laser puzzle game for Helwan Linux" 
arch=('x86_64') 
url="https://github.com/helwan-linux/hel-prism" 
license=('GPL') 
depends=('gtk3') 
makedepends=('gcc' 'git') 
source=("git+https://github.com/helwan-linux/hel-prism.git") 
md5sums=('SKIP') 

build() {
    # 1. ندخل مجلد الـ Repo اللي الـ git عمله 
    cd "$srcdir/hel-prism"
    
    # 2. ندخل المجلد الفرعي اللي فيه الكود فعلياً حسب الـ diagram 
    cd "helwan-prism"
    
    # 3. التجميع الآن هيلاقي الملفات قدامه 
    gcc helwan-prism.c -o helwan-prism $(pkg-config --cflags --libs gtk+-3.0) -lm 
}

package() {
    # الدخول للمكان اللي فيه الملفات الجاهزة 
    cd "$srcdir/hel-prism/helwan-prism"

    # تثبيت الملف التنفيذي 
    install -Dm755 helwan-prism "$pkgdir/usr/bin/helwan-prism"

    # تثبيت ملف الـ Desktop 
    install -Dm644 helwan-prism.desktop "$pkgdir/usr/share/applications/helwan-prism.desktop"

    # تثبيت الأيقونة 
    install -Dm644 icon.png "$pkgdir/usr/share/icons/hicolor/512x512/apps/helwan-prism.png"
}
