#!/usr/bin/env sh

cd /opt/riski-sh/

if [ ! -d ./libwebsockets/ ]; then
  git clone https://github.com/warmcat/libwebsockets
fi

cd ./libwebsockets/
git pull
mkdir -p build
cd build
cmake -DLWS_WITH_HTTP2=0 -DCMAKE_INSTALL_PREFIX:PATH=/opt/riski-sh/env ..
make
make install
