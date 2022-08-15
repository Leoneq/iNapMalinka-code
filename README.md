# iNapMalinka-code
Source code of [iNap Malinka](https://github.com/Leoneq/iNapMalinka). 
The code **and only the code** is shared under GNU GPL v3.0, due to open source libraries which are used:
* [fbcp-ili9341](https://github.com/juj/fbcp-ili9341)
* [RetroPie](https://retropie.org.uk)
* [bcm2835 C library](http://www.airspayce.com/mikem/bcm2835/)

# Setting up a fresh system
If for any reason you want to install fresh system and not use the OS picture I provided, this guide should help you.
## Choosing the OS
The most suitable OS would be RetroPie, although it's possible to install [Raspbian](https://www.raspberrypi.com/software/operating-systems/) or other OS. If you don't want to run custom scripts, you may want to take look at [Lakka](https://www.lakka.tv). In this guide we will use RetroPie as it runs both games *and* custom scripts, and already has builtin many benefits like RetroArch.

Get an image from [here](https://retropie.org.uk/download/) and choose the option for your Pi (probably Zero 2W). Install it on your SD card as you always do, for example with [balenaEtcher](https://www.balena.io/etcher/).

## Enabling SSH
As soon as you installed the system on your SD card, don't insert it in in the RPi yet! Copy two files, `ssh` (note tha lack of extension) and `wpa_supplicant.conf` to the boot partition. Remember to change your country, ssid and password as needed!

![image](https://user-images.githubusercontent.com/36605644/183885306-a4845190-8052-432f-99c8-e1fabf36dc18.png)

![image](https://user-images.githubusercontent.com/36605644/183885343-3791a66a-d178-4ba4-b2a3-7f314704fde1.png)

Insert the card into Pi, and after few minutes it should be visible in your local net (you can either log into your router and see connected devices, or use an [IP Scanner](https://www.advanced-ip-scanner.com)). 
Use any SSH of your choice: PuTTy, Bitvise SSH or Windows builtin SSH. 
Log in with password "raspberry", and change it if you want. You should see a screen like this:

![image](https://user-images.githubusercontent.com/36605644/174088933-b08761f4-0888-4aaa-b807-c0eca600544d.png)

## config.txt
In this file we will activate following things:
* auxiliary SPI with two devices,
* disable main SPI,
* change the right screen resolution
* enable PWM audio
* disable unnecessary things
Open the file with `sudo nano /boot/config.txt` or open it manually.

### Enabling SPI
The auxiliary SPI is used for the radio and ADC converter. Add `dtoverlay=spi1-2cs` and **make sure to comment** `#dtparam=spi=on`.
We will use custom SPI driver for that.
### Changing screen resolution
Add following lines:
```
hdmi_force_hotplug=1
hdmi_group=2
hdmi_mode=87
hdmi_cvt=480 320 60 1 0 0 0 
```
First line forces HDMI output even if nothing's connected physically. Second and third line forces "blank" display mode, which we define as 480x320@60fps picture.
### Enabling PWM audio
Malinka utilizes a similar DAC design as the original Pi does.
```
gpio=12,13=a0
dtparam=audio=on
audio_pwm_mode=2
dtoverlay=audremap,pins_12_13
disable_audio_dither=1
#enable_audio_dither=1
```
We tell the system to switch pins 12 and 13 into their PWM modes. We enable (pwm) sound, and disable audio dither to prevent a "hiss" sound: if you feel confident, you can comment "disable" line and activate "enable" line too.
### Enabling the touch screen
Paste following lines:
```
force_eeprom_read=0
dtparam=i2c1=on
dtparam=i2c_arm=on
dtoverlay=malinkats
```
The first line tells Raspberry to not look for any shield eeprom. Then we activate the I2C interface, and we load a custom overlay. 

Save with CTRL+S, and exit with CTRL+X. Reboot the system (`sudo reboot now`) and type in `ls /dev/ | grep spi`. It should show two SPI devices:

![image](https://user-images.githubusercontent.com/36605644/174094449-c7a09ce8-30f8-4da0-b5c7-4ad2e2984d57.png)

If you type in `dmesg | grep ft`, you should see a loaded touchscreen driver.

![image](https://user-images.githubusercontent.com/36605644/184629129-2d567e26-37fc-4030-9529-de09cf264bfa.png)




# Building
```
cd ~
git clone https://github.com/Leoneq/iNapMalinka-code.git
cd iNapMalinka-code
# TODO!!!!
```

The main project found [here](https://github.com/Leoneq/iNapMalinka) is licensed under CC BY-NC-SA 4.0!
