MSI GE60 Apache Pro
=====================

This is a modded code of PaNaVTEC's MSI_GT_GE_Led_Enabler to compile in Linux Debian.

And edited by nake (Alfonso Arbona Gimeno) to work with MSI GE60 in Debian, adding
also new features stolen from https://github.com/Wufe/msi-keyboard
Forked from https://github.com/gokuhs/MSI_GT_GE_Led_Enabler

How to compile:
===============

Install dependences (as root):
`apt-get install libhidapi-dev libudev-dev`

And for the sensors sample install also:
`apt-get install libsensors`

It's simple first clone the repository with 

`$ git clone https://github.com/nake90/MSI_GE60_Led_Enabler`

Edit *msiledenabler.cpp* (I just created a sample code)

Now in a terminal type:

```
$ make
$ sudo ./msiledenabler
```

Remember that **you need root access**!

Use of sensors_sample:
======================

First edit *sensors_sample.cpp*

There are a few defines that you need to map to your system and preferences.
DEVICE_ID and SUBDEV_ID define which sensor to use. If you don't know which one
you should use, uncomment the printf's in the main loop and use that ID.

For example, in my system I have the following sensors:

```
temp1 [0,0]  = 27.80ºC
temp2 [0,1]  = 105.00ºC
temp3 [0,2]  = 29.80ºC
Physical id 0 [1,0]  = 52.00ºC
Core 0 [1,1]  = 84.00ºC
Core 1 [1,2]  = 100.00ºC
```

And I want to use *Physical id 0*, so I defined DEVICE_ID to 1 and SUBDEV_ID to 0.

The color of the left block will be updated every REFRESH_INTERVAL seconds to
completely red if the temperature is above TEMP_HIGH, completely green if it's
below TEMP_LOW. Fading if the temperature is between those two values.

Then:

`$ make sensors_sample`

And (as root):

`# ./sensors_sample`


To close the program send a INT signal (Control-C).


Original text
================


This is a proof of concept to get MSI keyboard light working on unix and OSX (hackintosh). It works on a MSI GT60 so I think this can activate backlight led keyboard on series GT and GE that have the same keyboard by steelseries.
The attached source is for compile on Mac if you want to compile on Unix I attached in https://dl.dropbox.com/u/5977601/MSI_GT_GE_Led_Enabler-master-unix.tar.gz the modifications needed. Also you need to get dependencies:


Thanks to Signal11 for their HIDAPI.
Thanks to PaNaVTEC for the original Source Code ( https://github.com/PaNaVTEC/MSI_GT_GE_Led_Enabler )
