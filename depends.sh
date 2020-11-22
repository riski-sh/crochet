#!/usr/bin/env sh

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
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
cmake ..
make install clean
