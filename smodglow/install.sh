#!/bin/bash

BUILD_DST="build"
BUILD_PLATFORM="X11"
BUILD_PARAM=

SU_CMD=sudo

if [[ -z "$(command -v $SU_CMD)" ]]; then
    SU_CMD=doas
    if [[ -z "$(command -v $SU_CMD)" ]]; then
        echo "Neither sudo or doas were detected on the system."
        exit
    fi
fi


if [[ "$*" == *"--wayland"* ]]
then
    BUILD_DST="build-wl"
    BUILD_PLATFORM="Wayland"
    BUILD_PARAM="-DKWIN_BUILD_WAYLAND=ON"
fi

echo "Building ${BUILD_PLATFORM} effect..."

cmake -DCMAKE_INSTALL_PREFIX=/usr $BUILD_PARAM -B "$BUILD_DST" .
cmake --build "$BUILD_DST"
$SU_CMD cmake --install "$BUILD_DST"

