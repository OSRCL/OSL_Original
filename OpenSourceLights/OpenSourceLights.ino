 /*
 * Open Source Lights   An Arduino-based program to control LED lights on RC vehicles. 
 * Version:             2.07
 * Last Updated:        10/25/2015
 * Copyright 2011       Luke Middleton
 *
 * For more information, see the RCGroups thread: 
 * http://www.rcgroups.com/forums/showthread.php?t=1539753
 *
 * CREDITS!    CREDITS!    CREDITS!
 *----------------------------------------------------------------------------------------------------------------------------------------------------->
 * Several people have contributed code to this project
 *
 * JChristensen        We are using Christensen's button library unchanged. It has been renamed from Button to OSL_Button simply because there are many Arduino
 *                     button libraries, and we don't want the install of this one to conflict with others you may already have. See JChristensen's project page here:
 *                     https://github.com/JChristensen/Button
 * Sergio Pizzotti     RCGroups username "wormch"
 *                         March 2015 - Made several impressive changes specifically for drift cars. Wrote all the code related to the backfiring and Xenon effects. 
 *                         Made ChangeSchemeMode more user-friendly, it can only be entered after the car has been stopped several seconds. 
 *                         Also fixed some bugs and taught me the F() macro!
 * Patrik              RCGroups username "Orque"
 *                         March 2015 - Expanded the Channel 3 functionality to read up to a 5 position switch (previously only worked to 3 positions)        
 * Jens                RCGroups username "learningarduino"
 *                         October 2014 - Fixed bugs related to pin initialization and debug printing. 
 * Peter               RCGroups username "4x4_RC_Pit"
 *                         September 2014 - Fixed several bugs in the RadioSetup routine. Also the first person to post a video of OSL in action.
 *
 *
 * Open Source Lights is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v3 as published by
 * the Free Software Foundation (http://www.gnu.org/licenses/)
 *
*/


// ====================================================================================================================================================>
//  INCLUDES 
// ====================================================================================================================================================>
    #include "AA_UserConfig.h"
    #include "Defines.h"
    #include <OSL_SimpleTimer.h>
    #include <EEPROM.h>
    #include <OSL_Button.h>    // By JChristensen. See: https://github.com/JChristensen/Button Renamed from Button to OSL_Button simply so it won't conflict with other button libraries. 
    #include <avr/eeprom.h>
    #include <avr/pgmspace.h>

   
