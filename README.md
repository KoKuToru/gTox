<h1><img src="https://rawgit.com/KoKuToru/gTox/master/Source/Icons/icon_128.svg">GTK3-Style Tox-Client</h1>

* [Features](#features)
* [How to install](#how-to-install)
  * [ArchLinux](#archlinux)
  * [Linux](#linux)
  * [Mac, Windows](#mac-windows)
* [How to build](#how-to-build)
  * [Linux Dependencies](#linux-dependencies)
* [Screenshot](#screenshot)

Features
============
For comparison https://wiki.tox.im/Client

|               | gTox          | Issue  | Comment |
| ------------- |:-------------:|:------:|-------|
| **Interface** | Desktop GUI   |        |
| Linux         | **Yes**       |        |
| Windows       | Maybe         |        |
| OS X          | Maybe         |        |
| BSD-like      | Maybe         |        |
| Android       | No            |        |
| iOS           | No            |        |
| 1v1 messaging | **Yes** | |
| File transfer | Yes/No | [#55](https://github.com/KoKuToru/gTox/issues/55) | only receiving working
| Group chat | No | |
| Group audio | No | |
| Audio | No | |
| Video | No | |
| DNS discovery | No | |
| Chat logs | **Yes** | |
| Proxy support | No | |
| Audio Filtering | No | |
| Faux offline messaging | No | |
| Contact aliases | No | |
| Contact blocking | No | |
| Save file encryption | No | |
| Multilingual | **Yes** | | *English / German / Russian / Spanish / Italian*
| Multiprofile | **Yes** | [#26](https://github.com/KoKuToru/gTox/issues/26) |
| Typing notification | No | |
| Audio notifications | **Yes** | | *When a message comes in*
| Emoticons | Yes/No | [#25](https://github.com/KoKuToru/gTox/issues/25) | *Only UTF-8*
| Spell check | No | | 
| Desktop sharing | No | |
| Inline images | **Yes++** | | **Inline videos**
| File resuming | Yes/No | | *only receiving working*
| Read receipts | **Yes** | | *But log wont save "failed" yet*
| Message splitting | No | | 
| Changing nospam | No | |
| tox: URI | No | [#93](https://github.com/KoKuToru/gTox/issues/93) |
| Avatars | Yes/No | [#27](https://github.com/KoKuToru/gTox/issues/27) | only receiving working

Special to gTox:
* Markup support bold(crtl+b), italic (crtl+i), underline (crtl+u)

Not much, not ready for daily use ! Please wait for the first release.

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
This project will need `toxcore`.

```bash
git clone https://github.com/KoKuToru/gTox.git
cd gTox
git submodule init
git submodule update
mkdir build
cd build
cmake ../Source
make
```

Linux Dependencies
-----------
`gtkmm3`, `libnotifymm`, `librsvg`, `sqlite`, `gettext`, `libcanberra`, `gstreamermm`

Screenshot
============
<img src="https://rawgit.com/KoKuToru/gTox/master/Screenshots/profile_selection.png">
<img src="https://rawgit.com/KoKuToru/gTox/master/Screenshots/profile_creation.png">
<img src="https://rawgit.com/KoKuToru/gTox/master/Screenshots/client_chat.png">

