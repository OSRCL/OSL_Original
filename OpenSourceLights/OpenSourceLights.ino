 /*
 * Open Source Lights   An Arduino-based program to control LED lights on RC vehicles. 
 * Copyright 2011       Luke Middleton
 *
 * For more information, see the RCGroups thread: 
 * http://www.rcgroups.com/forums/showthread.php?t=1539753
 * 
 * GitHub Repository:
 * https://github.com/OSRCL/OSL_Original
 * 
 *
 * To compile select               Tools -> Board     -> Arduino Duemilanove or Diecimila or Nano
 * Also select correct processor   Tools -> Processor -> ATmega328P
 *                                 If you have problems writing to the device, try Tools -> Processor -> Atmega328P (Old Bootloader)
 *
*/

// ====================================================================================================================================================>
//  INCLUDES 
// ====================================================================================================================================================>
    #include "AA_UserConfig.h"
    #include "src/OSL_Settings/OSL_Settings.h"
    #include "src/elapsedMillis/elapsedMillis.h"
    #include "src/OSL_Button/OSL_Button.h"  // By JChristensen. See: https://github.com/JChristensen/Button Renamed from Button to OSL_Button so it won't conflict with other button libraries. 
    #include "src/OSL_LedHandler/OSL_LedHandler.h"
    #include "src/OSL_SimpleTimer/OSL_SimpleTimer.h"
    #include "src/OSL_PinChangeInterrupt/PinChangeInterrupt.h"

   
