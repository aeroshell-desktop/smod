#!/bin/bash

BUILD_DST="build"
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

rm -rf "$BUILD_DST"
mkdir -p "$BUILD_DST"
cmake -DCMAKE_INSTALL_PREFIX=/usr -B build $USE_NINJA .
cmake --build "$BUILD_DST"
$SU_CMD cmake --install "$BUILD_DST"

cd smodglow
if [[ ! "$*" == *"--skip-x11"* ]]
then
    bash install.sh
fi
bash install.sh --wayland

