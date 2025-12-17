#!/bin/env bash

sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)

make

DEV="$(ls -1 /sys/bus/hid/devices | grep -i '^0003:0416:C169\.' | head -n1)"

# remove from Generic-Driver
echo -n "$DEV" | sudo tee /sys/bus/hid/drivers/hid-generic/unbind

# load Modul
sudo insmod hid-tstp-mtouch.ko

# explicitly bind to hid-tstp-mtouch driver
echo -n "$DEV" | sudo tee /sys/bus/hid/drivers/hid-tstp-mtouch/bind