// ====================================================================================================================================================>
//  GLOBAL VARIABLES
// ====================================================================================================================================================>
    // Useful names 
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        const int NA                   =    -1;                 // For each of the 8 states the light can have the following settings: On, Off, NA, Blink, FastBlink, or Dim. On/Off are already defined above
        const int BLINK                =    -2;                 // These give us numerical values to these names which makes coding easier, we can just type in the name instead of the number. 
        const int FASTBLINK            =    -3;
        const int SOFTBLINK            =    -4;
        const int DIM                  =    -5;
        const int FADEOFF              =    -6;
        const int XENON                =    -7;
        const int BACKFIRE             =    -8;
        
        const byte ON = 1;
        const byte OFF = 0;
        const byte YES = 1;
        const byte NO = 0;
        const byte PRESSED = 0;                                 // Used for buttons pulled-up to Vcc, who are tied to ground when pressed
        const byte RELEASED = 1;                                // Used for buttons pulled-up to Vcc, who are tied to ground when pressed

    // SIMPLE TIMER 
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        OSL_SimpleTimer                   timer;                 // Instantiate a SimpleTimer named "timer"
        boolean TimeUp                 =  true;

    // STARTUP
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean Startup                =  true;                 // This lets us run a few things in the main loop only once instead of over and over

    // DRIVING
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean Braking                = false;                 // Are we braking            
        boolean Decelerating           = false;                 // Are we sharply decelerating 
        boolean Accelerating           = true;                  // Are we sharply accelerating

        typedef char DRIVEMODES; 
        #define UNKNOWN      0
        #define STOP         1
        #define FWD 	     2
        #define REV          3
        #define LAST_MODE    REV

        const __FlashStringHelper *printMode(DRIVEMODES Type);     //Returns a character string that is name of the drive mode.
        
        // Little function to help us print out actual drive mode names, rather than numbers. 
        // To use, call something like this:  Serial.print(printMode(DriveModeCommand));
        const __FlashStringHelper *printMode(DRIVEMODES Type) {
            if(Type>LAST_MODE) Type=UNKNOWN;
            const __FlashStringHelper *Names[LAST_MODE+1]={F("UNKNOWN"),F("STOP"),F("FORWARD"),F("REVERSE")};
            return Names[Type];
        };


        // Throttle
        int ThrottleCommand            =     0;                 // A mapped value of ThrottlePulse to (0, MapPulseFwd/Rev) where MapPulseFwd/Rev is the maximum FWD/REV speed (100, or less if governed)
        int ThrottlePulse;                                      // Positive = Forward, Negative = Reverse <ThrottlePulseCenter - ThrottlePulseMin> to <0> to <ThrottlePulseCenter + ThrottlePulseMax>
        int ThrottlePulseMin;                                   // Will ultimately be determined by setup procedure to read max travel on stick, or from EEPROM if setup complete
        int ThrottlePulseMax;                                   // Will ultimately be determined by setup procedure to read max travel on stick, or from EEPROM if setup complete
        int ThrottlePulseCenter;                                // EX: 1000 + ((2000-1000)/2) = 1500. If Pulse = 1000 then -500, 1500 = 0, 2000 = 500
        int ThrottleCenterAdjust       =     0;                 // If small throttle commands do not result in movement due to gearbox/track resistance, increase this number. FOR NOW, LEAVE AT ZERO. IF SET, MUST BE SMALLER THAN THROTTLEDEADBAND
        boolean ThrottleChannelReverse;                         // Can be used to reverse the throttle channel if they don't have reversing on radio
        int MaxFwdSpeed                =   100;                 // 
        int MaxRevSpeed                =  -100;                 // 

        // Steering
        boolean SteeringChannelPresent;                         // On startup we check to see if this channel is connected, if not, this variable gets set to False and we don't bother checking for it again until reboot
        int TurnCommand                =     0;                 // A mapped value of ThrottlePulse from (TurnPulseMin/TurnPulseMax) to MaxLeft/MaxRight turn (100 each way, or less if governed)
        int TurnPulse;                                          // Positive = Right, Negative = Left <TurnPulseCenter - TurnPulseMin> to <0> to <TurnPulseCenter + TurnPulseMax>
        int TurnPulseMin;                                       // Will ultimately be determined by setup procedure to read max travel on stick, or from EEPROM if setup complete
        int TurnPulseMax;                                       // Will ultimately be determined by setup procedure to read max travel on stick, or from EEPROM if setup complete
        int TurnPulseCenter;                                    // EX: 1000 + ((2000-1000)/2) = 1500. If Pulse = 1000 then -500, 1500 = 0, 2000 = 500
        int TurnCenterAdjust           =     0;                 // Leave at ZERO for now
        boolean TurnChannelReverse;                             // Can be used to reverse the steering channel if they don't have reversing on radio
        int MaxRightTurn               =   100;                 // 
        int MaxLeftTurn                =  -100;
        boolean TurnSignal_Enable      =  true;                 // If the user decides to restrict turn signals only to when the car is stopped, they can further add a delay that begins
                                                                // when the car first stops, and until this delay is complete, the turn signals won't come on. This flag indicates if the delay
                                                                // is up. Initialize to true, otherwise turn signals won't work until the car has driven forward once and stopped.
        int TurnSignalOverride         =     0;                 // The other situation we want to modify the turn signal is when starting from a stop, while turning. In this case we leave the turn signals on
                                                                // for a period of time even after the car has started moving (typically turn signals are disabled while moving). This mimics a real car where
                                                                // the turn signal isn't cancelled until after the steering wheel turns back the opposite way. In our case we don't check the steering wheel, 
                                                                // we just wait a short amount of time (user configurable in AA_UserConfig.h, variable TurnFromStartContinue_mS)

        // Channel 3
        boolean Channel3Present;                                // On startup we check to see if this channel is connected, if not, this variable gets set to False and we don't bother checking for it again until reboot
        int Channel3Pulse;                                      // 
        int Channel3PulseMin;        
        int Channel3PulseMax;
        int Channel3PulseCenter;
        int Channel3                   =   OFF;                 // State of the Channel 3 switch: On (1), Off (0), Disconnected (-1)
        boolean Channel3Reverse;
        #define Pos1                         0                  // Position defines for Channel 3 switch (can be up to 5 positions)
        #define Pos2                         1
        #define Pos3                         2
        #define Pos4                         3
        #define Pos5                         4
    
    // LIGHTS 
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        int CurrentScheme;                                      // Indicates which scheme is presently selected and active. Number from 1 to NumSchemes. 
                                                                // Note that the actual schemes are zero-based (0 to NumSchemes-1) but don't worry about that,
                                                                // the code takes care of it. 
        #define NumLights                    8                  // The number of light outputs available on the board
        #define NumStates                    13                 // There are 12 possible states a light can be in: 
                                                                // - Mode 1, Mode 2, Mode 3, Mode 4, Mode 5 (all from Channel3 switch), 
                                                                // - Forward, Reverse, Stop, Brake (from Throttle Channel), 
                                                                // - Right Turn, Left Turn (from Turn Channel)
                                                                // - Accelerating - 
                                                                // - Decelerating - special state that occurs on heavy deceleration (from Throttle Channel)
        const byte Mode1               =     0;                 // Channel 3 in 1st position
        const byte Mode2               =     1;                 // Channel 3 in 2nd position
        const byte Mode3               =     2;                 // Channel 3 in 3rd position
        const byte Mode4               =     3;                 // Channel 3 in 4th position
        const byte Mode5               =     4;                 // Channel 3 in 5th position        
        const byte StateFwd            =     5;                 // Moving forward
        const byte StateRev            =     6;                 // Moving backwards
        const byte StateStop           =     7;                 // Stopped
        const byte StateBrake          =     8;                 // Braking
        const byte StateRT             =     9;                 // Right turn
        const byte StateLT             =     10;                // Left turn
        const byte StateAccel          =     11;                // Acceleration
        const byte StateDecel          =     12;                // Deceleration
       
        int ActualDimLevel;                                     // We allow the user to enter a Dim level from 0-255. Actually, we do not want them using numbers 0 or 1. The ActualDimLevel corrects for this.
                                                                // In practice, it is unlikely a user would want a dim level of 1 anyway, as it would be probably invisible. 
        int LightPin[NumLights] = {9,10,11,6,5,3,15,16};        // These are the Arduino pins to the 8 lights in order from left to right looking down on the top surface of the board. 
                                                                // Note that the six Arduino analog pins can be referred to by numbers 14-19
        int Dimmable[NumLights] = {1,1,1,1,1,1,0,0};            // This indicates which of these pins are capable of ouputting PWM, in order. PWM-capable pins on the Arduino are 3, 5, 6, 9, 10, 11
                                                                // Dimmable must be true in order for the light to be capable of DIM, FADEOFF, or XENON settings
        int LightSettings[NumLights][NumStates];                // An array to hold the settings for each state for each light. 
        int PriorLightSetting[NumLights][NumStates];            // Sometimes we want to temporarily change the setting for a light. We can store the prior setting here, and revert back to it when the temporary change is over.
        int PWM_Step[NumLights] = {0,0,0,0,0,0,0,0};            // What is the current PWM value of each light. 

        // FadeOff effect
        int FadeOff_EffectDone[NumLights] = {0,0,0,0,0,0,0,0};  // For each light, if = 1, then the Fade  effect is done, don't do it again until cleared (Fade_EffectDone = 0)

        // Xenon effect
        int Xenon_EffectDone[NumLights] = {0,0,0,0,0,0,0,0};    // For each light, if = 1, then the Xenon effect is done, don't do it again until cleared (Xenon_EffectDone = 0)
        int Xenon_Step[NumLights]       = {0,0,0,0,0,0,0,0};    // Save the current step variable for the Xenon light effect
        unsigned long Xenon_millis[NumLights] = {0,0,0,0,0,0,0,0};
        unsigned long Xenon_interval    = 25;                   // The interval between the various step of the Xenon effect

        // Backfire effect
        unsigned long backfire_interval;                        // Will save the random interval for the backfire effect
        unsigned long backfire_timeout;                         // Will save the random timeout interval to turn off the LED
        unsigned long Backfire_millis  =  0;                    // will store last time LED was updated
        boolean canBackfire            = false;                 // Is the backfiring effect currently active?

        // Overtaking effect
        boolean Overtaking             = false;
        
        // Blinking effect
        boolean Blinker                =  true;                 // A flip/flop variable used for blinking
        boolean FastBlinker            =  true;                 // A flip/flop variable used for fast blinking
        boolean IndividualLightBlinker[NumLights] = {true, true, true, true, true, true, true, true};   // A flip/flop variable but this time one for each light. Used for SoftBlink.


    // RC CHANNEL INPUTS
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        const byte ThrottleChannel_Pin =     2;                 // The Arduino pin connected to the throttle channel input
        const byte SteeringChannel_Pin =    17;                 // The Arduino pin connected to the steering channel input (this is the same as saying pin A3)
        const byte Channel3_Pin        =     4;                 // The Arduino pin connected to the Channel 3 input
        boolean Failsafe               = false;                 // If we loose contact with the Rx this flag becomes True
        unsigned long ServoTimeout     = 30000;                 // Value in microseconds (uS) - length of time to wait for a servo pulse. Measured on Eurgle/HK 3channel at ~20-22ms between pulses
                                                                // Up to version 2.03 of OSL code this value was 21,000 (21ms) and it worked fine. However with the release of Arduino IDE 1.6.5, 
                                                                // something has changed about the pulseIn function, or perhaps the way it is compiled. At 21ms, pulseIn would return 0 every other read.
                                                                // Increasing the timeout to 30ms seems to have fixed it. LM - 7/15/2015
        int PulseMin_Bad               =   700;                 // Pulse widths lower than this amount are considered bad 
        int PulseMax_Bad               =  2300;                 // Pulse widths greater than this amount are considered bad
 
    // BOARD OBJECTS
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        const byte GreenLED            =    18;                 // The Arduino pin connected to the on-board Green LED (this is the same as saying pin A4)
        const byte RedLED              =    19;                 // The Arduino pin connected to the on-board Red LED (this is the same as saying pin A5) 
        const byte SetupButton         =    14;                 // The Arduino pin connected to the on-board push button (this is the same as saying pin A0) 
        // Button Object
        OSL_Button InputButton = OSL_Button(SetupButton, true, true, 25);   // Initialize a button object. Set pin, internal pullup = true, inverted = true, debounce time = 25 mS

    // CHANGE-SCHEME-MODE MENU VARIABLES
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean canChangeScheme     = false;                    // Are we allowed to enter Change Scheme Mode? (This is set to true after being in the STOP state for several seconds)
        unsigned int BlinkOffID;                                // SimpleTimer ID number for the blinking lights
        static boolean Blinking;                                // Are the lights blinking? 
        static boolean State;                                   // If blinking, are they blinking on (true) or off (false)? 
        static boolean PriorState;                              // Blinking state in the prior iteration
        static int TimesBlinked;                                // How many times have the lights blinked
        static boolean ChangeSchemeMode = false;                    // A flag to indicate if we are in Change-Scheme-Mode or not

    // EEPROM
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        const long EEPROM_Init         = 0xAA43;                // Change this any time the EEPROM content changes
        struct __eeprom_data {                                  // __eeprom_data is the structure that maps all of the data we are storing in EEPROM
          long E_InitNum;                                       // Number that indicates if EEPROM values have ever been initialized 
          int E_ThrottlePulseMin;
          int E_ThrottlePulseMax;
          int E_ThrottlePulseCenter;
          int E_TurnPulseMin;
          int E_TurnPulseMax;
          int E_TurnPulseCenter;
          int E_Channel3PulseMin;
          int E_Channel3PulseMax;
          int E_Channel3PulseCenter;
          boolean E_ThrottleChannelReverse;
          boolean E_TurnChannelReverse;
          boolean E_Channel3Reverse;
          int E_CurrentScheme;
        };




