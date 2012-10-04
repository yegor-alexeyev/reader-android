#!/usr/bin/env bash
adb -d logcat -c
adb -d logcat -v time  | grep reader 
