#!/bin/bash

# LD_PRELOAD lists libraries that are to be mapped into the
# binary's process address space before execution.
# This makes libheapsentryu's malloc() and free() to be found
# before the ones belonging to libc.
export LD_PRELOAD=./lib/libheapsentryu.so
./libtest
