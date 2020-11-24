#!/usr/bin/env bash

mkdir depends
cd depends

if [ ! -d ./libwebsockets/ ]; then
  git clone https://github.com/warmcat/libwebsockets
fi

cd ./libwebsockets/
git pull
mkdir -p build
cd build
cmake ..
make
