#!/bin/bash
AUTO_XLIBS="$LICHEE_PLATFORM_DIR/framework/auto/t5_sdk_xlibs"

if [ "x" = "x$LICHEE_BR_OUT" ]; then
    echo "auto/build.sh:error LICHEE_BR_OUT string is empty"
	echo -e "\033[31mpls source .buildconfig first!\033[0m"
	exit
fi

cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_lib/lib64/*                     $LICHEE_BR_OUT/target/usr/lib/
cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_lib/cedarx/lib/*.so             $LICHEE_BR_OUT/target/usr/lib/
cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_lib/cedarx/bin/*           	 $LICHEE_BR_OUT/target/usr/bin/
cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_demo/bin/* 					 $LICHEE_BR_OUT/target/usr/bin/
cp -rf $LICHEE_PLATFORM_DIR/framework/auto/rootfs/*                          $LICHEE_BR_OUT/target/

if [ -d "$AUTO_XLIBS" ]; then
	make -C $AUTO_XLIBS
fi

make -C $LICHEE_PLATFORM_DIR/framework/auto/sdk_lib 
if [ $? -ne 0 ]; then
	echo  "compile sdk_lib fail .... "
	exit 1
else
	cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_lib/lib64/*                     $LICHEE_BR_OUT/target/usr/lib/
fi

echo "compile sdk_demo"
make -C $LICHEE_PLATFORM_DIR/framework/auto/sdk_demo
if [ $? -ne 0 ]; then
	echo  "compile sdk_demo fail"
	exit 1
else
	cp -f $LICHEE_PLATFORM_DIR/framework/auto/sdk_demo/bin/* 					 $LICHEE_BR_OUT/target/usr/bin/
fi

#we only use libtinyalsa.so in /usr/lib,not the /lib
if [ -e "$LICHEE_BR_OUT/target/lib/libtinyalsa.so" ]; then
	rm $LICHEE_BR_OUT/target/lib/libtinyalsa.so
fi

if [ "x" != "x$QT_INSTALL_DIR"  -a "$LICHEE_PLATFORM" != "ubuntu18"  \
        -a "${LICHEE_PLATFORM}" != "debian10" ]; then
	if [ -e $QT_INSTALL_DIR/bin/qmake ];then
		cd $LICHEE_PLATFORM_DIR/framework/auto/qt_demo
		./build.sh
		cp -f ./CameraUI/CameraUI $LICHEE_BR_OUT/target/usr/bin/
		cp -f ./Launcher/bin/Launcher $LICHEE_BR_OUT/target/usr/bin/
		cp -f ./MediaUI/bin/MediaUI $LICHEE_BR_OUT/target/usr/bin/
		cd -
		echo "build qt demo done."
	else
		echo "\033[33mWAR: there are no Qt libs, so skip to build qt_demo.  \033[0m"
	fi
fi