// ====================================================================================================================================================>
//  GLOBAL VARIABLES
// ====================================================================================================================================================>

    // Hardware version
    // -------------------------------------------------------------------------------------------------------------------------------------------------->                
        uint8_t HardwareVersion =             1;                // We will detect the actual hardware version later in Setup, this just initializes to something.

    // Startup
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean Startup                  = true;                // This lets us run a few things in the main loop only once instead of over and over

    // Objects
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        OSL_LedHandler                   RedLED;
        OSL_LedHandler                 GreenLED;
        OSL_LedHandler  LightOutput[NumLights];                // LED handler for each output (NUM_LIGHT_OUTPUTS is defined in OSL_Settings.h)
        OSL_Button                  InputButton;                // Button object, will get set later in Setup() when we know what hardware version we are running on.

    // Simple Timer
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        OSL_SimpleTimer                   timer;                // Instantiate a SimpleTimer named "timer"
        boolean TimeUp                   = true;

    // EEPROM
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        const long EEPROM_Init         = 0xDF03;                // Change this any time the EEPROM content changes
        struct __eeprom_data {                                  // __eeprom_data is the structure that maps all of the data we are storing in EEPROM
          long E_InitNum;                                       // Number that indicates if EEPROM values have ever been initialized 
          int16_t E_ThrottlePulseMin;
          int16_t E_ThrottlePulseMax;
          int16_t E_ThrottlePulseCenter;
          int16_t E_TurnPulseMin;
          int16_t E_TurnPulseMax;
          int16_t E_TurnPulseCenter;
          int16_t E_Channel3PulseMin;
          int16_t E_Channel3PulseMax;
          int16_t E_Channel3PulseCenter;
          boolean E_ThrottleChannelReverse;
          boolean E_TurnChannelReverse;
          boolean E_Channel3Reverse;
          uint8_t E_CurrentScheme;
        };

    // RC Channel defines
    // -------------------------------------------------------------------------------------------------------------------------------------------------->            
        struct _rc_channel {
            uint8_t  pin;                                       // Pin number of channel
            uint8_t  channel;                                   // What channel is this (0 = throttle, 1 = steering, 2 = Channel 3)
            char     state;                                     // State of this individual channel (acquiring, synched, lost)
            uint16_t rawPulseWidth;                             // Unchecked pulse-width, may or may not be valid
            int16_t  pulse;                                     // Actual pulse-width in uS, typically in the range of 1000-2000 (sanity checked). Although it should always be positive, this needs to be signed for smoothing to work.
            int16_t  pulseMin;                                  // Minimum pulse width of incoming channel, as saved during Radio Setup
            int16_t  pulseCenter;                               // Center pulse width of incoming channel, as saved during Radio Setup
            int16_t  pulseMax;                                  // Maximum pulse width of incoming channel, as saved during Radio Setup
            uint8_t  deadband;                                  // Deadband around center where changes are ignored
            boolean  readyForUpdate;                            // Set to true if a signal has been read (saved to rawPulseWidth) and we are ready for the main loop to process
            boolean  updated;                                   // Has the value on this channel changed since the last check?
            boolean  reversed;                                  // Should this channel be reversed
            int8_t   mappedCommand;                             // For throttle and steering channels, the current command (adjusted for known center and endpoint settings, and mapped to a range of -100 to 100)
            uint8_t  numSwitchPos;                              // In the case of a digital channel, how many switch positions can it read. 
            uint8_t  switchPos;                                 // In the case of Channel 3, what switch "position" is the channel presently in
            boolean  smooth;                                    // Apply averaging to the incoming value
            int16_t  smoothedValue;                             // Remember the last value to use for smoothing (we smooth the pulse, not the mapped command/switch position, so values are always positive)
            uint32_t lastEdgeTime;                              // Timing variable for measuring pulse width
            uint32_t lastGoodPulseTime;                         // Time last signal was received for this channel
            uint8_t  acquireCount;                              // How many pulses have been acquired during acquire state
            boolean  Digital;                                   // Is this a digital channel (switch input) or an analog (variable) input?             
        }; 
        _rc_channel RC_Channel[NUM_RC_CHANNELS];

        boolean Failsafe                = false;                // If we loose contact with the Rx this flag becomes true
        char RC_State =  RC_SIGNAL_UNINITIALIZED;               // State of the entire radio system (as opposed to per-channel states above)
        char Last_RC_State = RC_SIGNAL_UNINITIALIZED;           // Last state. The uninitialized state is used only once, at startup, afterwards it is either acquiring, synched, or lost
        int RxSignalLostTimerID             = 0;                // Timer used to flash the lights if the radio signal is lost
        int8_t ThrottleCommand              = 0;                // Discrete variables are also useful, we will map these to the correct channel inputs in ProcessRCCommand()
        int8_t TurnCommand                  = 0;
        uint8_t Channel3Command             = 0;                        
        int8_t ThrottleCommand_Previous     = 0;                // We keep a second set so we can compare and determine when a command has changed
        uint8_t Channel3Command_Previous    = 0;
        int8_t TurnCommand_Previous         = 0;

    // Driving
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean Braking                 = false;                // Are we braking       
        boolean Braking_Previous        = false;                // Allows us to track changes     
        boolean Decelerating            = false;                // Are we sharply decelerating 
        boolean Accelerating             = true;                // Are we sharply accelerating
        boolean StoppedLongTime         = false;                // Have we been stopped for a long time (actual length of time set on the AA_UserConfig tab - LongStopTime_mS)
        int8_t Direction              = NO_TURN;                // As opposed to command which indicates how much we are turning, this just tells us if we are in a right turn, left turn, or no turn.
        int8_t Direction_Previous     = NO_TURN;
        boolean TurnSignal_Enable       =  true;                // If the user decides to restrict turn signals only to when the car is stopped, they can further add a delay that begins
                                                                // when the car first stops, and until this delay is complete, the turn signals won't come on. This flag indicates if the delay
                                                                // is up. Initialize to true, otherwise turn signals won't work until the car has driven forward once and stopped.
        int8_t TurnSignalOverride           = 0;                // The other situation we want to modify the turn signal is when starting from a stop, while turning. In this case we leave the turn signals on
                                                                // for a period of time even after the car has started moving (typically turn signals are disabled while moving). This mimics a real car where
                                                                // the turn signal isn't cancelled until after the steering wheel turns back the opposite way. In our case we don't check the steering wheel, 
                                                                // we just wait a short amount of time (user configurable in AA_UserConfig.h, variable TurnFromStartContinue_mS)
    // Light Settings
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        uint8_t LightSettings[NumLights][NumStates];           // An array to hold the settings for each state for each light. 
        uint8_t CurrentLightSetting[NumLights];                // What state is the light in presently
        boolean canBackfire             = false;                // Is the backfiring effect currently active?
        uint16_t backfire_timeout;                              // Will save the random timeout interval to turn off the LED
        int BackfireTimerID                 = 0;                // Backfire event timer ID
        boolean Overtaking              = false;                // Overtaking effect
        int OvertakeTimerID                 = 0;                // Overtaking event timer ID

    // Schemes & Change-Scheme-Mode
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        uint8_t CurrentScheme;                                  // Indicates which scheme is presently selected and active. Number from 1 to NumSchemes (set in AA_UserConfig.h, though it must also match
                                                                // the number of schemes literally defined in AA_LIGHT_SETUP) 
                                                                // Note that the actual schemes are zero-based (0 to NumSchemes-1) but don't worry about that, the code takes care of it. 
        boolean canChangeScheme         = false;                // Are we allowed to enter Change Scheme Mode? (This is set to true after being in the STOP state for several seconds)
        int CSM_BlinkOffTimerID             = 0;                // SimpleTimer ID number for the blinking lights
        boolean CSM_Blinking;                                   // Are the change-scheme-mode lights blinking? 
        boolean CSM_State;                                      // If blinking, are they blinking on (true) or off (false)? 
        boolean CSM_PriorState;                                 // Blinking state in the prior iteration
        uint8_t CSM_TimesBlinked;                               // How many times have the lights blinked
        boolean ChangeSchemeMode        = false;                // A flag to indicate if we are in Change-Scheme-Mode or not

    // Shelf-queen mode
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        boolean shelfQueenMode          = false;                // A flag to indicate if we are in shelf-queen mode or not



