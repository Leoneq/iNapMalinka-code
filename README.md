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

**Do not forget** to update your system!
```
sudo apt-get update
sudo apt-get upgrade
```

## config.txt
In this file we will activate following things:
* auxiliary SPI with two devices,
* disable main SPI,
* change the right screen resolution
* enable PWM audio
* disable unnecessary things
Open the file with `sudo nano /boot/config.txt` or open it manually. (or just skip and copy-paste the file from the repo)

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

## Installing the joystick driver
First, clone and configure the code repository:
```
sudo apt-get install cmake
cd ~
git clone https://github.com/Leoneq/iNapMalinka-code.git
cd iNapMalinka-code/driver
tar zxvf bcm2835-1.66.tar.gz
cd bcm2835-1.66/
patch -p1 < ../bcm2835-1.66-aux-spi.patch
# (Thanks Doug for the patch! https://groups.google.com/g/bcm2835/c/NHbYW9V2vmQ)
./configure
make
sudo make check
sudo make install
cd ..
make
```
As a result, you will get two executable iles: `malinkabtn` and `malinkabtn-sans`, which is a special branch of the driver made just to fight sans.
For the first setup, let's stick with the main `malinkabtn`, and test it with `sudo ./malinkabtn`:
![image](https://user-images.githubusercontent.com/36605644/184636035-f2a6850e-3706-4112-b406-542449194325.png)
Exit with CTRL+C. 

## Installing the screen driver
I really recommend to use fbcp-ili9341 instead of the old, default fbcp. The author made a really blazing-fast driver, pushing the display to it's limits. To install it, follow the code:
```
cd ~
git clone https://github.com/juj/fbcp-ili9341.git
cd fbcp-ili9341
mkdir build
cd build
```
Now, to show a battery overlay, simply put the custom file `low_battery.cpp` and replace it in the source folder. Build the driver:
```
cp ~/iNapMalinka-code/fbcp-ili9341/low_battery.cpp ~/fbcp-ili9341/low_battery.cpp
cmake -DILI9488=ON -DGPIO_TFT_DATA_CONTROL=24 -DGPIO_TFT_RESET_PIN=25 -DGPIO_TFT_BACKLIGHT=26 -DBACKLIGHT_CONTROL=ON -DSTATISTICS=0 -DSPI_BUS_CLOCK_DIVISOR=10 -DDISPLAY_CROPPED_INSTEAD_OF_SCALING=ON .. 
make -j
```
and you can test if it works, by running `sudo ./fbcp-ili9341`. If the screen flickers or shows artifacts, change the `DSPI_BUS_CLOCK_DIVISOR=10` to higher values, but remember that it must be an even number. More info in the [main repo](https://github.com/juj/fbcp-ili9341).

## Auto startup
The screen driver and joystick driver must load at every boot of the system automatically. You can either leave the executable files in their folders, or make a new folder, move them and delete the source files. 
```
mkdir ~/drivers
cp /home/pi/fbcp-ili9341/build/fbcp-ili9341 /home/pi/drivers/fbcp-ili9341
cp /home/pi/iNapMalinka-code/driver/malinkabtn /home/pi/drivers/malinkabtn
# OPTIONALLY: this will remove the repositories permanently!
rm -rf /home/pi/fbcp-ili9341
rm -rf /home/pi/iNapMalinka-code
```
Open the startup file with `sudo nano /etc/rc.local` and paste following lines:
```
sudo /home/pi/fbcp-ili9341/build/fbcp-ili9341 &
sudo /home/pi/iNapMalinka-code/driver/malinkabtn &
```

## Custom script run from RetroPie
There are two ways in RetroPie to run custom scripts. Your .sh file can be put directly into `~/RetroPie/retropiemenu` to show as a script, or you can make your own emulationstation 'system'. Open the ES systems file with `sudo nano /etc/emulationstation/es_systems.cfg` and add the following system:
```  
<system>
    <name>apps</name>
    <fullname>Applications</fullname>
    <path>/home/pi/RetroPie/roms/apps</path>
    <extension>.sh .SH</extension>
    <command>sudo./%BASENAME%.sh</command>
    <platform>default</platform>
    <theme>default</theme>
</system>
```
Now, you can put your .sh scripts into ``/home/pi/RetroPie/roms/apps`` even through Samba!

## Installing the X server
To run graphical applications (that utilizes touchscreen, and colorful LCD) instead of normal console apps, you will need an X server. You can install it with `sudo apt-get install xserver-xorg` and run with `Xorg -config /etc/X11/X.conf :3` - but remember that this is just the X server, so you don't have even a window manager and other "x" stuff. But this is just enough for running embedded applications! Take a look at the example script to see how I did a GUI application. 

## Increasing SWAP size
This step isn't necessary but 256MB of ram isn't really much for C compiling, if one was about to build i. e. Xash3D or other emu from source.
```
sudo dphys-swapfile swapoff
sudo nano /etc/dphys-swapfile
```
Change the `CONF_SWAPSIZE` to higher value, such as 512MB or 1024MB (remember that higher values may shorten the life od SD card significantly)
```
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
```
## Enabling the equalizer
In order to improve the audio output, you may want to enable the audio equalizer. Install the alsa plugin with `sudo apt-get install -y libasound2-plugin-equal`, and edit the setting file `sudo nano ~/.asoundrc` - paste following content (assuming that headphones are device nr. 1)
```
pcm.!default {
  type plug
  slave.pcm plugequal;
}
ctl.!default {
  type hw card 1
}
ctl.equal {
  type equal;
}
pcm.plugequal {
  type equal;
  slave.pcm "plughw:1,0";
}
pcm.equal {
  type plug;
  slave.pcm plugequal;
}
```
And reload the alsa with `alsactl kill rescan`. You can play an example sound with `speaker-test -t sine -f 400`, or `speaker-test -t wav -c 6` to play sine wave or a voice clip respectively. Alternatively, run a game in the background while adjusting the audio properties. Open the equalizer with `alsamixer -D equal`. A proposed configuration:

![image](https://user-images.githubusercontent.com/36605644/186242154-bb422139-986e-4c70-9d86-3458f4419cfa.png)

# Other useful stuff
(needs cleaning!)
### Overlay
```
dtc -@ -I dts -O dtb -o malinkats.dtbo malinkats.dts
sudo cp malinkats.dtbo /boot/overlays/malinkats.dtbo
```
### Installing chrome
```
sudo apt-get install chromium-browser omxplayer libgnome-keyring-common libgnome-keyring0 libnspr4 libnss3 xdg-utils matchbox xorg gconf-service libgconf-2-4 rpi-chromium-mods libwidevinecdm0
```
### example script to run chromium in kiosk mode
```
echo "hi"
sudo screen -dmS X Xorg -config /etc/X11/X.conf -nocursor :3
export DISPLAY=:3
chromium-browser --kiosk --app=https://jcw87.github.io/c2-sans-fight/ --start-fullscreen
```
### installing retropie extra
```
cd ~
git clone https://github.com/zerojay/RetroPie-Extra.git
cd RetroPie-Extra/
./install-extras.sh
```
### installing nuklear
```
wget https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/master/nuklear.h
sudo apt-get install libglfw3 libglfw3-dev
sudo apt-get install libglew-dev
```
### turning off the act led
```
echo 0  | sudo tee /sys/class/leds/led0/brightness
```
### alsa
```
aplay something.wav
alsamixer
alsamixer -D equal
aplay -l
#lists all audio devices
```
### turning off an input device
```
evtest --grab /dev/input/event0 > /dev/null
```


The main project found [here](https://github.com/Leoneq/iNapMalinka) is licensed under CC BY-NC-SA 4.0!
