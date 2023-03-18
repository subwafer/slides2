#!/usr/bin/env sh

set -xe

CC=clang
CFLAGS="-Wall -Wextra -ggdb -pedantic"

$CC $CFLAGS -o main main.c
