<h1><img src="https://rawgit.com/KoKuToru/gTox/master/Icons/icon_128.svg">GTK3-Style Tox-Client</h1>

How to install
============
Archlinux
------------
<a href="https://aur.archlinux.org/packages/gtox-git/">MAKEPKG</a>
```bash
yaourt gtox-git
```
Linux
------------
Check "How to build" and `make install`

Mac, Windows
------------
No idea. Somehow get dependencies installed.
*Good luck.*

How to build
============
First read <a href="https://github.com/irungentoo/toxcore/blob/master/INSTALL.md">`https://github.com/irungentoo/toxcore/blob/master/INSTALL.md`</a>.<br />
This project will download and compile `toxcore`.
Make sure you have all dependencies.

```bash
mkdir build
cd build
cmake ..
make
```

Linux Dependencies
-----------
`gtkmm3`, `libnotifymm`, `librsvg`

Screenshot
============
<img src="https://rawgit.com/KoKuToru/gTox/master/20141012.png">

Mockup
============
<img src="https://rawgit.com/KoKuToru/gTox/master/mockup.svg">

