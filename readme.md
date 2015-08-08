USBaspTool
==========

USBaspTool is a host application, utilizing USBaspLoader's
"*BOOTLOADER_HIDDENEXITCOMMAND"-feature.

Many other bootloaders (avrboot, hidboot or dfu for example)
implement similar features and are resetable by their
corresponding host application.

Calling "./usbasploader exit" enables users to startup
firmware directly from script (a.k.a. makefile) without
physically pressing "reset"- or "prog"-button. 

It basically sends an ISP raw-command "0xff 0xXX 0xXX 0xXX"
to the usbasp device, as it could be done via
"send 0xff 0x00 0x00 0x00" in avrdude's ISP console:
"avrdude -c usbasp -p atmega8 -t"
But since this can not be implemented in script, there is
USBaspTool.


USBaspTool is multi device compatible, so a specific usbasp
board can be commanded to, even if multiple boards are
connected to the same host.
Together with tinyUSBboards "bootloader_startup()"-API,
modified versions of USBaspTool can be done in order make
firmwares resetable, too.
Enabled to switch both, bootloader and firmware, gives users
full softwarecontrol for updates or arduino-like feeling.

by Stephan Baerwolf (stephan@matrixstorm.com), Schwansee 2015
