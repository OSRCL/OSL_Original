#include "OSL_Settings.h"



// Function to print out light settings names
const __FlashStringHelper *ptrLightSetting(char setting) 
{
	if(setting>LAST_LIGHT_SETTING) setting=LS_UNKNOWN;
	const __FlashStringHelper *Names[LAST_LIGHT_SETTING+1]={F("Off"), F("On"),F("N/A"),F("Blink"),F("Blink Alt"),
															F("Fast Blink"),F("Fast Blink Alt"),F("Soft Blink"),
														    F("Dim"),F("Fade-off"),F("Fade-on"),F("Xenon"),F("Backfire"),
															F("Safety Blink"), F("Safety Blink Alt"),
															F("Unknown")};
	return Names[setting];
};

// Same thing, but these are the official capitalized versions
const __FlashStringHelper *ptrLightSettingCap(char setting) 
{
	if(setting>LAST_LIGHT_SETTING) setting=LS_UNKNOWN;
	const __FlashStringHelper *Names[LAST_LIGHT_SETTING+1]={F("OFF"), F("ON"),F("NA"),F("BLINK"),F("BLINK_ALT"),
															F("FASTBLINK"),F("FASTBLINK_ALT"),F("SOFTBLINK"),
														    F("DIM"),F("FADEOFF"),F("FADEON"),F("XENON"),F("BACKFIRE"),
															F("SAFETYBLINK"), F("SAFETYBLINK_ALT"),
															F("UNKNOWN")};
	return Names[setting];
};

// Function to help us print out actual drive mode names, rather than numbers. 
// To use, call something like this:  Serial.print(printMode(DriveModeCommand));
const __FlashStringHelper *printMode(char mode) 
{
	if(mode>LAST_MODE) mode=UNKNOWN;
	const __FlashStringHelper *Names[LAST_MODE+1]={F("UNKNOWN"),F("STOP"),F("FORWARD"),F("REVERSE")};
	return Names[mode];
};


const __FlashStringHelper *printState(char state)     
{
	if(state>LAST_STATE) state=State_Unknown;
	const __FlashStringHelper *Names[LAST_STATE+1]={F("Pos 1"),F("Pos 2"),F("Pos 3"),F("Pos 4"),F("Pos 5"),F("Forward"),
													F("Reverse"),F("Stop"),F("Stop Delay"),F("Brake"),F("Right Turn"),
													F("Left Turn"),F("No Turn"),F("Accelerate"),F("Decelerate"),F("Unknown")};
	return Names[state];
};

// Function to help us print out actual radio state names, rather than numbers. 
// To use, call something like this:  Serial.print(printRadioState(RC_State));
const __FlashStringHelper *printRadioState(char rcstate) {
	if (rcstate>LAST_RC_STATE) rcstate = RC_SIGNAL_UNINITIALIZED;
	const __FlashStringHelper *StateNames[LAST_RC_STATE+1]={F("UNINITIALIZED"),F("Acquiring"),F("Connected"),F("Disconnected")};
	return StateNames[rcstate];
};