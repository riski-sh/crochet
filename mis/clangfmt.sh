#!/bin/bash
find src/ -name '*.c' -or -name '*.h' | xargs clang-format -i