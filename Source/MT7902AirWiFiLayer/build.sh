#!/bin/zsh
# Copyright (C) 2026 Yusufcan Yavic, SPDX-License-Identifier: GPL-3.0-or-later
# MT7902AirWiFiLayer: cekirdek bayraklariyla derle ve kext paketini olustur.
set -e
D=${0:A:h}
# MT7902_MACOS=26: macOS 26 layer (sdk26 with IONetworkController slots named as in 26, -DMT7902_OS26)
if [ "${MT7902_MACOS:-15}" = 26 ]; then SDK=${MACKERNELSDK:-$D/../sdk26/MacKernelSDK}; OSDEF=-DMT7902_OS26
else SDK=${MACKERNELSDK:-$D/../MT7902MtkWlan/MacKernelSDK}; OSDEF=; fi
XC=${DEVELOPER_DIR:-$(xcode-select -p)}
CXX=$(DEVELOPER_DIR=$XC xcrun -f clang++); CC=$(DEVELOPER_DIR=$XC xcrun -f clang)
KSDK=$(DEVELOPER_DIR=$XC xcrun --sdk macosx --show-sdk-path)
OUT=$D/build; rm -rf $OUT; mkdir -p $OUT/obj $OUT/MT7902AirWiFiLayer.kext/Contents/MacOS
FLAGS=(-target x86_64-apple-macos11.0 -mkernel -fapple-kext -nostdinc -fno-builtin -fno-common -O2 -g
       -DMT7902_VERSION_STR='"'${MT7902_VERSION:-dev}'"' ${OSDEF:+$OSDEF} -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DPRIVATE -D__PRIVATE_SPI__ -DAPPLE -DNeXT
       -I$D/src -I$SDK/Headers -I$SDK/Headers/IOKit -isystem $KSDK/System/Library/Frameworks/Kernel.framework/Headers
       -fmacro-prefix-map=$D/../= -fdebug-prefix-map=$D/../=
       -Wno-everything -Werror=implicit-function-declaration)
$CXX -x c++ -std=gnu++17 -fno-exceptions -fno-rtti $FLAGS -c $D/src/MT7902AirWiFiLayer.cpp -o $OUT/obj/layer.o
$CC -x c ${FLAGS:#-fapple-kext} -c $D/src/MT7902AirWiFiLayer_info.c -o $OUT/obj/info.o
$CXX -target x86_64-apple-macos11.0 -nostdlib -Xlinker -kext -L$SDK/Library/x86_64 -L$KSDK/usr/lib \
     $OUT/obj/layer.o $OUT/obj/info.o -lkmodc++ -lkmod -lcc_kext -lkmod \
     -o $OUT/MT7902AirWiFiLayer.kext/Contents/MacOS/MT7902AirWiFiLayer
cp $D/Info.plist $OUT/MT7902AirWiFiLayer.kext/Contents/Info.plist
echo "kext: $OUT/MT7902AirWiFiLayer.kext"
