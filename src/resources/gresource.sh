#!/usr/bin/bash
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > $1
echo "<gresources>" >> $1
echo "<gresource prefix=\"$2\">" >> $1
for var in ${1:+"${@:4}"}
do
    echo "<file $3>$var</file>" >> $1
done
echo "</gresource>" >> $1
echo "</gresources>" >> $1
