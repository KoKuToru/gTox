#!/usr/bin/bash
echo "translation | untranslated"
echo ":-----------|------------:" 
for f in *.po; do
    if [ $f -ne en.po ]; then
        untranslated=`msgfmt --statistics -o template.pot $f 2>&1 | tr -dc '0-9 ' | awk -v N=2 '{print $N}'`
        printf "%-12s|%13s\n" $f $untranslated
    fi
done