// ====================================================================================================================================================>
//  SETUP
// ====================================================================================================================================================>
void setup()
{
    // Serial
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        Serial.begin(BaudRate);  

    // Hardware Version
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        // Let's figure out which version of hardware we're running on
        pinMode(pin_VCHECK_A, INPUT_PULLUP);
        pinMode(pin_VCHECK_B, INPUT_PULLUP);
        delay(10);  // Wait a bit before reading
        if      ( digitalRead(pin_VCHECK_A) &&  digitalRead(pin_VCHECK_B)) HardwareVersion = 1; // A high, B high - original design board (where in fact both pins are connected to nothing, it is the internal pullup that makes them high)
        else if ( digitalRead(pin_VCHECK_A) && !digitalRead(pin_VCHECK_B)) HardwareVersion = 2; // A high, B low  - hardware version 2
        else if (!digitalRead(pin_VCHECK_A) &&  digitalRead(pin_VCHECK_B)) HardwareVersion = 1; // A low,  B high - hardware version x (unspecified, keep at version 1 for now)
        else if (!digitalRead(pin_VCHECK_A) && !digitalRead(pin_VCHECK_B)) HardwareVersion = 1; // A low,  B low  - hardware version x (unspecified, keep at version 1 for now) 

    // Load values from EEPROM  
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        long Temp;
        eeprom_read(Temp, E_InitNum);                           // Get our EEPROM Initialization value
        if(Temp != EEPROM_Init)                                 // If EEPROM has never been initialized before, do so now
        {    Initialize_EEPROM();  }
        else
        {    Load_EEPROM();        }                            // Otherwise, load the values from EEPROM. Note this is only for non-channel values, 
                                                                // channel-EEPROM values will get assigned below in InitializeRCChannels();
    // RC Inputs
    // -------------------------------------------------------------------------------------------------------------------------------------------------->        
        InitializeRCChannels();                                 // Initialize/clear RC channels
        EnableRCInterrupts();                                   // Start checking the RC pins for a signal

    // Initialize lights
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        SetLightScheme(CurrentScheme);                          // Set the lights to the Scheme last saved in EEPROM

    // Initialize backfire settings
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        // Activate the random Seed and set an inital random value for the deceleration effect. It will be updated to a new 
        // random value each time a deceleration event occurs, but we need to initialize it for the first event. 
        if (HardwareVersion == 2) randomSeed(analogRead(pin_HW2_SetupButton));    // When the button is not pressed, this input is left floating, meaning the value could be anything, which is what we want for randomSeed
        else                      randomSeed(analogRead(pin_HW1_SetupButton));
        backfire_timeout  = random(BF_Time_Short, BF_Time_Long);

    // Setup light objects
    // -------------------------------------------------------------------------------------------------------------------------------------------------->        
        if (HardwareVersion == 1)
        {
            RedLED.begin(pin_HW1_RedLED, false);                        
            GreenLED.begin(pin_HW1_GreenLED, false);
            LightOutput[0].begin(pin_HW1_Light1, false, true);          // First boolean is whether or not to invert the pin behavior - all these set to false
            LightOutput[1].begin(pin_HW1_Light2, false, true);          // Second boolean indicates if we are able to analog-write to the pin (is it PWM-able)
            LightOutput[2].begin(pin_HW1_Light3, false, true);
            LightOutput[3].begin(pin_HW1_Light4, false, true);
            LightOutput[4].begin(pin_HW1_Light5, false, true);
            LightOutput[5].begin(pin_HW1_Light6, false, true);
            LightOutput[6].begin(pin_HW1_Light7, false, false);         // Outputs 7 & 8 are not PWM-able
            LightOutput[7].begin(pin_HW1_Light8, false, false);
            InputButton.begin(pin_HW1_SetupButton, true, true, 25);     // Initialize a button object. Set pin, internal pullup = true, inverted = true, debounce time = 25 mS
        }
        else if (HardwareVersion == 2)
        {
            RedLED.begin(pin_HW2_RedLED, false);                        
            GreenLED.begin(pin_HW2_GreenLED, false);
            LightOutput[0].begin(pin_HW2_Light1, false, true);          // First boolean is whether or not to invert the pin behavior - all these set to false
            LightOutput[1].begin(pin_HW2_Light2, false, true);          // Second boolean indicates if we are able to analog-write to the pin (is it PWM-able)
            LightOutput[2].begin(pin_HW2_Light3, false, true);
            LightOutput[3].begin(pin_HW2_Light4, false, true);
            LightOutput[4].begin(pin_HW2_Light5, false, true);
            LightOutput[5].begin(pin_HW2_Light6, false, true);
            LightOutput[6].begin(pin_HW2_Light7, false, false);         // Outputs 7 & 8 are not PWM-able
            LightOutput[7].begin(pin_HW2_Light8, false, false);
            InputButton.begin(pin_HW2_SetupButton, true, true, 25);     // Initialize a button object. Set pin, internal pullup = true, inverted = true, debounce time = 25 mS
        }        
        // Start lights in the off state
        RedLED.off();
        GreenLED.off();
        for (uint8_t i=0; i<NumLights; i++)
        {   
            LightOutput[i].off();
            CurrentLightSetting[i] = LS_UNKNOWN;
        }
}



