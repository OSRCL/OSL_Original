/* OSL_Settings.h   Settings file - a header file that defines many of the hardware elements of the OSL board
 * Source:          https://github.com/OSRCL/OSL_Original
 * Authors:         Luke Middleton
 *
 * These values should not need to be modified by general users. 
 *   
 */ 


#ifndef OSL_SETTINGS_H
#define OSL_SETTINGS_H

#include <Arduino.h>


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// OPEN SOURCE LIGHTS (OSL) FIRMWARE VERSION NUMBER
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    #define FIRMWARE_VERSION       "0.5.03"       	    	// version. Last update 11/8/2023
    


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// PINS & HARDWARE DEFINITIONS
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>	
	#define NumLights				      8					// How many light outputs do we have on this board



// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// FIRMWARE DEFINITIONS
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
	// There are 15 possible states a light can be in: 
	// - Mode 1, Mode 2, Mode 3, Mode 4, Mode 5 (all from Channel3 switch), 
	// - Forward, Reverse, Stop, Stop Delay, Brake (from Throttle Channel), 
	// - Right Turn, Left Turn, No Turn (from Turn Channel)
	// - Accelerating - special state that occurs on heavy acceleration (from Throttle channel)
	// - Decelerating - special state that occurs on heavy deceleration (from Throttle channel)
	#define NumStates                    15         		// Number of possible states
	// State definitions
	#define Mode1                         0         		// Channel 3 in 1st position
	#define Mode2                         1         		// Channel 3 in 2nd position
	#define Mode3                         2         		// Channel 3 in 3rd position
	#define Mode4                         3         		// Channel 3 in 4th position
	#define Mode5                         4         		// Channel 3 in 5th position        
	#define StateFwd                      5         		// Moving forward
	#define StateRev                      6         		// Moving backwards
	#define StateStop                     7         		// Stopped
	#define StateStopDelay                8         		// Stopped for a user-defined length of time
	#define StateBrake                    9         		// Braking
	#define StateRT                      10         		// Right turn
	#define StateLT                      11         		// Left turn
	#define StateNT                      12         		// "Neutral" turn, ie, no turn
	#define StateAccel                   13         		// Acceleration
	#define StateDecel                   14         		// Deceleration
	#define State_Unknown			     15
	#define LAST_STATE	  State_Unknown
	const __FlashStringHelper *printState(char state);      //Returns a character string that is name of the state

	// For every state, each light output can have the following settings. 
	// Giving names to numerical values allows the user to easily create their own light setup
	#define COUNT_SETTINGS				 12
	#define OFF					          0
	#define ON					          1
	#define NA                            2         
	#define BLINK                         3         
	#define FASTBLINK                     4
	#define SOFTBLINK                     5
	#define DIM                           6
	#define FADEOFF                       7
	#define FADEON						  8
	#define XENON                         9
	#define BACKFIRE                     10 
	#define LS_UNKNOWN			         11
	#define LAST_LIGHT_SETTING	 LS_UNKNOWN
	const __FlashStringHelper *ptrLightSetting(char setting); //Returns a character string that is name of the light setting (more friendly name format)
	const __FlashStringHelper *ptrLightSettingCap(char setting); //Same thing, but the official capitalized names
	
	// For debugging, we'd like to print a neat table showing the scheme settings. This array lets us know how many spaces to pad
	// after each name. We are looking for a consistent 14 characters, so the padding is 14 - number of characters in name
	const PROGMEM uint8_t _SettingNamesPadding[COUNT_SETTINGS] = 
	{   11,			// OFF
		12,			// ON
		12,			// NA
		 9,			// BLINK
		 5,			// FASTBLINK
		 5,			// SOFTBLINK
		11,			// DIM
		 7,			// FADEOFF
		 8,			// FADEON
	     9,			// XENON
		 6,			// BACKFIRE
		 7			// UNKNOWN 
	};
	
	
	// These are simplifications of the turn channel state. We have the actual command, but this lets us know simply in which direction is the wheel turned
	#define RIGHT_TURN			          1
	#define NO_TURN					      0
	#define LEFT_TURN			         -1
	                                    
	// Drive mode                     
	#define UNKNOWN      			      0
	#define STOP         			      1
	#define FWD 	     			      2
	#define REV          			      3
    #define LAST_MODE		 		    REV
	const __FlashStringHelper *printMode(char mode);     	//Returns a character string that is name of the drive mode.
	


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// RADIO DEFINITIONS
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
	#define NUM_RC_CHANNELS               3                 // Number of RC channels we can read
	#define PULSE_WIDTH_ABS_MIN         800                 // Absolute minimum pulse width considered valid
	#define PULSE_WIDTH_ABS_MAX        2200                 // Absolute maximum pulse width considered valid
	#define PULSE_WIDTH_TYP_MIN        1000                 // Typical minimum pulse width
	#define PULSE_WIDTH_TYP_MAX        2000                 // Typical maximum pulse width        
	#define PULSE_WIDTH_TYP_CENTER     1500                 // Stick centered pulse width
                                                           
	#define RC_PULSECOUNT_TO_ACQUIRE      5                 // How many pulses on each channel to read before considering that channel SIGNAL_SYNCHED
	#define RC_TIMEOUT_US           100000UL           		// How many micro-seconds without a signal from any channel before we go to SIGNAL_LOST. Note a typical RC pulse would arrive once every 20,000 uS
	#define RC_TIMEOUT_MS               100                 // Here it is again in milliseconds, so RC_TIMEOUT_US / 1000 which is 100 mS which is 1/10th of a second

    #define COMMAND_MAX_FORWARD         100					// We are ultimately going to change the throttle and steering pulses into 
    #define COMMAND_MAX_REVERSE        -100					// a more convenient -100/100 value range

	#define BLINK_RATE_LOST_SIGNAL       50     		   	// How fast should we blink the lights when the radio signal is lost
	
	// RC state machine
	#define RC_SIGNAL_UNINITIALIZED       0
	#define RC_SIGNAL_ACQUIRE             1
	#define RC_SIGNAL_SYNCHED             2
	#define RC_SIGNAL_LOST                3
	#define LAST_RC_STATE    RC_SIGNAL_LOST
	const __FlashStringHelper *printRadioState(char rcstate);  //Returns a character string that is name of the radio state.     

	// Position defines for Channel 3 switch (can be up to 5 positions)
	#define Pos1                          0                  
	#define Pos2                          1
	#define Pos3                          2
	#define Pos4                          3
	#define Pos5                          4

	// Shelf queen Channel 3 position number
	#define ShelfQueenCh3Position		  0


	#define FWD_to_REV_BrakeTime		200					// If DoubleTapReverse = true, we treat the first tap into reverse from forward as a brake command. However, depending on how fast the user moves 
															// from forward to reverse, and when the radio signal is actually read, OSL may show it as a process from forward, to stop, and then to reverse. 
															// This define sets the maximum amount of time from the last forward command to a reverse command that we will count as a brake signal on the first
															// tap into reverse. Should be small, we don't want the first transition from stop to reverse to show up as a brake command. 


// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// PINS 
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
	// Note that the six Arduino analog pins can be referred to by numbers 14-19

	// Hardware check pins - the firmware will read the values of these pins to determine the hardware version
		#define pin_VCHECK_A		      7					// Input    - version check pin A
		#define pin_VCHECK_B		  	  8					// Input	- version check pin B
		
	// HARDWARE VERSION 1
	// --------------------------------------------------------------------------------------------------------------------------------------------------->>
    // This includes all original through-hole OSL boards up to and including version v1.7, plus user boards based on that design, which include the
	// three SMD boards designed by irun4fundotca, supercaby, and learningarduino 
	// Radio channels
        #define pin_HW1_Throttle		  2         		// Input    - Pin for throttle channel 
        #define pin_HW1_Steering		 17	 				// Input    - Pin for steering channel
		#define pin_HW1_Ch3				  4					// Input    - Pin for channel 3
		            
    // Light outputsHW1_		
        #define pin_HW1_Light1            9       			// Output   - Light 1
        #define pin_HW1_Light2           10       			// Output   - Light 2
        #define pin_HW1_Light3           11      			// Output   - Light 3
        #define pin_HW1_Light4            6      			// Output   - Light 4
        #define pin_HW1_Light5            5      			// Output   - Light 5
		#define pin_HW1_Light6            3      			// Output   - Light 6
		#define pin_HW1_Light7           15       			// Output   - Light 7
		#define pin_HW1_Light8           16       			// Output   - Light 8
		            
	// Other		
        #define pin_HW1_GreenLED         18       			// Output   - on-board Green LED (this is the same as saying pin A4)
        #define pin_HW1_RedLED           19       			// Output   - on-board Red LED (this is the same as saying pin A5) 
		#define pin_HW1_SetupButton      14       			// Input    - on-board push button (this is the same as saying pin A0) 
		            

	// HARDWARE VERSION 2
	// --------------------------------------------------------------------------------------------------------------------------------------------------->>
	// For a future OSL board design
    // Radio channels
        #define pin_HW2_Throttle		  2         		// Input    - Pin for throttle channel 
        #define pin_HW2_Steering		 17	 				// Input    - Pin for steering channel
		#define pin_HW2_Ch3				  4					// Input    - Pin for channel 3
		              
    // Light outputsHW2_		
        #define pin_HW2_Light1            9       			// Output   - Light 1
        #define pin_HW2_Light2           10       			// Output   - Light 2
        #define pin_HW2_Light3           11      			// Output   - Light 3
        #define pin_HW2_Light4            6      			// Output   - Light 4
        #define pin_HW2_Light5            5      			// Output   - Light 5
		#define pin_HW2_Light6            3      			// Output   - Light 6
		#define pin_HW2_Light7           15       			// Output   - Light 7
		#define pin_HW2_Light8           16       			// Output   - Light 8
		              
	// Other		  
        #define pin_HW2_GreenLED         18       			// Output   - on-board Green LED (this is the same as saying pin A4)
        #define pin_HW2_RedLED           19       			// Output   - on-board Red LED (this is the same as saying pin A5) 
		#define pin_HW2_SetupButton      14       			// Input    - on-board push button (this is the same as saying pin A0) 



// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// EEPROM macros
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    #define eeprom_read_to(dst_p, eeprom_field, dst_size) eeprom_read_block(dst_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(dst_size, sizeof((__eeprom_data*)0)->eeprom_field))
    #define eeprom_read(dst, eeprom_field) eeprom_read_to(&dst, eeprom_field, sizeof(dst))
    #define eeprom_write_from(src_p, eeprom_field, src_size) eeprom_write_block(src_p, (void *)offsetof(__eeprom_data, eeprom_field), MIN(src_size, sizeof((__eeprom_data*)0)->eeprom_field))
    #define eeprom_write(src, eeprom_field) { typeof(src) x = src; eeprom_write_from(&x, eeprom_field, sizeof(x)); }
    #define MIN(x,y) ( x > y ? y : x )
    #define MAX(x,y) ( x > y ? x : y )



// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SIMPLE TIMER
// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
    // We use the OSL_SimpleTimer class for convenient timing functions throughout the project, it is a modified and improved version 
    // of SimpleTimer: http://playground.arduino.cc/Code/SimpleTimer
    // The class needs to know how many simultaneous timers may be active at any one time. We don't want this number too low or operation will be eratic, 
    // but setting it too high will waste RAM. Each additional slot costs 19 bytes of global RAM. 
	// Timers used by this project: 
	// - Turn from start continue (used for both forward and reverse, but will not occur at the same time) = 1 timer (no timer ID)
	// - BackfireTimerID, OvertakeTimerID - by definition will never occur at the same time = 1 timer
	// - CSM_TurnTimerID - used to enter change-scheme-mode, but not again while in it = 1 timer
	// - CSM_BlinkOffTimerID - used during change-scheme-mode = 1 timer
	// - Blink all lights to indicate the currently-selected scheme in change-scheme-mode = 1 timer
	// - RxSignalLostTimerID - used to flash the failsafe lights = 1 timer
	// - StartWaiting (time's up) timer - used during radio setup only, otherwise assigned to one of the above IDs = 1 timer
	// That makes 7 total, but the maximum that should ever be active at once is really only about 2. We give ourselves 5 to have plenty of 
	// leeway, but if you need to save a few bytes you can drop this number. 
    
	#define MAX_SIMPLETIMER_SLOTS         5



// ------------------------------------------------------------------------------------------------------------------------------------------------------->>
// SERIAL
// ------------------------------------------------------------------------------------------------------------------------------------------------>
    #define BaudRate                 38400          // This is the default baud rate for communication with the computer. 



#endif


