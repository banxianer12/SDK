::adb push %~dp0avm/avm_test /
adb push %~dp0avm_demo /avm_test/

adb shell chmod 777 /avm_test/avm_demo
pause