// ====================================================================================================================================================>
//  MAIN LOOP
// ====================================================================================================================================================>
void loop()
{
    // Main loop local variables
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
    // Drive Modes
    static uint8_t DriveModeCommand      = STOP;                // The Drive Mode presently being commanded by user. At startup we don't know, so initialize to UNKNOWN.
    static uint8_t DriveMode             = STOP;                // As opposed to DriveModeCommand, this is the actual DriveMode being implemented. 
    static uint8_t DriveMode_Previous    = STOP;                // The previous Drive Mode implemented. There is no "previous" when first started, initialize to STOP
    static uint8_t DriveModeCommand_Previous = STOP;            // There is no "previous" command when we first start. Initialize to STOP
    static uint8_t DriveMode_LastDirection = STOP;


    // Driving
    static boolean canBrake             = false;                // If BrakeAtThrottlePctBelow > 0 (in AA_UserConfig) we may set the brake flag even when moving in forward or reverse
    static uint32_t TimeStopped;                                // When did we stop - will be used to initiate a delay after stopping before turn signals can be used, or when stop-delay light settings can take effect.
    static uint32_t StopCMDLength;                              // Counter to determine how long we have been commanding Stop.
    static uint32_t Last_FWD_Time;                              // Keep track of how recently we were in forward. This will help us know whether to enter the Brake state or not. NOTE: The brake state
                                                                // will only ever apply if DoubeTapReverse = true. For crawler type ESCs that can go directly into reverse from forward, the only way to get
                                                                // the brake lights to turn on is to set them to the Stop state. 
    static uint8_t ReverseTaps              = 0;                // How many times have we gone into reverse - used for ESCs that require tapping reverse twice before the car will move in reverse


    // Scheme change variables
    #define CSM_StopDelay_mS               3000L                // How long after being stopped should we wait before we enable the user to enter change-scheme-mode
    #define CS_MAX_TURN                       90                // 90 percent of Turn Command, we consider this a full turn for purposes of the change-scheme selection
    #define CS_MIN_TURN                       20                // 20 percent of Turn Command, this is the minimum movement to be considered a turn command during Change-Scheme-Mode
    static byte RightCount                  = 0;                // We use these to count up the number of times the user has cranked the
    static byte LeftCount                   = 0;                // steering wheel completely over. 
    #define ChangeModeTime_mS              2000L                // Amount of time user has to enter Change-Scheme-Mode (this is the time within which they must turn the wheel back and forth rapidly)
    static boolean ChangeSchemeTimerFlag = false;               // Has the timer begun
    static int CSM_TurnTimerID;                                 // An ID for the timer that counts the turns of the wheel                      
    static int8_t ExitSchemeFlag            = 0;                // A flag to indicate whether or not to exit Change-Scheme-Mode
    #define CSM_ChangeTime_mS                400                // How long to hold the wheel over to increment/decrement to the next scheme
    #define CSM_TimeToWait_mS              1200L                // Time to wait between change-scheme commands (otherwise as long as you held the wheel over it would keep cycling through rapidly)
    #define CSM_TimeToExit_mS              3000L                // How long to hold the wheel over until Change-Scheme-Mode is exited
    static unsigned long CSM_ExitStart;                         // Start time of the exit waiting period
    #define CSM_InactiveExit_Time         20000L                // If there are no commands given for this length of time, automatically exit change-scheme-mode
    static boolean TimeoutFlag;
    static boolean HoldFlag; 
    static unsigned long HoldStart;
    static uint32_t TransitionStart;


    // Temp vars
    static unsigned long currentMillis;        
    static boolean WhatState = true;
   

    // STARTUP - RUN ONCE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
    if (Startup)
    {       
        RedLED.off();                                           // Start with these off
        GreenLED.off();

        if (enableShelfQueenMode)
        {
            uint8_t tempScheme; 
            shelfQueenMode = true;                              // Set this to true temporarily, we need to do this prior to actually entering shelf-queen mode to prevent the radio failsafe handler from messing things up
            delayWhilePolling(100);                             // Let's give ourselves plenty of time to detect something

            if (RC_State == RC_SIGNAL_UNINITIALIZED)
            {
                // Change the scheme to the shelf-queen scheme
                if (shelfQueenSchemeNumber > NumSchemes) tempScheme = 1; // If the user has entered an invalid scheme number, default to 1
                else                                     tempScheme = shelfQueenSchemeNumber;

                if (enableShelfQueenDelay)
                {
                    // We want to delay some random amount of time before turning the lights on
                    uint32_t sq_delay; 
                    sq_delay = random(sqd_Time_Short, sqd_Time_Long);
                    Serial.print(F("Shelf Queen Mode Delay: ")); Serial.print(((float)sq_delay/1000.0),1); Serial.println(F(" seconds"));
                    
                    currentMillis = millis();                   // Save the time
                    do {
                        PerLoopUpdates();
                    } while (((millis() - currentMillis) < sq_delay) && RC_State == RC_SIGNAL_UNINITIALIZED); 
                }

                if (RC_State != RC_SIGNAL_UNINITIALIZED)
                {
                    // The radio was detected while we were waiting, cancel shelf queen mode
                    shelfQueenMode = false; 
                }
    
                if (shelfQueenMode == true)
                {
                    // Ok, we're still good to enter shelf queen mode
                    if (DEBUG)
                    {
                        Serial.println();
                        Serial.println();
                        PrintLine(80);
                        Serial.print(F("SHELF QUEEN MODE - SCHEME ")); Serial.println(tempScheme);
                        PrintLine(80);
                    }
    
                    // Load the desired scheme
                    SetLightScheme(tempScheme);
    
                    // Set channel 3 to position 1 and set the lights
                    Channel3Command = ShelfQueenCh3Position; 
                    SetLights(STOP);
    
                    // Keep the onboard LEDs off
                    RedLED.off();
                    GreenLED.off();
    
                    // Keep polling the radio
                    do{
                        PerLoopUpdates();
                    } while (RC_State == RC_SIGNAL_UNINITIALIZED);
    
                    // If we make it to here, a radio signal has been detected.
                    shelfQueenMode = false;                        // Exit shelf-queen mode
                    SetLightScheme(CurrentScheme);                 // Go back to CurrentScheme
    
                    if (DEBUG)
                    {                
                        PrintLine(80);
                        Serial.print(F("EXIT SHELF QUEEN MODE - REVERT TO SCHEME ")); Serial.println(CurrentScheme);
                        PrintLine(80);
                        Serial.println();             
                        Serial.println(); 
                    }
    
                    // Now proceed as normal...
                }
            }
            else
            {
                shelfQueenMode = false;
            }
        }

        if (DEBUG) DumpSystemInfo();                            // This puts some useful information to the Serial Port if we have Debug enabled

        currentMillis = millis();                               // Initializing some variables 
        TransitionStart = currentMillis;  
        TimeStopped = currentMillis;
        TurnSignal_Enable = false;
        StoppedLongTime = false;
        
        canChangeScheme = false;    
        ChangeSchemeTimerFlag = false;
        ChangeSchemeMode = false;
        RightCount = 0;
        LeftCount = 0;
        Startup = false;                                        // This ensures the startup stuff only runs once
   }


// ETERNAL LOOP
// ------------------------------------------------------------------------------------------------------------------------------------------------>    

    // Per loop updates - things that need to be polled
        PerLoopUpdates();


    // Everything from here on, we only run if we are receiving valid radio commands
    if (Failsafe == false)
    {
        
    // USER WANTS TO RUN SETUPS
    // -------------------------------------------------------------------------------------------------------------------------------------------------->
        if (InputButton.pressedFor(2000))    
        {
            // User has held down the input button for two seconds. We are going to enter the radio setup routine. 
            RadioSetup(); 
        }

    
    // DETECT IF THE USER WANTS TO ENTER CHANGE-SCHEME-MODE
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        // To enter scheme-change-mode, the user needs to turn the steering wheel all the way back and forth at least six times (three each way) in quick succession (within ChangeModeTime_mS milliseconds)
        if (canChangeScheme && (abs(TurnCommand) >= CS_MAX_TURN) && !ChangeSchemeMode)
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
                CSM_TurnTimerID = StartWaiting_mS(ChangeModeTime_mS);
            }
            
            if (!TimeUp)
            {   if (((RightCount + LeftCount) > 5) && (RightCount > 2) && (LeftCount > 2))
                {   ChangeSchemeTimerFlag = false;
                    RightCount = 0;
                    LeftCount = 0;
                    timer.deleteTimer(CSM_TurnTimerID);
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
            Serial.println();
            PrintLine(80);
            Serial.println(F("ENTERING CHANGE-SCHEME MODE")); 
            PrintLine(80);
            Serial.print(F("Current Scheme: ")); Serial.println(CurrentScheme);
            TwinkleLights(2000);
            CSM_TimesBlinked = 0;
            CSM_Blinking = true;
            CSM_State = true;
            CSM_PriorState = false;
            ExitSchemeFlag = 0;
            TimeoutFlag = false;
            HoldFlag = false;
            do
            {
                // Polling updates
                PerLoopUpdates();

                if (abs(TurnCommand) >= CS_MAX_TURN)                                
                {   // They are holding the stick completely over - start waiting to see if they want to exit Change-Scheme-Mode
                    if (ExitSchemeFlag == 0)    // If the wait hasn't already begun
                    {   if (TurnCommand > 0)       {ExitSchemeFlag =  1;}    // Here we remember which direction they are holding the steering wheel
                        else if (TurnCommand < 0 ) {ExitSchemeFlag = -1;}
                        CSM_ExitStart = millis();    // Start the wait
                    }
                    else
                    {   // The wait has already begun - check to see if they are still holding full over
                        if ((ExitSchemeFlag > 0) && (TurnCommand < CS_MAX_TURN))  { ExitSchemeFlag = 0; }    // They were holding full right but now they're somewhere less
                        if ((ExitSchemeFlag < 0) && (TurnCommand > -CS_MAX_TURN)) { ExitSchemeFlag = 0; }    // They were holding full left but now theyr'e somewhere less
                    }
                    TimeoutFlag = false;    // Regardless, we detected movement, so we reset the timeout flag
                }
                else if (abs(TurnCommand) >= CS_MIN_TURN)
                {   if (!HoldFlag)
                    {   HoldFlag = true;
                        HoldStart = millis();
                    }
                    ExitSchemeFlag = 0;     // If we make it to here they are no longer trying to exit, so reset
                    if ((millis() - HoldStart) >= CSM_ChangeTime_mS)
                    {   HoldFlag = false;
                        if ((millis() - TransitionStart) >= CSM_TimeToWait_mS)    // Only change scheme if we've waited long enough from last command
                        {   if (TurnCommand > 0) 
                            {   CurrentScheme += 1;
                                if (CurrentScheme > NumSchemes) { CurrentScheme = 1; }
                                Serial.print(F("Scheme changed to: ")); Serial.println(CurrentScheme);
                            }
                            else if (TurnCommand < 0)
                            {   CurrentScheme -= 1;
                                if (CurrentScheme < 1) { CurrentScheme = NumSchemes; }
                                Serial.print(F("Scheme changed to: ")); Serial.println(CurrentScheme);
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
                        CSM_ExitStart = millis();
                    }
                    ExitSchemeFlag = 0; // But we also reset this flag, because they are obviously not holding the wheel all the way over either
                    HoldFlag = false;
                }

                BlinkAllLights(CurrentScheme);                
                
                if ((ExitSchemeFlag != 0) && ((millis() - CSM_ExitStart) >= CSM_TimeToExit_mS))
                {   // User is exiting out of Change-Scheme-Mode
                    ChangeSchemeMode = false;
                }
                else if (TimeoutFlag &&  ((millis() - CSM_ExitStart) >= CSM_InactiveExit_Time))
                {   // No movement of steering wheel for a long time - automatic exit
                    ChangeSchemeMode = false;
                }
            }
            while (ChangeSchemeMode);
            PrintLine(80);
            Serial.print(F("EXIT CHANGE-SCHEME MODE IN SCHEME: ")); Serial.println(CurrentScheme);
            PrintLine(80);
            Serial.println();
            TwinkleLights(2000);
            SetLightScheme(CurrentScheme);  // Change the scheme
            SaveScheme_To_EEPROM();      // Save the selection to EEPROM
            PerLoopUpdates();
        }


    // CALCULATE DRIVE MODE FROM COMMAND - The command and mode are not always the same, if DoubleTapReverse = true
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        DriveModeCommand = ReturnDriveMode(ThrottleCommand);    // Figure out what Drive Mode the receiver is indicating

        switch (DriveModeCommand)
        {
            case FWD:                                           
                DriveMode = FWD;                                // Pretty much all ESCs allow us to go directly into forward
                ReverseTaps = 0;                                // Reset reverse taps
                StoppedLongTime = false;                        // Reset the StoppedLongTime flag because we are no longer stopped
                Last_FWD_Time = millis();                       // Keep track of the most current time we are commanding forward, this will be used to set the brake lights if DoubleTapReverse = true
                canChangeScheme = false;                        // Also reset the canChangeScheme flag, we don't want to enter ChangeScheme mode while moving forward
                if (BrakeAtThrottlePctBelow > 0 && canBrake == false && ThrottleCommand > BrakeAtThrottlePctBelow)
                {   
                    Braking = false;                            // Since we are above the threshold, we are not now braking
                    canBrake = true;                            // But as we've exceeded the threshold, we can enable braking so that if we fall back below it the brake State can activate
                }
                
                if (BrakeAtThrottlePctBelow > 0 && canBrake == true && ThrottleCommand <= BrakeAtThrottlePctBelow)
                {
                    Braking = true;                             // We are below the threshold, so we can activate the Braking state
                    canBrake = false;                           // But we also reset the canBrake flag, so if we go back over the threshold the Brake state will be disactivated
                }
                break;

            case REV:
                if (DoubleTapReverse)
                {   // Here we have just started commanding reverse from something else. Start counting taps. 
                    if (DriveModeCommand_Previous != REV)
                    {
                        ReverseTaps +=1;

                        if (ReverseTaps >= 2)
                        {
                            ReverseTaps = 2;
                            DriveMode = REV;
                            StoppedLongTime = false;            // Rset the StoppedLongTime flag because we are no longer stopped
                            canChangeScheme = false;            // Also reset the canChangeScheme flag, we don't want to enter ChangeScheme mode while moving forward
                        }
                        else
                        {
                            if (millis() - Last_FWD_Time < FWD_to_REV_BrakeTime)  // FWD_to_REV_BrakeTime is defined in OSL_Settings.h
                            { 
                                DriveMode = STOP;               // A reverse command here, if it is only the first tap, is counted as a DriveMode = STOP
                                Braking = true;                 // But also, it is counted as braking
                            }
                        }                        
                    }

                    // Here we are actually in reverse, not just commanding it
                    if (DriveMode == REV)
                    {   
                        if (BrakeAtThrottlePctBelow > 0 && canBrake == false && abs(ThrottleCommand) > BrakeAtThrottlePctBelow)
                        {   
                            Braking = false;                            // Since we are above the threshold, we are not now braking
                            canBrake = true;                            // But as we've exceeded the threshold, we can enable braking so that if we fall back below it the brake State can activate
                        }
                        
                        if (BrakeAtThrottlePctBelow > 0 && canBrake == true && abs(ThrottleCommand) <= BrakeAtThrottlePctBelow)
                        {   
                            Braking = true;                             // We are below the threshold, so we can activate the Braking state
                            canBrake = false;                           // But we also reset the canBrake flag, so if we go back over the threshold the Brake state will be disactivated
                        }                        
                    }
                }
                else
                {
                    DriveMode = REV;
                    StoppedLongTime = false;                    // Reset the StoppedLongTime flag because we are no longer stopped
                    canChangeScheme = false;                    // Also reset the canChangeScheme flag, we don't want to enter ChangeScheme mode while moving forward
                    if (BrakeAtThrottlePctBelow > 0 && canBrake == false && abs(ThrottleCommand) > BrakeAtThrottlePctBelow)
                    {   
                        Braking = false;                            // Since we are above the threshold, we are not now braking
                        canBrake = true;                            // But as we've exceeded the threshold, we can enable braking so that if we fall back below it the brake State can activate
                    }
                    
                    if (BrakeAtThrottlePctBelow > 0 && canBrake == true && abs(ThrottleCommand) <= BrakeAtThrottlePctBelow)
                    {
                        Braking = true;                             // We are below the threshold, so we can activate the Braking state
                        canBrake = false;                           // But we also reset the canBrake flag, so if we go back over the threshold the Brake state will be disactivated
                    }              
                }
                break;

            case STOP:
                if (DriveModeCommand_Previous != STOP)
                {   //Previously we were commanding something else, meaning we have just started the stop. 
                    // This tells us we need to start the stop time counter
                    StopCMDLength = millis();
                    // We also reset the braking flag
                    Braking = false;
                    canBrake = false;
                }
                DriveMode = STOP;
                break;
                
        }


    // NOW WE KNOW OUR ACTUAL DRIVE MODE - TAKE CARE OF TURN SIGNALS AND STOP LENGTH
    // ------------------------------------------------------------------------------------------------------------------------------------------------>        
        switch (DriveMode)
        {
            case FWD: 
            case REV:
                if (DriveMode_Previous != DriveMode)
                {   // We've just started moving forwards or backwards. 
                    // Should we leave the turn signal on even though we're now moving forward? 
                    // TurnCommand != 0                 The wheels must be turned   
                    // TurnFromStartContinue_mS > 0     User must have enabled this setting
                    // BlinkTurnOnlyAtStop = true       This must be true, otherwise we just allow blinking all the time, so none of this is necessary
                    // TurnSignal_Enable = true         This means the car has already been stopped for some length of time
                    if (TurnCommand != 0 && TurnFromStartContinue_mS > 0 && BlinkTurnOnlyAtStop && TurnSignal_Enable) 
                    {
                        // In this case we have just begun starting to move, and our wheels are turned at the same time. 
                        // We will keep the turn signals on for a brief period after starting, as set by TurnFromStartContinue_mS
                        TurnSignalOverride = TurnCommand;                                 // TurnSignalOverride saves the turn direction, and will act as a fake turn command in the SetLights function
                        timer.setTimeout(TurnFromStartContinue_mS, ClearBlinkerOverride); // ClearBlinkerOverride will set TurnSignalOverride back to 0 when the timer is up. 
                    }
                }   
                break;

            case STOP:
                // Turn signal timer - start the timer when the car first comes to a stop. 
                if (DriveMode_Previous != STOP)
                {   // The car has only just now come to a stop. We begin a timer. Not until this timer runs out will a turn command (while still stopped)
                    // actually engage the turn signal lights (if we are stills stopped when the timer expires). 
                    // This is only if the user has specified BlinkTurnOnlyAtStop = true. 
                    TurnSignal_Enable = false;
                    TimeStopped = millis();
                    // We use this same timer to delay the option of entering ChangeScheme mode
                    canChangeScheme = false;
                }
                else 
                {   // We have been stopped already. Check to see if the turn signal signal timer is up - if so, we will permit a turn command 
                    // to engage the turn signal lights. This is only if the user has specified BlinkTurnOnlyAtStop = true. 
                    if (TurnSignalDelay_mS > 0 && ((millis() - TimeStopped) >= TurnSignalDelay_mS))
                    {
                        TurnSignal_Enable = true;
                    }        

                    // We have been stopped already. Check to see if enough time has passed to enable entry into Change-Scheme-Mode, if the user desires
                    if (millis() - TimeStopped >= CSM_StopDelay_mS)
                    {
                        canChangeScheme = true;                        
                    }
                    
                    // Check to see if we have been stopped a "long" time, this will enable stop-delay effects
                    if ((millis() - TimeStopped) >= LongStopTime_mS)
                    {
                        StoppedLongTime = true;
                    }
                    
                    // If we are stopped and have been stopped, we are also no longer decelerating, so reset these flags.
                    Decelerating = false;
                    Accelerating = false;
                }
                break;
        }


    // DECELERATING
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Here we are trying to identify sharp deceleration commands (user quickly letting off the throttle). 
        // We need to be moving in a direction (forward or reverse), and still commanding the same direction, but the throttle amount needs to have dropped by DecelPct.
        if ( ((DriveMode == FWD) && (ThrottleCommand >= 0) && (ThrottleCommand < ThrottleCommand_Previous - DecelPct)) ||
             ((DriveMode == REV) && (ThrottleCommand <= 0) && (ThrottleCommand > ThrottleCommand_Previous + DecelPct)) )        
        {   // We are decelerating
            Decelerating = true;
        }
        else
        {   // We are not decelerating, clear
            Decelerating = false;
        }


    // BACKFIRE Enable
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Backfire is confusingly the name of a LED setting (a series of random blinks), but also here canBackfire is a flag that refers to a timed 
        // event that occurs when a heavy deceleration is detected. The length of time the canBackfire event lasts is set in AA_UserConfig.h (BF_Time_Short and BF_Time_Long)
        // During that time, any settings specified under the Deceleration column will take effect, however the only setting that really makes sense there
        // is BACKFIRE. When the timer is up, the lights will revert back to whatever they were previously.         
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
        // - if we are presently moving forward (we don't apply the accelerating state in reverse), and
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
        {   // disable overtaking effect if the timer has run out  
            Overtaking = false;
        }


    // FINALLY - SET THE LIGHTS
    // ------------------------------------------------------------------------------------------------------------------------------------------------>          
        SetLights(DriveMode);        // SetLights will take into account whatever position Channel 3 is in, as well as the present drive mode


    // DEBUGING
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // There is an option in UserConfig.h to have the board LEDs reflect car movement, this can be useful for making sure OSL is reading your radio correctly.
    
        // If moving forward, green led is on, if moving backwards, red led is on. 
        // If stopped, both are off.
        // If braking, both are on. 
        // If right turn, green LED blinks quickly
        // If left turn, red LED blinks quickly
 
        if (LED_DEBUG)
        {
            // First braking, which is not a drive mdoe
            if (Braking)
            { 
                // While braking, both the Red and Green LEDs are On.
                RedLED.on(); 
                GreenLED.on(); 
            } 
            // Secondly, turn
            else
            {
                if (Direction != Direction_Previous || (DriveModeCommand != DriveModeCommand_Previous && DriveModeCommand == STOP))
                {
                    if (Direction == RIGHT_TURN) GreenLED.startBlinking(); else GreenLED.off();     // Right turn   
                    if (Direction == LEFT_TURN)  RedLED.startBlinking();   else RedLED.off();       // Left turn
                    if (Direction == NO_TURN)  { GreenLED.off(); RedLED.off(); }                    // No turn
                }
                // Thirdly, movement forward, back or stop
                else
                {
                    if (DriveMode != DriveMode_Previous || Braking != Braking_Previous)     // We need to add the braking check here as well as change in drive mode, in order to overwrite the LEDs with new values if Braking changes
                    {
                        // Only show the message on changes in DriveMode
                        if (DEBUG && DriveMode_Previous != DriveMode) { Serial.print(F("Drive Mode: ")); Serial.println(printMode(DriveMode)); } // Actual drive mode, not commanded
                        
                        switch (DriveMode)
                        {
                            case FWD: 
                                // When moving forward, the Green LED is on an the Red LED is Off
                                RedLED.off(); 
                                GreenLED.on();
                                break;
                            
                            case REV: 
                                // When moving in reverse, the Red LED is on an the Green LED is Off
                                RedLED.on();  
                                GreenLED.off();
                                break;
                            
                            case STOP: 
                                // When stopped both LEDs are off. But we let the user keep the brake lights on manually by holding brake even after they've stopped. 
                                RedLED.off(); 
                                GreenLED.off();
                                break;
                        }
                    }
                }
            }
        }


    //  SAVE COMMANDS FOR NEXT ITERATION
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // Set previous variables to current
        DriveMode_Previous = DriveMode;
        DriveModeCommand_Previous = DriveModeCommand;
        ThrottleCommand_Previous = ThrottleCommand;
        TurnCommand_Previous = TurnCommand;
        Channel3Command_Previous = Channel3Command;
        Direction_Previous = Direction;
        Braking_Previous = Braking;

    } // End of code that only takes place when we are NOT in failsafe

} // End of Loop
