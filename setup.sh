#!/usr/bin/env sh

# bool function to test if the user is root or not (POSIX only)
is_user_root() { [ "$(id -u)" -eq 0 ]; }

# bool function to test if user crochet exists
exists_riski() {
  id -u riski
  [ "$?" -eq 0 ];
}

if ! is_user_root ; then
  echo 'I need to run as root'
  exit 1
fi

if ! exists_riski ;
then
  echo 'creating user crochet'
  pw user add -n riski
fi

mkdir -p /opt/riski-sh
chown -R riski:wheel /opt/riski-sh

sudo -u riski sh << EOF

cd /opt/riski-sh/
git clone https://github.com/riski-sh/crochet

EOF
