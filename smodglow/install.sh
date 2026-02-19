#!/bin/bash

BUILD_DST="build"
BUILD_PLATFORM="X11"
BUILD_PARAM=
USE_NINJA=
BUILD_COMMAND="make"

SU_CMD=sudo

if [[ "$*" == *"--ninja"* ]]
then
    if [[ -z "$(command -v ninja)" ]]; then
        echo "Attempted to build using Ninja, but Ninja was not found on the system. Falling back to GNU Make."
    else
        echo "Compiling using Ninja"
        USE_NINJA="-G Ninja"
        BUILD_COMMAND="ninja"
    fi
fi

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
rm -rf "${BUILD_DST}"
mkdir "${BUILD_DST}"
cd "${BUILD_DST}"


echo "Building ${BUILD_PLATFORM} effect..."
cmake ../ -DCMAKE_INSTALL_PREFIX=/usr $BUILD_PARAM $USE_NINJA

$BUILD_COMMAND
sudo $BUILD_COMMAND install
