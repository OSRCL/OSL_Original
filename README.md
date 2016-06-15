# Open Source RC Lights
(aka, "OSL")

![OSL Board](http://openpanzer.org/images/osl/OSL_gh.jpg "Assembled OSL Board")

OSL is an Arduino-based, open source project for controlling lights in RC cars and trucks. It uses an ATmega 328 as the processors and has 8 light outputs. All the components on the board are through-hole which makes it easy to assemble and solder. 
 
Some features: 
  * Control up to 8 light circuits
  * Reads throttle, steering, and an optional third channel which can be a 2, 3, or 5 position switch. 
  * Each light can respond to various "States": forward, reverse, stopped, stopped after delay, accelerating, decelerating, braking, right turn, left turn, and any of up to 5 positions for the channel 3 switch if attached. 
  * For each light in each State you can set the value you want it to be: On, Xenon-on effect, Off, Fade off effect, Blink, Soft Blink, Fast Blink, Backfire effect, or Dim
  * The collection of all Settings for all States for all lights is called a "Scheme". You can have as many Schemes as you like and you can switch between them from your controller using nothing but the steering channel. 
  * All Settings/States/Schemes are set in the Arduino sketch, but are in tabular format and there is an example already included so it is easy to create your own. 
  * Code can be written in the Arduino IDE and flashed to the board with any of the common FTDI cables or adapters.
  * Compatible with both 5 and 6-volt RC systems, or the lights can be powered by a separate voltage source.
  * Compatible with standard aftermarket RC LED lights sush as those sold by YeahRacing. 

There are no pre-assembled boards available for sale at this time, but you can buy bare PCBs [from OSH Park](https://oshpark.com/shared_projects/kmCzNipk). 

A bill of materials is included in the Hardware folder. 

Please [see the RCGroups thread](http://www.rcgroups.com/forums/showthread.php?t=1539753) for more information. 
