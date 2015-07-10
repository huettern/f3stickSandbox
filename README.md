# STM32F3 template
[![Build Status](https://travis-ci.org/noah95/f3stickSandbox.svg?branch=usb_cdc)](https://travis-ci.org/noah95/f3stickSandbox)

### Directory List

- config: 		Configuration files
- src: 			User sources
- doc: 			Documentation




### Building OpenOCD
Get it from http://sourceforge.net/projects/openocd
```bash
sudo apt-get install libusb-1.0-0-dev
./configure --enable-stlink
make
sudo make install
```

### Allow access to ST-Link
Add user to group plugdev
```bash
sudo useradd -G plugdev USERNAME
```
Add udev rule
```bash
sudo echo 'ATTRS{idProduct}=="3748", ATTRS{idVendor}=="0483", MODE="666", GROUP="plugdev"' > /etc/udev/rules.d/49-stlinkv2.rules
```
Reload rules
```bash
sudo udevadm trigger
```
Users added to the group plugdev should now be able to use openocd without sudo




**Free Software, Hell Yeah!**
