
adb shell killall streamPlayer
adb push %~dp0streamPlayer /usr/bin
adb shell chmod 777 /usr/bin/streamPlayer

rd /s /q %~dp0test_file\output\
adb shell rm -r /tmp/output/*
adb push %~dp0test_file/. /tmp/
adb shell mkdir /tmp/output/


adb shell streamPlayer -w 2 -t 1
adb pull /tmp/output/ %~dp0test_file\output\

pause
