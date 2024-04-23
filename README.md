# DSView
DSView编译为Windows版本，解决libusb_get_pollfds、USB热插拔在Windows上无法使用问题

基于[DSView v1.3.1](https://github.com/DreamSourceLab/DSView/tree/v1.3.1)修改、编译。 

# 依赖包下载
`
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-qt5-static
...
`

# 编译
`
cmake .
ninja
`
