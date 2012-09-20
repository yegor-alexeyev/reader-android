#!/usr/bin/env bash
cscope -b -i <(find src /home/keeper/source/android/frameworks/base -type f -name '*.java')
