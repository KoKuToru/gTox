#!/bin/bash

for file in $(find . -name \*.h -o -name \*.cpp); do
        if [[ "$file" != *"svg2h.cpp" ]]; then
	if [[ "$file" != "./toxcore"* ]]; then
	if [[ "$file" != "./.git"* ]]; then
	if [[ "$file" != "./SQLiteCpp"* ]]; then
	if [[ "$file" != "./Generated"* ]]; then
		echo "Updating $file"
                file_cpy=`echo "$file.tmp"`
                mv "$file" "$file_cpy"
                clang-format "$file_cpy" > "$file"
                rm "$file_cpy"
        #       cat "$file"
	fi
	fi
	fi
	fi
        fi
done

