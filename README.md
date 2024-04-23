# DSView
DSView编译为Windows版本，解决libusb_get_pollfds、USB热插拔在Windows上无法使用问题

基于[DSView v1.3.1](https://github.com/DreamSourceLab/DSView/tree/v1.3.1)修改、编译。 

# 依赖包下载  
通过pacman命令下载以下依赖项
 - gcc (>= 4.0)
 - make
 - cmake >= 2.6
 - Qt >= 5.0
 - libglib >= 2.32.0
 - zlib
 - libusb-1.0 >= 1.0.16
	On FreeBSD, this is an integral part of the FreeBSD libc, not an extra package/library.
	This is part of the standard OpenBSD install (not an extra package), apparently.
 - libboost >= 1.42
 - libfftw3 >= 3.3
 - libpython > 3.2
 - libtool
 - pkg-config >= 0.22
   
如：
```
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-qt5-static
......
```
   
# 编译
```
cmake .  
ninja  
```
如果在链接时遇到glib中的符号未定义，则将build.ninja中的-lglib-2.0移动到行位，再次执行ninja将继续编译链接。
