# pico-usb-keyboard

This is a simple example of using the Pico as an automated USB keyboard.

There is an example of how to use a Pico as a USB device [in the examples package](https://github.com/raspberrypi/pico-examples/tree/master/usb/device/dev_hid_composite)
but it emulates a bunch of devices like mouse and game pad. I couldn't get it
working and was trying to debug it, so I simplified it down to just what I needed
which was the keyboard. The result was useful to me, so it might be useful
to someone else.

## Usage

Build and program the Pico with this code, then use a USB cable to connect
the Pico's USB port to your PC. In USB terminology you're connecting a
keyboard "device" to a PC "host". The USB cable acts as both the power to
the Pico, and as the USB data cable between the devices.

I figured a photo wasn't really necessary for that. :)

Now press the white BOOTSEL button on the Pico and the Pico will "type"
a letter 'a'. If the PC is running an editor you'll see the 'a' appear.

## Troubleshooting

I'm no expert on USB, so limited help from me here.

Check the USB cable is a data cable (4 internal wires) as opposed to a
power one (2 internal wires). If you got it with a cheap device that only
needs it for USB power it's probably a basic power one and won't work.

On Linux, open check /var/log/syslog when you plug the Pico USB cable in.
It should come up something like:

```
Mar 23 10:51:25 donkey kernel: [1885587.777372] usb 2-1.5: new full-speed USB device number 98 using ehci-pci
Mar 23 10:51:25 donkey kernel: [1885587.892295] usb 2-1.5: New USB device found, idVendor=cafe, idProduct=4004, bcdDevice= 1.00
Mar 23 10:51:25 donkey kernel: [1885587.892305] usb 2-1.5: New USB device strings: Mfr=1, Product=2, SerialNumber=3
Mar 23 10:51:25 donkey kernel: [1885587.892309] usb 2-1.5: Product: TinyUSB Device
Mar 23 10:51:25 donkey kernel: [1885587.892311] usb 2-1.5: Manufacturer: TinyUSB
Mar 23 10:51:25 donkey kernel: [1885587.892313] usb 2-1.5: SerialNumber: E6614C30934B5C2C
Mar 23 10:51:25 donkey kernel: [1885587.895180] input: TinyUSB TinyUSB Device Keyboard as /devices/pci0000:00/0000:00:1d.0/usb2/2-1/2-1.5/2-1.5:1.0/0003:CAFE:4004.000D/input/input272
```

[Derek Fountain](https://www.derekfountain.org/), March 2024
