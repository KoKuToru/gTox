#!/bin/bash
#if [ "$TRAVIS_BRANCH" != "master" -o "$TRAVIS_PULL_REQUEST" != "false" ]; then
#    echo "Nothing todo" 
#    exit
#fi

xdg-app build-export ../repro .
xdg-app repo-update ../repro

if [ ! -z "$PUBLISH_KEY" ]; then
    echo "Upload"
    find ../repro -type f -exec curl -ftp-create-dirs -u $PUBLISH_KEY -T {} $PUBLISH_HOST/{} -k \;
fi

