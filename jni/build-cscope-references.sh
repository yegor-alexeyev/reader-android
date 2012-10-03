#!/usr/bin/env bash
cscope -k -b -i <(find . /opt/android.com/android-ndk-r8b/platforms/android-14/arch-arm/usr/include/ /home/keeper/source/opencv/modules -type f -name '*.c' -or -name '*.h' -or -name '*.cpp' -or -name '*.hpp')
