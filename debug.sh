#!/usr/bin/env bash
if test `adb get-state` != device
then
  echo Error: device is not connected to the Android Debug Bridge
fi
ant debug
adb -d install -r bin/reader-debug.apk
adb -d shell 'am start -D -n org.yegor.reader/org.yegor.reader.UIHandler'
for pid in `adb -d jdwp`
do
  name=$(adb shell ps $pid | awk -v RS=\\r\\n 'NR==2 {printf $NF}')
  echo Found a process hosting a JDWP transport: PID=$pid, NAME = $name
  if test $name = org.yegor.reader
  then
    adb forward tcp:29882 jdwp:$pid
    exec jdb -connect com.sun.jdi.SocketAttach:hostname=localhost,port=29882
  fi
done
echo Error: debug launch failure
