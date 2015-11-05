#!/usr/bin/bash
echo "translation | untranslated"
echo ":-----------|------------:" 
for f in *.po; do
    if [ "$f" != "en.po" ]; then
        untranslated=`msgcat template.pot $f -o - | msgfmt --statistic - 2>&1 | tr -dc '0-9 ' | awk -v N=2 '{print $N}'`
        printf "%-12s|%13s\n" $f $untranslated
    fi
done
