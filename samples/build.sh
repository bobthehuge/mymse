#!/bin/bash

ARG=$1
TARGET="${ARG%.m68k}"
a68k -o$TARGET.srec -s -n -rmal $ARG
