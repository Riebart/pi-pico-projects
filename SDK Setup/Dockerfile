FROM ubuntu:jammy

ENV PICO_SDK_PATH="/pico-sdk/pico-sdk"
ENV PICO_EXAMPLES_PATH="/pico-sdk/pico-examples"
ENV PICO_EXTRAS_PATH="/pico-sdk/pico-extras"
ENV PICO_PLAYGROUND_PATH="/pico-sdk/pico-playground"

WORKDIR /working

RUN mkdir /pico-sdk
RUN apt update && \
    apt install -y \
        pkg-config \
        libftdi-dev texinfo \
        libusb-1.0-0 libusb-1.0-0-dev libusb-dev libusb-1.0-doc \
        python3 git cmake libtool automake autoconf build-essential \
        gcc-arm-none-eabi gcc g++ libstdc++-arm-none-eabi-newlib && \
    apt clean

RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/pico-sdk && \
    cd /pico-sdk/pico-sdk && git submodule update --init
RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/pico-examples && \
    cd /pico-sdk/pico-examples && git submodule update --init
RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/pico-extras && \
    cd /pico-sdk/pico-extras && git submodule update --init
RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/pico-playground && \
    cd /pico-sdk/pico-playground && git submodule update --init
RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/picotool.git && \
    cd /pico-sdk/picotool && git submodule update --init && \
    mkdir build && cd build && cmake ../ && make
RUN git -C /pico-sdk clone -b master https://github.com/raspberrypi/picoprobe.git && \
    cd /pico-sdk/picoprobe && git submodule update --init && \
    mkdir build && cd build && cmake ../ && make

# RUN /root/setup.sh
