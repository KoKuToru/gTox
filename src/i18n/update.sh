#!/usr/bin/bash
for f in *.po; do
	msgcat --use-first $f template.pot -o $f
	msgattrib --set-obsolete --ignore-file=template.pot -o $f $f
	msgattrib --no-obsolete -o $f $f
done