// ====================================================================================================================================================>
//  SETUP
// ====================================================================================================================================================>
void setup()
{
    // SERIAL
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        Serial.begin(BaudRate);  

    // PINS
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        pinMode(RedLED, OUTPUT);                                // Set RedLED pin to output
        pinMode(GreenLED, OUTPUT);                              // Set GreenLED pin to output
        RedLedOff();                                            // Turn off board LEDs to begin with
        GreenLedOff();

        for (int i=0; i<NumLights; i++)                             
        {
            pinMode(LightPin[i], OUTPUT);                       // Set all the external light pins to outputs
            TurnOffLight(i);                                    // Start with all lights off                        
        }
    
        pinMode(ThrottleChannel_Pin, INPUT_PULLUP);             // Set these pins to input, with internal pullup resistors enabled
        pinMode(SteeringChannel_Pin, INPUT_PULLUP);
        pinMode(Channel3_Pin, INPUT_PULLUP);
        pinMode(SetupButton, INPUT_PULLUP);    

    // CONNECT TO RECEIVER
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        Failsafe = true;                                        // Set failsafe to true
        GetRxCommands();                                        // If a throttle signal is measured, Failsafe will turn off
        SteeringChannelPresent = CheckSteeringChannel();        // Check for the presence of a steering channel. If we don't find it here, we won't be checking for it again until the board is rebooted
        Channel3Present = CheckChannel3();                      // Check for the presence of Channel 3. If we don't find it here, we won't be checking for it again until the board is rebooted

            
    // LOAD VALUES FROM EEPROM    
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        long Temp;
        eeprom_read(Temp, E_InitNum);                           // Get our EEPROM Initialization value
        if(Temp != EEPROM_Init)                                 // If EEPROM has never been initialized before, do so now
        {    Initialize_EEPROM();  }
        else
        {    Load_EEPROM();        }                            // Otherwise, load the values from EEPROM

    // RUN LIGHT SETUP
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        SetupLights(CurrentScheme);                             // Set the lights to the Scheme last saved in EEPROM
        FixDimLevel();                                          // Takes care of a bug that only occurs if a user sets the Dim level to 1 (unlikely)

    // INITIATE BACKFIRE settings
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        // Activate the random Seed and set inital random values. These will be set to new random values each time a backfire event occurs, but 
        // we need to initialize them for the first event. 
        randomSeed(analogRead(0));
        backfire_interval = random(BF_Short, BF_Long);
        backfire_timeout  = BF_Time + random(BF_Short, BF_Long);

}



