#!/bin/bash

#install xdg-app
su -c "yaourt -Sy --noconfirm --m-arg \"--skipchecksums --skippgpcheck\" xdg-app" -s /bin/bash nobody || exit $?

xdg-app add-remote --user gnome-sdk http://sdk.gnome.org/repo/ --no-gpg-verify || exit $?
xdg-app install-runtime --user gnome-sdk org.gnome.Platform 3.18 || exit $?
xdg-app install-runtime --user gnome-sdk org.freedesktop.Platform 1.0 || exit $?
xdg-app install-runtime --user gnome-sdk org.gnome.Sdk 3.18 || exit $?

xdg-app-builder . ../xdg-app/org.kokutoru.gtox.json || exit $?
