/* OSL_LedHandler.h     Led Handler - class for handling LEDs, requires use of elapsedMillis
 * Source:              https://github.com/OSRCL
 * Authors:             Luke Middleton
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */  

#ifndef OSL_LedHandler_h
#define OSL_LedHandler_h

#include <Arduino.h>
#include "../elapsedMillis/elapsedMillis.h"
#include "../../AA_UserConfig.h"


// These are all the possible states the light can be in, within this class. These are not always strictly the same thing as the states within the sketch,
// and an output may go though multiple states to get to the state the sketch wants (transition states)
#define LED_STATE_OFF					      0	
#define LED_STATE_ON					      1	
#define LED_STATE_DIM					      2	
#define LED_STATE_BLINK				   	      3				// Applies to both Blink and FastBlink
#define LED_STATE_SOFTBLINK			          4	
#define LED_STATE_RANDOMBLINK			      5	
#define LED_STATE_FADE				   	      6	
#define LED_STATE_XENON				          7	
#define LED_STATE_FADE_TO				      8	
#define LED_STATE_SAFETYBLINK				  9

// Stream blinker struct
#define MAX_STREAM_STEPS                    10            	// A stream consists of a pattern of on/off blinks separated by user-specified lengths of time. A single blink (on/off) takes 2 steps. 
typedef struct                                          	// This struct holds an array of blink patterns, and a flag to indicate if it should repeat or not
{
    uint16_t        interval[MAX_STREAM_STEPS];
    boolean         repeat;
	boolean			altBlink;
} BlinkStream;

#define DEFAULT_BLINK_INTERVAL              378				// Used when an interval is not specified, though OSL always will

#define MIN_PWM							      0				// PWM value at Off
#define MIN_PWM_FLOAT					    0.0
#define MAX_PWM							    255				// PWM value at On
#define MAX_PWM_FLOAT				      255.0	

// Fading - You really probably shouldn't change any of this! These are the settings that work best with the hardcoded processes in the cpp file. 
#define FADE_IN                               1
#define FADE_OUT                              2
#define FADE_TYPE_EXP				          0				// There are two types of fades, but for purposes of RC cars exponential looks better (and sometimes we modify even that)
#define FADE_TYPE_SINE					      1				// I don't believe we end up using the sine fade anywhere
															// For exponential fades the formula for fade-outs (decrease brightness) is pwm = priorPWM * Ratio. 
															// For fade-ins (increase brightness) the formula is pwm = priorPWM + ((1-Ratio) * priorPWM)
#define SINE_START_ANGLE_ON			       1.57				// Sine fading is more complicated and involves using angular math, these are the angles on a sine wave
#define SINE_START_ANGLE_OFF		      4.712				// that represent the peak of the curve (on) or the trough (off)
#define NUM_FADE_UPDATES                     50				// Used for fading in or out from full on or off
#define DEFAULT_FADE_TIME				    500				// Length of time for generic fade
#define FADE_OFF_RATIO					    0.9				// We decrease the PWM by this ratio each step of a standard fade
#define FADE_ON_R_VAL				    12.5088				// Used for exponential fade-ins
#define FADE_TO_RATIO			            0.9				// We can have a different fade-out ratio for the fade-to function (fades to a target, rather than full off)

#define XENON_STEP_1_ON_TIME     		     50				// Step 1 in the xenon process - how long to flash the LED at full brightness to start 
#define XENON_STEP_2_DIM_TIME			    100		    	// Step 2 in the xenon process - how long to turn off the light, or set it very dim, after the first flash
#define XENON_STEP_2_DIM_LEVEL			      0				// Step 2 in the xenon process - how bright during this brief interval between the flash and fade in. Set to 0 to keep it off
#define XENON_STEP_3_FADE_TIME			   6000				// Step 3 in the xenon process - how long should the fade-in take. Sergio Pizzotti's original implementation was about 6.3 seconds with the initial flash. 
#define XENON_STEP_3_FADE_STEPS		        150				// Step 3 in the xenon process - how many steps for the fade in. All these numbers work together, and with the actual code in .cpp, so if you change anything, 
															// you will need to change the code as well. See the Excel spreadsheet for what they represent. Best to just leave everything alone. 
											
