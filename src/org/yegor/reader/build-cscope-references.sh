#!/usr/bin/env bash
cscope -b -i <(find . /home/keeper/source/android/libcore/luni/src/main/java /home/keeper/source/android/frameworks/base -type f -name '*.java')
