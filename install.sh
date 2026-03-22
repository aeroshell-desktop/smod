#!/bin/bash

BUILD_DST="build"

SU_CMD=sudo

if [[ -z "$(command -v $SU_CMD)" ]]; then
    SU_CMD=doas
    if [[ -z "$(command -v $SU_CMD)" ]]; then
        echo "Neither sudo or doas were detected on the system."
        exit
    fi
fi

rm -rf "$BUILD_DST"
mkdir -p "$BUILD_DST"
cmake -DCMAKE_INSTALL_PREFIX=/usr -B "$BUILD_DST" .
cmake --build "$BUILD_DST"
$SU_CMD cmake --install "$BUILD_DST"

cd smodglow
if [[ ! "$*" == *"--skip-x11"* ]]
then
    bash install.sh
fi
bash install.sh --wayland