#define SOFTBLINK_STEP_1_FADE_ON_TIME  	    220		    	// Step 1 in the soft blink process - how long does the initial fade-in take
#define SOFTBLINK_STEP_1_FADE_ON_STEPS	     20	
#define SOFTBLINK_STEP_2_ON_TIME		    100				// Step 2 in the soft blink process - how long to remain at full brightness
#define SOFTBLINK_STEP_3_FADE_OFF_TIME	    416		    	// Step 3 in the soft blink process - how long does the fade-out take
#define SOFTBLINK_STEP_3_FADE_OFF_STEPS      32	
#define SOFTBLINK_FADE_ON_RATIO			   0.24			
#define SOFTBLINK_FADE_OFF_RATIO		   0.84	
#define SOFTBLINK_TO_TARGET_FADEDOWN_ONLY false				// This is one of the few defines you can change without messing anything up. If set to true, when a softblink ends and the next state is dim, 
															// this will cause the blink to stop at the dim level only when fading down. That means if the change to dim occurs when the softblink effect is lower
															// than the desired dim level, the softblink will blink one more time to full brightness and then stop at dim on the way down. 
															// If set to false, it will stop at the desired dim level on either the upswing or down, depending on which happens first. 
															// I think the stop on fadedown only looks better, but you may end up with one extra blink at stop, so if you don't like that set the define to false. 
class OSL_LedHandler
{   public:
        OSL_LedHandler() {}; 
        
        void begin (byte p, boolean i=false, boolean w=false); 					// p = pin, i = invert, w = pwm-able
        void on(void);
		boolean isOn(void);
        void off(void);
        void toggle(void);
		void dim(uint8_t level);												// Level should be between 0-MAX_PWM
        void update(void);                                                      // Update blinking effect
        void Blink(uint16_t interval=DEFAULT_BLINK_INTERVAL);                   // Blinks once at interval specified
        void Blink(uint8_t times, uint16_t interval=DEFAULT_BLINK_INTERVAL);    // Overload - Blinks N times at interval specified (on and off interval will be the same)
		void Blink(uint8_t times, uint16_t on_interval=DEFAULT_BLINK_INTERVAL, uint16_t off_interval=DEFAULT_BLINK_INTERVAL);   // Overload - Blinks N times at intervals specified (on and off time individually set)
        void startBlinking(uint16_t on_interval=DEFAULT_BLINK_INTERVAL, uint16_t off_interval=DEFAULT_BLINK_INTERVAL, boolean alt=false);   // Starts a continuous blink at the set intervals
		void stopBlinking(void);
		void softBlink(void);
        void StreamBlink(BlinkStream bs, uint8_t numSteps);
		void randomBlink(void);
        void Fade(uint8_t fade_in, uint16_t span, char f=FADE_TYPE_EXP);
		void FadeTo(uint8_t desiredLevel);
        void stopFading(void);		
		void Xenon(void);
		void SafetyBlink(uint16_t sbRate, uint8_t sbCount, uint16_t sbInt, boolean alt=false);	
        
		
    private:
		void clearUpdateProcess(void);
        void pinOn(void);
        void pinOff(void);
		void offWithExtra(boolean includeExtra=false);
		void setPWM(float level);
		void changeLEDState(uint8_t changeState);
		void softBlinkWithStartFlag(boolean start=false);
        elapsedMillis   _time;
        byte            _pin;
		boolean			_pwmable;
		float			_pwm;
		boolean			_fadeToTarget; 
		int16_t			_pwmTarget;
        boolean         _invert;
		uint8_t			_fadeType;
		uint8_t         _fadeDirection;		
		uint8_t			_curProcessStep;
		uint8_t			_numProcessSteps;
		boolean		    _AltProcess;
        uint8_t         _curStep;
		uint8_t         _numSteps;
        uint16_t        _nextWait;
        boolean         _fixedInterval;
        BlinkStream     _blinkStream;
		boolean			_blinkToDim;
        float           _fadeAdjustment;
		uint8_t			_ledCurState;
		uint8_t			_ledPriorState;
		uint16_t		_safetyBlinkRate;
		uint8_t			_safetyBlinkCount;
		uint16_t		_safetyBlinkInterval;
		uint16_t		_safetyBlinkCountTimesTwo;
};


#endif 
