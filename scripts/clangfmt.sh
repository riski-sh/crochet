#!/bin/bash
find src/ -name '*.c' -or -name '*.h' | xargs clang-format -i
find libs/ -name '*.c' -or -name '*.h' | xargs clang-format -i
