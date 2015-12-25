#!/bin/bash
cd ..
pwd

#install xdg-app
su -c "yaourt -Sy --noconfirm --m-arg \"--skipchecksums --skippgpcheck\" xdg-app" -s /bin/bash nobody || exit $?

echo "add remote"
xdg-app add-remote --user gnome-sdk http://sdk.gnome.org/repo/ --no-gpg-verify || exit $?
echo "install org.gnome.Platform"
xdg-app install-runtime --user gnome-sdk org.gnome.Platform 3.18 || exit $?
echo "install org.freedesktop.Platform"
xdg-app install-runtime --user gnome-sdk org.freedesktop.Platform 1.0 || exit $?
echo "install org.gnome.Sdk"
xdg-app install-runtime --user gnome-sdk org.gnome.Sdk 3.18 || exit $?

echo "build org.kokutoru.gtox.json"
xdg-app-builder build ../xdg-app/org.kokutoru.gtox.json || exit $?
