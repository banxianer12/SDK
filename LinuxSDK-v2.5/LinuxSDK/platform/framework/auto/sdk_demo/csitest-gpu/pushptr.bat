adb push %~dp0csitestT5_ptr /usr/bin
adb push %~dp0../../sdk_lib/sdk_opengl/libsdk_egl.so /usr/lib/
adb shell chmod 777 /usr/bin/csitestT5_ptr
pause

