arduino-hamradio-watch
======================
          OPEN HAM WATCH v. 1.0 - IK8YFW - 2015

This is an OpenSource project based on Arduino platform, for building a smart watch with some
interesting functions: OLED Display, clock RTC, Satellite pass prediction routine, Moon phase 
routine, multi Alarms user and Satellite approaching alert, power safe, DST, Timezone and dual time.

This watch with use a Monochrome OLEDs based on SSD1306 drivers 128x64 size display using I2C 
to communicate.

The compiled code size is about 31kb, you need capable arduino device with at least 32kb of memory.
All the Arduino Nano card will work well.

Blocks schematic diagram to: https://sites.google.com/site/ik8yfw/projects-experiments

NOTE: This software use the Adafruit SSD1306 and GFX dispay libraries to connect the oled display. 
These libreries can be downloaded from Adafruit site.

TO build the software you must download the SSD1306 and GFX libray from adafruit:

SSD1306: https://github.com/adafruit/Adafruit_SSD1306 GFX: https://github.com/adafruit/Adafruit-GFX-Library

[UPDATE - 12.10.2024]  Add the folder libraries that contains the right versions to allow the correct compilation.
                       Versions are:  Adafruit-GFX-Library-1.1.1  and   Adafruit_SSD1306-1.0.0
                       The code was successfull compiled with Arduino IDE 2.x on Arduino NANO 328P board.
                       The output compilation returns this output:
                       
                       "Sketch uses 29292 bytes (95%) of program storage space. Maximum is 30720 bytes.
                        Global variables use 1711 bytes (83%) of dynamic memory, leaving 337 bytes for local variables. Maximum is 2048 bytes. "


Author: GIUSEPPE CALLIPO - ik8yfw[AT]libero.it License: The software is released under Creative Commons (CC) license.
