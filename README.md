<h1><img src="https://rawgit.com/KoKuToru/gTox/master/Icons/icon_128.svg">GTK3-Style Tox-Client</h1>

How to install
============
Archlinux
------------
<a href="https://aur.archlinux.org/packages/gtox-git/">MAKEPKG</a>
```bash
yaourt gtox-git
```
Linux, Mac, Windows
------------
Check "How to build"

How to build
============
First read <a href="https://github.com/irungentoo/toxcore/blob/master/INSTALL.md">`https://github.com/irungentoo/toxcore/blob/master/INSTALL.md`</a>.<br />
This project will download and compile `toxcore`.
Make sure you have all dependencys.

```bash
mkdir build
cd build
cmake ..
make
```

Linux Dependecy
-----------
`gtkmm3`, `libnotifymm`, `librsvg`

Screenshot
============
<img src="https://rawgit.com/KoKuToru/gTox/master/20141012.png">

Mockup
============
<img src="https://rawgit.com/KoKuToru/gTox/master/mockup.svg">

