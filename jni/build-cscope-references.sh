#!/usr/bin/env bash
cscope -k -b -i <(find . /opt/android.com/android-ndk-r8b/platforms/android-14/arch-arm/usr/include/ -type f -name '*.c' -or -name '*.h' -or -name '*.cpp')
