#!/bin/env bash

MODNAME="crochet"

if [[ ! -z $1 ]] && [[ "$1" == "clean" ]]; then
  echo "cleaning"
  find . -name ".bhash" | xargs rm
fi

# Retreive the current working directory
CWD=$(pwd)

# Make sure we create an objects folder to store all the objects
mkdir -p $CWD/objects > /dev/null 2>&1

# A list of required libraries
REQUIRES=("openssl")

# A list of modules that will be compiled
MODULES=("${CWD}/src/exchanges/" "${CWD}/src/ffjson/" "${CWD}/src/finmath/"
  "${CWD}/src/hashmap/" "${CWD}/src/httpws/" "${CWD}/src/orderbooks/"
  "${CWD}/src/security/" "${CWD}/src/pprint" "${CWD}/src/globals/")

# The single file that contains the main class
MAINC="${CWD}/src/main.c"

# Define default W flags
WFLAGS=("-Weverything" "-Wpedantic" "-Werror" "-O2" "-g")

# Define default CFLAGS
CFLAGS=("-I$CWD" "-I$CWD/src/")

# Define default LFLAGS
LFLAGS=()

# Make sure a compiler is specified if not this will try to find the default
# clang compiler
if [[ -z $CC ]]; then
  # Try to find a default clang compiler and test again
  CC=$(command -v clang)
  if [[ -z $CC ]]; then
    # Unable to find the default clang compiler
    echo "error: please set CC or install clang"
    exit 1
  fi
fi

# Look for pkg-config
PKGCFG=$(command -v pkg-config)
if [[ -z $PKGCFG ]]; then
  echo "error: please install pkg-config"
  exit 1
fi

# Loop through each library in REQUIRES and find the CFLAGS and LFLAGS
# for that library
for lib in ${REQUIRES[*]}
do

  # Test pkg-config to make sure the library exists
  $(pkg-config $lib) > /dev/null 2>&1
  if [[ $? -ne 0 ]]; then
    echo "error: unable to locate $lib"
  fi

  CFLAGS+=($(pkg-config --cflags $lib))
  LFLAGS+=($(pkg-config --libs $lib))
done

# Loop through each subdirectory defined in the MODULES array
for i in ${MODULES[@]}
do

  # Go int othe module subdirectory
  cd $i

  BASEMODULENAME=$(basename $i)

  # Touch the .bhash file to make sure it exists
  touch .bhash

  # Hash each file in the subdirectory
  find . -name "*.c" | xargs shasum >> .bhashtmp

  # Compare the diffs between the current file hashes and the hashes
  # of the last build
  DIFFS=$(diff .bhash .bhashtmp)

  # If there are no differences remove the temp file and continue on to the
  # next submodule
  if [[ -z $DIFFS ]]; then
    rm .bhashtmp
    continue
  fi

  MODULEOBJECTS=()

  # Loop through each c file in this folder and compile it
  for f in *.c
  do
    objname=$(echo "$f" | cut -f 1 -d '.')\.o
    MODULEOBJECTS+=("$CWD/objects/$objname")
    set -e
    echo $CC -c -fPIC ${CFLAGS[*]// /} ${WFLAGS[*]// /} $f -o $CWD/objects/$objname
    $CC -c -fPIC ${CFLAGS[*]// /} ${WFLAGS[*]// /} $f -o $CWD/objects/$objname
    set +e
  done

  # Create a shared library
  echo $CC -shared ${MODULEOBJECTS[*]// /} -o $CWD/objects/$BASEMODULENAME\.so
  $CC -shared ${MODULEOBJECTS[*]// /} -o $CWD/objects/$BASEMODULENAME\.so

  rm -rf ${MODULEOBJECTS[*]// /}

  # Remove the current bhash and bhashtmp files
  # Write out a new bhash file.
  rm .bhash
  rm .bhashtmp
  find . -name "*.c" | xargs shasum >> .bhash

done

cd $CWD
echo $CC -o $MODNAME $LFLAGS $CFLAGS $MAINC ./objects/*.so
$CC -o $MODNAME ${LFLAGS[*]// /} ${CFLAGS[*]// /} $MAINC ./objects/*.so
