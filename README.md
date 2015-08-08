MSI GE60 Apache Pro
=====================

This is a modded code of PaNaVTEC's MSI_GT_GE_Led_Enabler to compile in Linux Debian.

And edited by nake (Alfonso Arbona Gimeno) to work with MSI GE60 in Debian, adding
also new features stolen from https://github.com/Wufe/msi-keyboard

How to compile:
===============

It's simple firs tclone the repository with 

$ git clone https://github.com/nake90/MSI_GT_GE_Led_Enabler

Edit msiledenabler.cpp (I just created a sample code)

Now in a terminal type:

$ make
$ sudo ./msiledenabler

Remember that you need root access!


Original text
================


This is a proof of concept to get MSI keyboard light working on unix and OSX (hackintosh). It works on a MSI GT60 so I think this can activate backlight led keyboard on series GT and GE that have the same keyboard by steelseries.
The attached source is for compile on Mac if you want to compile on Unix I attached in https://dl.dropbox.com/u/5977601/MSI_GT_GE_Led_Enabler-master-unix.tar.gz the modifications needed. Also you need to get dependencies:


Thanks to Signal11 for their HIDAPI.
Thanks to PaNaVTEC for the original Source Code ( https://github.com/PaNaVTEC/MSI_GT_GE_Led_Enabler )
