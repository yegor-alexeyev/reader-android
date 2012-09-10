#!/usr/bin/env bash
PROJECT=reader
PACKAGE=org.yegor.reader
ACTIVITY=UIHandler

if test `adb get-state` != device
then
  echo Error: device is not connected to the Android Debug Bridge
fi
ant debug
adb -d install -r bin/$PROJECT-debug.apk
adb -d shell "am start -D -n $PACKAGE/$PACKAGE.$ACTIVITY"
for pid in `adb -d jdwp`
do
  name=$(adb shell ps $pid | awk -v RS=\\r\\n 'NR==2 {printf $NF}')
  echo Found a process hosting a JDWP transport: PID=$pid, NAME = $name
  if test $name = $PACKAGE
  then
    adb forward tcp:29882 jdwp:$pid
    exec jdb -connect com.sun.jdi.SocketAttach:hostname=localhost,port=29882
  fi
done
echo Error: debug launch failure
