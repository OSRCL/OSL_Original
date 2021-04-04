/*----------------------------------------------------------------------*
 * Arduino Button Library v1.0                                          *
 * Jack Christensen Mar 2012                                            *
 *                                                                      *
 * This work is licensed under the Creative Commons Attribution-        *
 * ShareAlike 3.0 Unported License. To view a copy of this license,     *
 * visit http://creativecommons.org/licenses/by-sa/3.0/ or send a       *
 * letter to Creative Commons, 171 Second Street, Suite 300,            *
 * San Francisco, California, 94105, USA.                               *
 *----------------------------------------------------------------------*/
#ifndef OSL_Button_h
#define OSL_Button_h
#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif
class OSL_Button
{
    public:
        OSL_Button() {}; 
        
        void begin (uint8_t pin, boolean puEnable, boolean invert, uint32_t dbTime); 	// pin, pull-up enable, inverted, debounce time
        uint8_t read();
        uint8_t isPressed();
        uint8_t isReleased();
        uint8_t wasPressed();
        uint8_t wasReleased();
        uint8_t pressedFor(uint32_t ms);
        uint8_t releasedFor(uint32_t ms);
        uint32_t lastChange();
    
    private:
        uint8_t _pin;           //arduino pin number
        boolean _puEnable;      //internal pullup resistor enabled
        boolean _invert;        //if false, interpret high state as pressed, else interpret low state as pressed
        uint8_t _state;         //current button state
        uint8_t _lastState;     //previous button state
        uint8_t _changed;       //state changed since last read
        uint32_t _time;         //time of current state (all times are in ms)
        uint32_t _lastChange;   //time of last state change
        uint32_t _dbTime;       //debounce time
};
#endif