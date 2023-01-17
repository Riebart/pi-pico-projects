#!/bin/bash

set +e

INSTALL_DIR="`pwd`"

mkdir pico
cd pico

for repo in sdk examples extra playground
do
    git clone -b master https://github.com/raspberrypi/pico-${repo}.git
    (cd pico-$repo; git submodule update --init)
done

export PICO_SDK_PATH="$INSTALL_DIR/pico/pico-sdk"
export PICO_EXAMPLES_PATH="$INSTALL_DIR/pico/pico-examples"
export PICO_EXTRAS_PATH="$INSTALL_DIR/pico/pico-extras"
export PICO_PLAYGROUND_PATH="$INSTALL_DIR/pico/pico-playground"

for tool in picotool picoprobe
do
    git clone -b master https://github.com/raspberrypi/picotool.git
    (cd $tool; mkdir build; cd build; cmake ../; make)
done