// ====================================================================================================================================================>
//  MAIN LOOP
// ====================================================================================================================================================>

void loop()
{
    // LOCAL VARIABLES
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
    // Drive Modes
    static int DriveModeCommand = UNKNOWN;                      // The Drive Mode presently being commanded by user. At startup we don't know, so initialize to UNKNOWN.
    static int DriveMode = STOP;                                // As opposed to DriveModeCommand, this is the actual DriveMode being implemented. 
    static int DriveMode_Previous = STOP;                       // The previous Drive Mode implemented. There is no "previous" when first started, initialize to STOP
    static int DriveModeCommand_Previous = STOP;                // There is no "previous" command when we first start. Initialize to STOP
    static int DriveMode_LastDirection = STOP;
    static boolean DriveModeTransition = false;                 // Are we in the midst of a drive mode transition event? 
    static int TransitionMode;                                  // Which transition are we doing? 
    static int ReverseTaps = 0;
    
    static int ThrottleCommand_Previous;
    
    // Shifting Direction
    static int DriveFlag = NO;                                  // We start with movement allowed
    static unsigned long TransitionStart;                       // A marker which records the time when the shift transition begins
    static unsigned long StopLength;                            // Counter to determine how long we have been commanding Stop

    // Turn signals
    static unsigned long TimeStopped;                           // When did we stop - will be used to initiate a delay after stopping before turn signals can be used

    // Scheme change variables
    static int MaxTurn;                                         // This will end up being 90 percent of Turn Command, we consider this a full turn
    static int MinTurn;                                         // This will end up being 10-20 percent of Turn Command, this is the minimum movement to be considered a turn command during Change-Scheme-Mode
    static byte RightCount = 0;                                 // We use these to count up the number of times the user has cranked the
    static byte LeftCount = 0;                                  // steering wheel completely over. 
    static int ChangeModeTime_mS = 2000;                        // Amount of time user has to enter Change-Scheme-Mode 
    static boolean ChangeSchemeTimerFlag = false;               // Has the timer begun
    static unsigned int TurnTimerID;                            // An ID for the timer that counts the turns of the wheel                      
    static int ExitSchemeFlag = 0;                              // A flag to indicate whether or not to exit Change-Scheme-Mode
    static int TimeToWait_mS = 1200;                            // Time to wait between change-scheme commands (otherwise as long as you held the wheel over it would keep cycling through rapidly)
    static int TimeToExit_mS = 3000;                            // How long to hold the wheel over until Change-Scheme-Mode is exited
    static unsigned long ExitStart;                             // Start time of the exit waiting period
    static boolean TimeoutFlag;
    static boolean HoldFlag; 
    static unsigned long HoldStart;

    // Backfire
    static unsigned int BackfireTimerID = 0; 
    // Overtaking
    static unsigned int OvertakeTimerID = 0;

    // Temp vars
    static unsigned long currentMillis;        
    static boolean WhatState = true;
    int i;    
    


    // STARTUP - RUN ONCE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
    if (Startup)
    {       
        if (DEBUG) { DumpDebug(); }                             // This puts some useful information to the Serial Port
        
        timer.setInterval(BlinkInterval, BlinkLights);          // This will call the function BlinkLights every BlinkInterval milliseconds
        timer.setInterval(FastBlinkInterval, FastBlinkLights);  // This will call the function FastBlinkLights every FastBlinkInterval milliseconds
        
        currentMillis = millis();                               // Initializing some variables 
        TransitionStart = currentMillis;  
        StopLength = currentMillis;  
        TimeStopped = currentMillis;
        TurnSignal_Enable = false;

        canChangeScheme = false;    
        ChangeSchemeTimerFlag = false;
        ChangeSchemeMode = false;
        MaxTurn = (int)((float)MaxRightTurn * 0.9);             // This sets a turn level that is near max, we use sequential back-and-forth max turns to enter Change-Scheme-Mode
        MinTurn = (int)((float)MaxRightTurn * 0.2);             // This is the minimum amount of steering wheel movement considered to be a command during Change-Scheme-Mode
        RightCount = 0;
        LeftCount = 0;

        Startup = false;                                        // This ensures the startup stuff only runs once

   }


// ETERNAL LOOP
// ------------------------------------------------------------------------------------------------------------------------------------------------>    

    // UPDATE TIMER/BUTTON STATE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        timer.run();
        InputButton.read();
        
        
    // USER WANTS TO RUN SETUPS
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        if (InputButton.pressedFor(2000))    
        {
            // User has held down the input button for two seconds. We are going to enter the radio setup routine. 
            RadioSetup(); 
        }

    // GET COMMANDS FROM RECEIVER
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        GetRxCommands();

    
    // DETECT IF THE USER WANTS TO ENTER CHANGE-SCHEME-MODE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        // To enter scheme-change-mode, the user needs to turn the steering wheel all the way back and forth at least six times (three each way) in quick succession (within ChangeModeTime_mS milliseconds)
        if (SteeringChannelPresent && canChangeScheme && (abs(TurnCommand) >= MaxTurn) && !ChangeSchemeMode)
        {    // Here we save how many times they have turned the wheel in each direction. 
            if ((TurnCommand > 0) && (WhatState == false))      
            {   RightCount += 1; 
                WhatState = true;
            }
            else if ((TurnCommand < 0) && (WhatState == true))  
            {   LeftCount += 1;  
                WhatState = false;
            }
            
            if (!ChangeSchemeTimerFlag)
            {   ChangeSchemeTimerFlag = true; 
                TurnTimerID = StartWaiting_mS(ChangeModeTime_mS);
            }
            
            if (!TimeUp)
            {   if (((RightCount + LeftCount) > 5) && (RightCount > 2) && (LeftCount > 2))
                {   ChangeSchemeTimerFlag = false;
                    RightCount = 0;
                    LeftCount = 0;
                    timer.deleteTimer(TurnTimerID);
                    ChangeSchemeMode = true;                    // The user has turned the steering wheel back and forth at least five times - enter Change-Scheme-Mode                    
                }
            }
        }
    
        if (ChangeSchemeTimerFlag == true && TimeUp)            // Time is up, don't enter Change-Scheme-Mode
        {   ChangeSchemeTimerFlag = false;
            RightCount = 0;
            LeftCount = 0;
            ChangeSchemeMode = false;
        }
            

    // CHANGE-SCHEME-MODE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        // When Change-Scheme-Mode is first entered, all the lights will blink rapidly for a few seconds. Afterwards, they will blink a sequence of numbers
        // that indicates which Scheme is currently selected. 
        // To select a different Scheme, turn the steering wheel briefly to the right (for the next Scheme) or briefly to the left (for the previous Scheme). 
        // When you get to the end of the Schemes it will just roll over. 
        // Each Scheme number is indicated by the same number of blinks on all lights. 
        // When you arrive at the desired Scheme number, hold the steering wheel all the way over to one side (doesn't matter which) and hold it there for 
        // three seconds. All lights will blink rapidly once again to indicate exit from Change-Scheme-Mode. The selected Scheme will be saved and the 
        // light controller will revert to it even after power is applied. 
        // If no control movements are made for 20 seconds, the program will automatically exit Change-Scheme-Mode
        if (ChangeSchemeMode)
        {
            TwinkleLights(2);
            TimesBlinked = 0;
            Blinking = true;
            State = true;
            PriorState = false;
            ExitSchemeFlag = 0;
            TimeoutFlag = false;
            HoldFlag = false;
            do
            {
                // Read the receiver
                GetRxCommands();

                timer.run();
               
                if (abs(TurnCommand) >= MaxTurn)                                
                {   // They are holding the stick completely over - start waiting to see if they want to
                    // exit Change-Scheme-Mode
                    if (ExitSchemeFlag == 0)    // If the wait hasn't already begun
                    {   if (TurnCommand > 0)       {ExitSchemeFlag =  1;}    // Here we remember which direction they are holding the steering wheel
                        else if (TurnCommand < 0 ) {ExitSchemeFlag = -1;}
                        ExitStart = millis();    // Start the wait
                    }
                    else
                    {   // The wait has already begun - check to see if they are still holding full over
                        if ((ExitSchemeFlag > 0) && (TurnCommand < MaxTurn))  { ExitSchemeFlag = 0; }    // They were holding full right but now they're somewhere less
                        if ((ExitSchemeFlag < 0) && (TurnCommand > -MaxTurn)) { ExitSchemeFlag = 0; }    // They were holding full left but now theyr'e somewhere less
                    }
                    TimeoutFlag = false;    // Regardless, we detected movement, so we reset the timeout flag
                }
                else if (abs(TurnCommand) >= MinTurn)
                {   if (!HoldFlag)
                    {   HoldFlag = true;
                        HoldStart = millis();
                    }
                    ExitSchemeFlag = 0;     // If we make it to here they are no longer trying to exit, so reset
                    if ((millis() - HoldStart) >= 500)
                    {   HoldFlag = false;
                        if ((millis() - TransitionStart) >= TimeToWait_mS)    // Only change scheme if we've waited long enough from last command
                        {   if (TurnCommand > 0) 
                            {   CurrentScheme += 1;
                                if (CurrentScheme > NumSchemes) { CurrentScheme = 1; }
                            }
                            else if (TurnCommand < 0)
                            {   CurrentScheme -= 1;
                                if (CurrentScheme < 1) { CurrentScheme = NumSchemes; }
                            }
                            TransitionStart = millis();                       // Force them to wait a bit before changing the scheme again
                        }
                    }
                    TimeoutFlag = false;    // Regardless, we detected movement, so we reset the timeout flag
                }
                else
                {   // In this case, they are not moving the steering wheel at all. We start the timeout timer
                    if (!TimeoutFlag)
                    {   TimeoutFlag = true;
                        ExitStart = millis();
                    }
                    ExitSchemeFlag = 0; // But we also reset this flag, because they are obviously not holding the wheel all the way over either
                    HoldFlag = false;
                }

                timer.run();
                BlinkAllLights(CurrentScheme);                
                
                if ((ExitSchemeFlag != 0) && ((millis() - ExitStart) >= TimeToExit_mS))
                {   // User is exiting out of Change-Scheme-Mode
                    ChangeSchemeMode = false;
                }
                else if (TimeoutFlag &&  ((millis() - ExitStart) >= 20000))
                {   // No movement of steering wheel for 20 seconds - automatic exit
                    ChangeSchemeMode = false;
                }
            }
            while (ChangeSchemeMode);
            TwinkleLights(2);
            SetupLights(CurrentScheme);  // Change the scheme
            SaveScheme_To_EEPROM();      // Save the selection to EEPROM
        }

    
    // CALCULATE APPARENT DRIVE MODE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        // Figure out what Drive Mode the receiver is indicating
        DriveModeCommand = ReturnDriveMode(ThrottleCommand); 


    // CALCULATE ACTUAL DRIVE MODE THAT THE CAR IS PRESENTLY IN
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        // Drive Mode Command can be one of four things - Forward, Reverse, Stop or Brake. Each one may have some conditions
        // to meet before we let our actual DriveMode = the commanded mode
        
        // COMMAND STOP
        // -------------------------------------------------------------------------------------------------------------------------------------------->   
        // Are we attempting to stop? If so, we only consider ourselves truly stopped if we have been commanding it for a 
        // specified length of time. The reason being, the car can take some time to coast to a stop - we may still want to apply brake during
        // the coasting period, and if we set our mode to Stop right away, opposite throttle would not be counted as a brake, but as a change in 
        // direction. 
        if (DriveModeCommand == STOP)
        {
           if (DriveModeCommand_Previous == STOP)
           {   // Check to see if we have been commanding stop long enough to change our DriveMode
               // This length of time will be different depending on whether we are stopping from forward (longer) or from reverse (quite short)
               if (DriveMode == FWD)
               {
                   if ((millis() - StopLength) >= TimeToStop_FWD_mS)
                   {
                       DriveMode = STOP;    // Throttle has been in the Stop position for TimeToStop_FWD_mS: we assume we are now really stopped. 
                   }
               }
               else if (DriveMode == REV)
               {
                   if ((millis() - StopLength) >= TimeToStop_REV_mS)
                   {
                       DriveMode = STOP;    // Throttle has been in the Stop position for TimeToStop_REV_mS: we assume we are now really stopped. 
                   }
               }
           }
           else
           {   //Previously we were commanding something else, so start the stop time counter
               StopLength = millis();
           }
        }
        else
        {
            // If the user is not commanding Stop, reset the TurnSignal_Enable flag. 
            TurnSignal_Enable = false;
            // Also reset the canChangeScheme flag, we don't want to enter ChangeScheme mode while moving forward
            canChangeScheme = false;
        }        

        // Turn signal timer - start the timer when the car first comes to a stop. 
        if ((DriveMode == STOP) && (DriveMode_Previous != STOP))
        {   // The car has only just now come to a stop. We begin a timer. Not until this timer runs out will a turn command (while still stopped)
            // actually engage the turn signal lights. This is only if the user has specified BlinkTurnOnlyAtStop = true. 
            TurnSignal_Enable = false;
            TimeStopped = millis();
            // We use this same timer to delay the option of entering ChangeScheme mode
            canChangeScheme = false;
        }
        else if ((DriveMode == STOP) && (DriveMode_Previous == STOP))
        {   // We have been stopped already. Check to see if the turn signal signal timer is up - if so, we will permit a turn command (while still stopped)
            // to engage the turn signal lighs. This is only if the user has specified BlinkTurnOnlyAtStop = true. 
            if ((millis() - TimeStopped) >= TurnSignalDelay_mS)
            {
                TurnSignal_Enable = true;
                // After this much time has passed being stopped, we also allow the user to enter ChangeScheme mode if they want
                canChangeScheme = true;
            }        
            
            // If we are stopped and have been stopped, we are also no longer decelerating, so reset this flag.
            Decelerating = false;
            Accelerating = false;
            canBackfire = false;
        }


        // DECELERATING
        // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Here we are trying to identify sharp deceleration commands (user quickly letting off the throttle). 
        // The "if" statement says: 
        // - if we are not already decelerating, and
        // - if we are presently moving forward, and
        // - if the user is not commanding a reverse throttle, and
        // - if the current throttle command is less than the prior command minus 20 (means, we have let off at least DecelPct steps of throttle since last time - there are 100 steps possible)
        // Then if all this is true, we set the Decelerating flag
        if ((DriveMode == FWD) && (ThrottleCommand >= 0) && (ThrottleCommand < ThrottleCommand_Previous - DecelPct))        
        {   // We are decelerating
            Decelerating = true;
        }
        else
        {   // We are not decelerating, clear
            Decelerating = false;
        }

        // BACKFIRE Enable
        // -------------------------------------------------------------------------------------------------------------------------------------------->
        if (Decelerating && !canBackfire)
        {   // The last backfire is over, and we have started decelerating again. Enable the backfire effect. 
            canBackfire = true;
            // Each backfire event lasts a random length of time. 
            BackfireTimerID = timer.setTimeout(backfire_timeout, BackfireOff);
        } 
        else if (canBackfire && !timer.isEnabled(BackfireTimerID)) 
        {   // disable Backfire effect if the timer has run out  
            canBackfire = false;
        }


        // ACCELERATING
        // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Here we are trying to identify sharp acceleration commands. 
        // The "if" statement says: 
        // - if we are not already accelerating, and
        // - if we are presently moving forward, and
        // - if the user is not commanding a reverse throttle, and
        // - if the current throttle command is greater than the prior command plus AccelPct (means, we have increased AccelPct throttle since last time)
        // Then if all this is true, we set the Accelerating flag
        if ((DriveMode == FWD) && (ThrottleCommand >= 0) && (ThrottleCommand > ThrottleCommand_Previous + AccelPct))        
        {   // We are accelerating
            Accelerating = true;
        }
        else
        {   // We are not accelerating, clear
            Accelerating = false;
        }

        // OVERTAKE Enable
        // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Overtaking is simply a timed event that occurs when a heavy acceleration is detected. The length of time the Overtake event lasts is set in 
        // UserConfig. During that time, any settings specified under the Acceleration column will take effect. When the timer is up, the lights will
        // revert back to whatever they were previously. 
        // Use this with a FASTBLINK setting under the Accelerating column for your headlights, to simulate the overtaking flash as seen in 24hr Le Mans races
        if (Accelerating && !Overtaking)
        {   // The last overtaking is over, and we have started accelerating again. Enable the Overtaking timer again.
            Overtaking = true;
            // This will set a timer of OvertakeTime length long, and when the timer expires, it will call the OvertakeOff function
            OvertakeTimerID = timer.setTimeout(OvertakeTime, OvertakeOff);
        } 
        else if (Overtaking && !timer.isEnabled(OvertakeTimerID)) 
        {   // disable Backfire effect if the timer has run out  
            Overtaking = false;
        }

        // COMMAND BRAKE
        // -------------------------------------------------------------------------------------------------------------------------------------------->           
        // If we are braking, turn on the brake light
        Braking = ReturnBrakeFlag(DriveMode_Previous, DriveModeCommand);
        if (Braking) DriveModeCommand = STOP;   //Braking also counts as a stop command since that is what we will eventually be doing
        
        // COMMAND FORWARD
        // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Are we attempting to transition to forward, and if so, is it presently allowed? 
        if (DriveFlag == NO && DriveModeCommand == FWD)
        {   // In this case we know we were previously at a stop, and we are now commanding forward
            // We allow the transition if:
            // A) We were moving forward before the stop (commanding same direction as previous), or
            // B) The necessary amount of time has passed for a change in direction
            if ((DriveMode_LastDirection == DriveModeCommand) || ((millis() - TransitionStart) >= TimeToShift_mS))
            {
                DriveMode = FWD;
                DriveFlag = YES;
                ReverseTaps = 0;        // Reset the reverse tap count

                // Should we leave the turn signal on even though we're now moving forward? 
                // TurnCommand != 0                 The wheels must be turned   
                // TurnFromStartContinue_mS > 0     User must have enabled this setting
                // BlinkTurnOnlyAtStop = true       This must be true, otherwise we just allow blinking all the time, so none of this is necessary
                // TurnSignal_Enable = true         This means the car has already been stopped for some length of time, and is not just coasting (set by TurnSignalDelay_mS)
                if (TurnCommand != 0 && TurnFromStartContinue_mS > 0 && BlinkTurnOnlyAtStop && TurnSignal_Enable) 
                {
                    // In this case we have just begun starting to move, and our wheels are turned at the same time. 
                    // We will keep the turn signals on for a brief period after starting, as set by TurnFromStartContinue_mS
                    TurnSignalOverride = TurnCommand;                                 // TurnSignalOverride saves the turn direction, and will act as a fake turn command in the SetLights function
                    timer.setTimeout(TurnFromStartContinue_mS, ClearBlinkerOverride); // ClearBlinkerOverride will set TurnSignalOverride back to 0 when the timer is up. 
                }
            }
        }
    
        // COMMAND REVERSE
        // -------------------------------------------------------------------------------------------------------------------------------------------->           
        // Are we attempting to transition to reverse, and if so, is it presently allowed? 
        if (DriveFlag == NO && DriveModeCommand == REV)
        {   // In this case we know we were previously at a stop, and we are now commanding reverse
            // We allow the transition if:
            if (DriveMode_LastDirection == DriveModeCommand)
            {   // A) We were moving in reverse before the stop (commanding same direction as previous)
                DriveMode = REV;
                DriveFlag = YES;
            }
            else if ((millis() - TransitionStart) >= TimeToShift_mS)
            {   // or, we allow it if
                // B) The necessary amount of time has passed for a change in direction AND, 
                if ((DoubleTapReverse == true && ReverseTaps > 1) || (DoubleTapReverse == false))
                {
                    // C) If DoubleTapReverse = true, we also check to make sure the number of reverse taps is > 1
                    //    otherwise the time constraint is all we need
                    DriveMode = REV;
                    DriveFlag = YES;
                }
            }

            // Should we leave the turn signal on even though we're now moving backwards? 
            // DriveFlag = YES                  One of the two checks above returned true (A, or B & C) - meaning, we are moving in reverse
            // TurnCommand != 0                 The wheels must be turned   
            // TurnFromStartContinue_mS > 0     User must have enabled this setting
            // BlinkTurnOnlyAtStop = true       This must be true, otherwise we just allow blinking all the time, so none of this is necessary
            // TurnSignal_Enable = true         This means the car has already been stopped for some length of time, and is not just coasting (set by TurnSignalDelay_mS)
            if (DriveFlag == YES && TurnCommand != 0 && TurnFromStartContinue_mS > 0 && BlinkTurnOnlyAtStop && TurnSignal_Enable) 
            {
                // In this case we have just begun starting to move, and our wheels are turned at the same time. 
                // We will keep the turn signals on for a brief period after starting, as set by TurnFromStartContinue_mS
                TurnSignalOverride = TurnCommand;                                 // TurnSignalOverride saves the turn direction, and will act as a fake turn command in the SetLights function
                timer.setTimeout(TurnFromStartContinue_mS, ClearBlinkerOverride); // ClearBlinkerOverride will set TurnSignalOverride back to 0 when the timer is up. 
            }
        }

    // WE NOW HAVE OUR ACTUAL DRIVE MODE - SET THE LIGHTS ACCORDINGLY
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        SetLights(DriveMode);        // SetLights will take into account whatever position Channel 3 is in, as well as the present drive mode


    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
    // GET READY FOR NEXT LOOP 
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    

    // SHIFT TRANSITION TIMER 
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // If we have just arrived at a stop, we start the transition timer. This will expire in TimeToShift_mS milliseconds. It is the length of time 
        // we must wait before the car is allowed to change direction (either from forwad to reverse or from reverse to forward)
        // This transition time can be set in UserConfig.h. It can even be zero. Some ESCs do require a time however and that is what we are trying to emulate here. 

        // Start transition timer when we first reach a stop from a moving direction:
        if (DriveMode_Previous != STOP && DriveModeCommand == STOP && DriveFlag == YES)
        {   //Start Timer
            TransitionStart = millis();
            // Save last direction
            DriveMode_LastDirection = DriveMode_Previous;
            // While transition is underway, DriveFlag = No
            DriveFlag = NO;
        }
    
    // COUNT REVERSE TAPS
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // Most ESCs require the user to tap reverse twice before the car will actually go into reverse. This is to prevent the user from going straight
        // into reverse from full speed ahead. However, some ESCs allow the user to disable this feature (eg, crawlers), so this setting can also be turned 
        // off in UserConfig.h

        // If reverse is commanded, count how many times the stick is moved to reverse - we are only truly moving in reverse on the second tap
        // But we do ignore reverse stick when it is in fact a brake command.
        if (DriveModeCommand == REV && DriveModeCommand_Previous == STOP )
        {
            ReverseTaps +=1;
            if (ReverseTaps > 2)
            {
                ReverseTaps = 2;
            }
        }


    // DEBUGING
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        if ((DEBUG == true) && (DriveModeCommand_Previous != DriveModeCommand))
        {
            Serial.print(F("Drive Command: "));
            Serial.println(printMode(DriveModeCommand));
        }

        if ((DEBUG == true) && (DriveMode_Previous != DriveMode))
        {
            Serial.print(F("Actual Drive Mode: "));
            Serial.println(printMode(DriveMode));
        }
    

    //  SAVE COMMANDS FOR NEXT ITERATION
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // Set previous variables to current
        DriveMode_Previous = DriveMode;
        DriveModeCommand_Previous = DriveModeCommand;
        ThrottleCommand_Previous = ThrottleCommand;


} // End of Loop






