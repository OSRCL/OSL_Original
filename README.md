# Open Source RC Lights
(aka, "OSL")

![OSL Board](http://openpanzer.org/images/osl/OSL_gh.jpg "Assembled OSL Board")

**[COMPLETE INSTRUCTIONS AVAILABLE HERE](http://www.rcgroups.com/forums/showthread.php?t=1539753)**

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
  * Compatible with standard aftermarket RC LED lights sush as the **[YeahRacing lights](http://www.rcmart.com/body-light-font-colorredblight-rcbfont-c-438_900.html)**. 

There are no pre-assembled boards available for sale at this time, but you can buy bare PCBs **[from OSH Park](https://oshpark.com/shared_projects/kmCzNipk)**. 

A bill of materials is included in the Hardware folder. 

Please **[see the RCGroups thread](http://www.rcgroups.com/forums/showthread.php?t=1539753)** for more information. 


## How to Use with Arduino
Download the source files. You can obtain them from the first post of the [RCGroups thread](http://www.rcgroups.com/forums/showthread.php?t=1539753) or click the green "Clone or Download" button at the top of this page, then select "Download ZIP." Unzip the files, then:

1. Put the folder named "OpenSourceLights" into your Arduino "Sketches" folder. This may be a location you specified or it may be the default location that Arduino chose when you installed the IDE. If you are unsure where Arduino thinks your Sketches folder is:
   - Open the Arduino IDE
   - Go to File -> Preferences
   - At the top of the popup screen that appears, look at the path under the heading "Sketchbook location". This shows you where your Sketches folder is. 

2. Also put the folder named "libraries" into your Sketches folder. There may already be a libraries folder there, if so just overwrite it (overwriting in this case will simply add the new libraries).
3. Compile and upload
   - Open the Arduino IDE
   - Make sure you have the correct board selected. Go to Tools -> Board -> "Arduino Duemilanove or Diecimila"
   - Make sure you have the correct processor selected. Go to Tools -> Processor -> "ATmega328"
   - Make sure you have the correct com port selected (the one hooked up to your board) - this is in Tools -> Serial Port
   - Click the "Upload" button to send the code to the board (this is the icon with the arrow facing right)
