#!/usr/bin/env bash

if ! [ $(id -u) = 0 ]; then
   echo "I am not root! sad sad"
   exit 1
fi

mkdir depends
cd depends

if [ ! -d ./libwebsockets/ ]; then
  git clone https://github.com/warmcat/libwebsockets
fi

cd ./libwebsockets/
git pull
mkdir -p build
cd build
cmake .. -DLWS_WITH_HTTP2=0
make install
