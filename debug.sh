#!/usr/bin/env bash

PROJECT=$(xmllint --xpath 'string(/project/@name)' build.xml) 
if test -z $PROJECT
then
  echo Error: cannot parse ant build file build.xml >&2
  exit 1
fi
echo Project: $PROJECT

PACKAGE=$(xmllint --xpath 'string(/manifest/@package)' AndroidManifest.xml) 
if test -z $PACKAGE 
then
  echo Error: cannot parse package manifest file AndroidManifest.xml >&2
  exit 1
fi
echo Package: $PACKAGE

#The script launches first activity declared in the manifest file
ACTIVITY=$(xmllint --xpath "string(/manifest/application/activity[1]/@*[namespace-uri()='http://schemas.android.com/apk/res/android' and local-name()='name'])" AndroidManifest.xml) 
if test -z $ACTIVITY 
then
  echo Error: cannot parse package manifest file AndroidManifest.xml >&2
  exit 1
fi
echo Activity: $ACTIVITY

if test `adb get-state| tail --lines=1` != device
then
  echo Error: device is not connected to the Android Debug Bridge
fi

ant nodeps debug || exit 1
adb uninstall $PACKAGE
adb -d install -r bin/$PROJECT-debug.apk
#adb -d install -r bin/$PROJECT-release.apk

if test "$1" = nodebug
then
  adb -d shell "am start -W -n $PACKAGE/$PACKAGE.$ACTIVITY"
  exit 0
else
  adb -d shell "am start -W -D -n $PACKAGE/$PACKAGE.$ACTIVITY"
fi

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