void DumpDebug()
{
    // Channel pulse values 
    Serial.println(F("PULSE:  Min - Ctr - Max"));
    Serial.print(F("Throttle "));
    Serial.print(ThrottlePulseMin);
    PrintSpaceDash();
    Serial.print(ThrottlePulseCenter);
    PrintSpaceDash();
    Serial.println(ThrottlePulseMax);

    Serial.print(F("Turn "));
    Serial.print(TurnPulseMin);
    PrintSpaceDash();
    Serial.print(TurnPulseCenter);
    PrintSpaceDash();
    Serial.println(TurnPulseMax);

    Serial.print(F("Ch3 "));
    Serial.print(Channel3PulseMin);
    PrintSpaceDash();
    Serial.print(Channel3PulseCenter);
    PrintSpaceDash();
    Serial.println(Channel3PulseMax);


    // Channel Reversing
    Serial.print(F(" - Throttle Channel Reverse: "));  
    PrintTrueFalse(ThrottleChannelReverse);
    Serial.print(F(" - Turn Channel Reverse: ")); 
    PrintTrueFalse(TurnChannelReverse);
    Serial.print(F(" - Channel 3 Reverse: "));  
    PrintTrueFalse(Channel3Reverse);


    // Channels disconnected? 
    Serial.print(F("Steering Channel: "));
    if (!SteeringChannelPresent == true) { Serial.print(F("NOT ")); }
    Serial.println(F("CONNECTED")); 

    Serial.print(F("Channel 3: "));
    if (!Channel3Present) { Serial.print(F("NOT ")); }
    Serial.println(F("CONNECTED")); 
}


