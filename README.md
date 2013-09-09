TARDIS Sound Controller
=======================
This code is designed to work with the Adafruit WAVE shield and the SparkFun MMA8452 Breakout board accelerometer. It is designed to sense movement (aka traveling) and to play the TARDIS materialization sound on a loop while moving. Otherwise, it will play the interrior hum if no movement is sensed.

Pin Break-Out
=======================
* MMA8452 Breakout -------- Arduino
* 3.3V -------------------------- 3.3V
* SDA -------^^(330)^^------- A4
* SCL -------^^(330)^^------- A5
* GND ------------------------- GND

DISCLAIMER
=======================
THIS SOFTWARE IS MADE AVAILABLE "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OF CONDITION, UNINTERRUPTED USE, MERCHANTABILITY, FITNESS FOR A PARTICULAR USE, OR NON-INFRINGEMENT.

License
=======================
This hardware and software is released under...

Creative Commons Share Alike v3.0 License http://creativecommons.org/licenses/by-sa/3.0/

You are free to use this code and/or modify it. All I ask is an attribution, including supporting libraries and their respective authors used in this software.

Required Libraries
=======================
* Timer1 (http://playground.arduino.cc/code/timer1)
* Arduino WaveHC (http://learn.adafruit.com/adafruit-wave-shield-audio-shield-for-arduino/download)
* Wire (included with Arduino IDE)
