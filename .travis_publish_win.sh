#!/usr/bin/bash
mkdir ./publish/
cp -r ./share ./publish/
cp gtox.exe ./publish/bin/gtox.exe

cd publish

echo "Fix libharfbuzz-1.dll bug"
cp bin/libharfbuzz-0.dll ./publish/bin/libharfbuzz-1.dll
cp /bin/libharfbuzz-gobject-0.dll ./publish/bin/libharfbuzz-gobject-1.dll
cp /bin/libharfbuzz-icu-0.dll ./publish/bin/libharfbuzz-icu-1.dll

echo "Generate loaders.cache for SVG"
FILE=lib/gdk-pixbuf-2.0/2.10.0/loaders.cache
echo "\"../lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.dll\"" > $FILE
echo "\"svg\" 2 \"gdk-pixbuf\" \"Scalable Vector Graphics\" \"LGPL\"" >> $FILE
echo "\"image/svg+xml\" \"image/svg\" \"image/svg-xml\" \"image/vnd.adobe.svg+xml\" \"text/xml-svg\" \"image/svg+xml-compressed\" \"\"" >> $FILE
echo "\"svg\" \"svgz\" \"svg.gz\" \"\"" >> $FILE
echo "\" <svg\" \"*    \" 100" >> $FILE
echo "\" <\!DOCTYPE svg\" \"*             \" 100" >> $FILE
echo "" >> $FILE
echo "" >> $FILE

echo "Remove unused files"
find . -name "*.a" -type f -delete
find . -name "*.h" -type f -delete
find . -name "*.c" -type f -delete
find . -name "*.hpp" -type f -delete
find . -name "*.cpp" -type f -delete
find . -name "*.o" -type f -delete
find . -name "*.pc" -type f -delete
find . -name "*.m4" -type f -delete
find . -name "*.sh" -type f -delete
find . -name "*.spec" -type f -delete
find . -name "*.cmake" -type f -delete
find . -name include -exec rm -rf {} \;
find . -name proc -exec rm -rf {} \;
rm -rf share/doc
find . -type d -empty -exec rm -rf {} \;

echo "Get version"
PKG_VERSION=`git describe --long --tags 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' || printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"`

echo "Generate Zip"
sudo pacman -S zip --noconfirm
zip -r9 ../$PUB_FILE.$PKG_VERSION.zip *
cd ..

if [ ! -z "$PUBLISH_KEY" ]; then
    echo "Upload"
    sudo pacman -S curl --noconfirm
    curl -v -u $PUBLISH_KEY -T $PUB_FILE.$PKG_VERSION.zip $PUBLISH_HOST
fi

