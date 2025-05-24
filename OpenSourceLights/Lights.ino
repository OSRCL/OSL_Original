
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// SETLIGHTSCHEME - This assigns the various settings to each of the light states. Run once on startup, and each time the scheme is changed. 
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void SetLightScheme(int WhatScheme)
{
    int i;
    int j;

    for (i=0; i<NumLights; i++)
    {
        for (j=0; j<NumStates; j++)
        {
            LightSettings[i][j] = pgm_read_word_near(&(Schemes[WhatScheme-1][i][j]));    // WhatScheme is minus -1 because Schemes are zero-based. We let the user use
        }                                                                                // one-based numbers for convenience
    }
    return;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// SETLIGHTS - the main function which assigns the appropriate setting to each light based on the current actual drive mode (different from commanded drive mode)
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void SetLights(int DriveMode)
{
    int SaveSetting[NumLights];
    int j;

    // Loop through each light, assign the setting appropriate to its state
    for (j=0; j<NumLights; j++)
    {
        // We will use the temporary variable SaveSetting to assign the setting for this light. 
        // A light could have multiple settings apply at one time, be we can only set it to one thing.
        // Therefore each setting is ranked by importance. If multiple settings apply to a light, 
        // the setting applied LAST will be the one used (each check can overwrite the prior one). 
        // You can re-order the checks below, the least important should come first, and most important last.
        
        if (shelfQueenMode == true)
        {   
            // In shelf-queen mode we only apply channel 3, and only position 1 (0)
            SaveSetting[j] = LightSettings[j][ShelfQueenCh3Position];
        }
        else
        {
            // Least important - does this light have a setting related to Channel 3? 
            // --------------------------------------------------------------------------------------------------->>
            SaveSetting[j] = LightSettings[j][Channel3Command];
    
            // Next - does this light have a setting related to Drive Mode? (Forward, reverse, stop)
            // --------------------------------------------------------------------------------------------------->>
            switch (DriveMode) {
                case FWD:
                    if (LightSettings[j][StateFwd]  != NA) { SaveSetting[j] = LightSettings[j][StateFwd]; }
                    break;
                case REV:
                    if (LightSettings[j][StateRev]  != NA) { SaveSetting[j] = LightSettings[j][StateRev]; }
                    break;
                case STOP:
                    // We have two stop states: 
                    // StateStop occurs when the vehicle stops
                    // StateStopDelay occurs when the vehicle has been stopped for LongStopTime_mS and will supersede StateStop when it occurs (if not NA)
                    if (LightSettings[j][StateStop] != NA) { SaveSetting[j] = LightSettings[j][StateStop]; }
                    if (LightSettings[j][StateStopDelay] != NA && StoppedLongTime == true) { SaveSetting[j] = LightSettings[j][StateStopDelay]; }
                    break;
            }
    
    
            // Next - does this light come on during deceleration (probably Backfiring?)
            // --------------------------------------------------------------------------------------------------->>        
            if (canBackfire)
            {
            //  if (LightSettings[j][StateDecel] != NA) { SaveSetting[j] = BACKFIRE; } // Override setting - we assume the only setting they want during decel is BACKFIRE
                if (LightSettings[j][StateDecel] != NA) { SaveSetting[j] = LightSettings[j][StateDecel]; } // Or we can allow any setting during deceleration
            }

    
            // Next - does this light come on during acceleration (aka, Overtaking?)
            // --------------------------------------------------------------------------------------------------->>        
            if (Overtaking)
            {
                if (LightSettings[j][StateAccel] != NA) { SaveSetting[j] = LightSettings[j][StateAccel]; } 
            }
    
            
            // Next - does this light come on during braking?
            // --------------------------------------------------------------------------------------------------->>        
            if (Braking)
            {
                if (LightSettings[j][StateBrake] != NA) { SaveSetting[j] = LightSettings[j][StateBrake]; }
            }
    
            
            // Next - does this light come on during turns? 
            // --------------------------------------------------------------------------------------------------->>        
            if (TurnCommand > 0 || TurnSignalOverride > 0)    // Right Turn
            {
                // If we have a blink command on right turn, and if we have the BlinkTurnOnlyAtStop = true, 
                // then we only appy the turn signal if we are stopped AND if the turn signal delay has expired (TurnSignal_Enable = true)
                if ((LightSettings[j][StateRT] == BLINK || LightSettings[j][StateRT] == SOFTBLINK) && (BlinkTurnOnlyAtStop == true))
                {
                    if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateRT]; }
                }
                // Same as above except for all other settings under turn
                else if (LightSettings[j][StateRT] != NA && AllTurnSettingsMatch == true )
                {
                    if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateRT]; }
                }
                // Otherwise if it is any other setting, or if the BlinkTurnOnlyAtStop flag and the AllTurnSettingsMatch are not true, then we apply the setting normally
                else if (LightSettings[j][StateRT] != NA) { SaveSetting[j] = LightSettings[j][StateRT]; }
            }
            if (TurnSignalOverride > 0) // Artificial Right Turn
            {   
                // In this case we want to artificially create a turn signal even though the wheel may or may not be turned.
                // We ignore driving state or TurnSignal_Enable state 
                if (LightSettings[j][StateRT] == BLINK || LightSettings[j][StateRT] == SOFTBLINK) { SaveSetting[j] = LightSettings[j][StateRT]; }
                // We may also want to artificially create any setting assigned to the turn state
                else if (AllTurnSettingsMatch)                                                    { SaveSetting[j] = LightSettings[j][StateRT]; }
            }
    
            if (TurnCommand < 0 || TurnSignalOverride < 0)    // Left Turn
            {
                // If we have a blink command on left turn, and if we have the BlinkTurnOnlyAtStop = true, 
                // then we only appy the turn signal if we are stopped AND if the turn signal delay has expired (TurnSignal_Enable = true)
                if ((LightSettings[j][StateLT] == BLINK || LightSettings[j][StateLT] == SOFTBLINK) && (BlinkTurnOnlyAtStop == true))
                {
                    if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateLT]; }
                }
                // Same as above except for all other settings under turn
                else if (LightSettings[j][StateLT] != NA && AllTurnSettingsMatch == true )
                {
                    if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateLT]; }
                }
                // Otherwise if it is any other setting, or if the BlinkTurnOnlyAtStop flag and the AllTurnSettingsMatch are not true, then we apply the setting normally
                else if (LightSettings[j][StateLT] != NA) { SaveSetting[j] = LightSettings[j][StateLT]; }
            }
            if (TurnSignalOverride < 0) // Artificial Left Turn
            {
                // In this case we want to artificially create a turn signal even though the wheel may or may not be turned.
                // We ignore driving state or TurnSignal_Enable state 
                if (LightSettings[j][StateLT] == BLINK || LightSettings[j][StateLT] == SOFTBLINK) { SaveSetting[j] = LightSettings[j][StateLT]; }
                // We may also want to artificially create any setting assigned to the turn state
                else if (AllTurnSettingsMatch)                                                    { SaveSetting[j] = LightSettings[j][StateLT]; }
            }
    
            if (TurnCommand == 0)       // No turn
            {
                if (LightSettings[j][StateNT] != NA) { SaveSetting[j] = LightSettings[j][StateNT]; }               
            }
        }
        

        // Light "j" now has a single setting = SaveSetting[j]
        // --------------------------------------------------------------------------------------------------->>          
        // We call the function that will set this light to that setting
        if (CurrentLightSetting[j] != SaveSetting[j])   // But only update the setting if it is different from the current setting
        {   
            CurrentLightSetting[j] = SaveSetting[j]; 
            SetLight(j, SaveSetting[j]);
            if (DEBUG) PrintLightSetting(j, CurrentLightSetting[j]);
        }
    }
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// SETLIGHT - This sets an individual light to a specific setting
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void SetLight(int WhatLight, int WhatSetting)
{
    switch (WhatSetting)
    {
        case ON:
            LightOutput[WhatLight].on();
            break;
        
        case OFF:
            LightOutput[WhatLight].off();
            break;
        
        case FADEOFF:
            LightOutput[WhatLight].Fade(FADE_OUT, FadeOutTime);
            break;         

        case FADEON:
            LightOutput[WhatLight].Fade(FADE_IN, FadeInTime, FADE_TYPE_SINE);   // We don't usually use the sine wave fade but for fading-in it looks better than exponential. 
            break;       
            
        case BLINK:
            LightOutput[WhatLight].startBlinking(BlinkInterval, BlinkInterval, false);
            break;

        case BLINK_ALT:
            LightOutput[WhatLight].startBlinking(BlinkInterval, BlinkInterval, true);
            break;            
            
        case FASTBLINK:
            LightOutput[WhatLight].startBlinking(FastBlinkInterval, FastBlinkInterval, false);
            break;
                                
        case FASTBLINK_ALT:
            LightOutput[WhatLight].startBlinking(FastBlinkInterval, FastBlinkInterval, true);
            break;
            
        case SAFETYBLINK:       
            LightOutput[WhatLight].SafetyBlink(SafetyBlinkRate, SafetyBlinkCount, SafetyBlink_Pause, false);
            break;

        case SAFETYBLINK_ALT:
            LightOutput[WhatLight].SafetyBlink(SafetyBlinkRate, SafetyBlinkCount, SafetyBlink_Pause, true);
            break;
        
        case SOFTBLINK:
            LightOutput[WhatLight].softBlink();
            break;
       
        case DIM:
            LightOutput[WhatLight].dim(DimLevel);
            break;

        case XENON:
            LightOutput[WhatLight].Xenon();
            break; 
            
        case BACKFIRE:
            LightOutput[WhatLight].randomBlink();
            break;            

        default:
            break;               
    }

    return;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// LIGHTBACKFIRE - briefly and randomly light a led
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void BackfireOff()
{
    // Time up - stop backfire effect
    canBackfire = false;
    // Reset the random backfire timeout for the next event
    backfire_timeout = random(BF_Time_Short, BF_Time_Long);
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// OVERTAKEOFF - Overtaking time is up
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void OvertakeOff()
{
    // Time up - stop the overtake effects
    Overtaking = false;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// TURN SIGNAL - Artificial turn signal cancel
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// If TurnSignalOverride is not zero, then it acts as an artificial turn signal command, keeping the turn signals on even after the car has begun
// to move forward. But it only does so for a brief period of time (set by the user in TurnFromStartContinue_mS in UserConfig.h). When that time
// runs out this function is called which sets TurnSignalOverride back to 0. 
void ClearBlinkerOverride(void)
{
    TurnSignalOverride = 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// TWINKLELIGHTS - Sets all lights to FastBlink for Seconds seconds, used to indicate entry and exit from Change-Scheme-Mode
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void TwinkleLights(int mS)
{
    // Start rapid blinking
    GreenLED.startBlinking(FastBlinkInterval, FastBlinkInterval);
    RedLED.startBlinking(FastBlinkInterval, FastBlinkInterval);
    for (int i=0; i<NumLights; i++)
    {    
        LightOutput[i].startBlinking(FastBlinkInterval, FastBlinkInterval);
    }

    // Wait desired seconds
    delayWhilePolling(mS);

    // End with off
    for (int i=0; i<NumLights; i++)
    {    
        LightOutput[i].off();
    }
    GreenLED.off();
    RedLED.off();
}
   

// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// BLINKALLLIGHTS - Blinks every single light, onboard and off, HowManyTimes. Used to indicate the currently selected scheme during Change-Scheme-Mode
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void BlinkAllLights(int HowManyTimes)
{
    if (CSM_Blinking)
    {
        if (CSM_State)
        {    
            if (!CSM_PriorState)
            {    
                // Turn everything on
                GreenLED.on();
                RedLED.on();
                for (int j=0; j<NumLights; j++)
                {   LightOutput[j].on(); } 
                CSM_PriorState = true;
                CSM_TimesBlinked += 1;
                timer.setTimeout(130, BlinkOn);
            }
        }
        else
        {
            if (CSM_PriorState)
            {
                // Turn everything off
                GreenLED.off();
                RedLED.off();
                for (int j=0; j<NumLights; j++)
                {   LightOutput[j].off(); }
                CSM_PriorState = false;
                CSM_BlinkOffTimerID = timer.setTimeout(160, BlinkOff);
            }

            if (CSM_TimesBlinked >= HowManyTimes)
            {   CSM_TimesBlinked = 0;
                CSM_Blinking = false;
                timer.setTimeout(1000, BlinkWait);
                timer.deleteTimer(CSM_BlinkOffTimerID);
            }
        }
    }
}

void BlinkOn()
{
    CSM_Blinking = true;
    CSM_State = false;
    CSM_PriorState = true;
}

void BlinkOff()
{
    CSM_Blinking = true;
    CSM_State = true;
    CSM_PriorState = false;
}

void BlinkWait()
{
    CSM_Blinking = true;
    CSM_State = true;
    CSM_PriorState = false;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
// Blink lights if the radio signal is lost. User can decide whether only to blink the onboard red and green LEDs, or all the external lights as well.
// ------------------------------------------------------------------------------------------------------------------------------------------------------->  
void StartFailsafeLights()
{
//#define FailsafeBlinkRate 50
    
    // Start the failsafe blinker if it isn't already
    if (timer.isEnabled(RxSignalLostTimerID) == false)   
    {   
        AllLightsOff();     // Turn all lights off to start
        RxSignalLostTimerID = timer.setInterval(BLINK_RATE_LOST_SIGNAL, BlinkLights_RX_SignalLost); // BLINK_RATE_LOST_SIGNAL is defined in OSL_Settings.h
    }
}

void StopFailsafeLights()
{
    // Quit the failsafe blinker
    if (timer.isEnabled(RxSignalLostTimerID))
    {
        timer.deleteTimer(RxSignalLostTimerID);
        RxSignalLostTimerID = 0;
        AllLightsOff();
    }
}

void BlinkLights_RX_SignalLost()
{
    static boolean Status; 

    // Flip flop the Red and Green LEDs
    Status ? RedLED.on() : RedLED.off();
    Status ? GreenLED.off() : GreenLED.on();

    // Blink the eight outputs, but only if the user wants it
    if (BLINK_LIGHTS_RX_LOST)
    {
        // Flip flop every other external light
        for (int j=0; j<NumLights; j += 2)
        {    
            Status ? LightOutput[j].on()    : LightOutput[j].off();
            Status ? LightOutput[j+1].off() : LightOutput[j+1].on(); 
        }
    }
    
    // Flop the flip
    Status = !Status;
}


void AllLightsOn()
{
    RedLED.on();
    GreenLED.on();
    for (int i=0; i<NumLights; i++)
    {    
        LightOutput[i].on();
        CurrentLightSetting[i] = ON;
    }
}

void AllLightsOff()
{
    RedLED.off();
    GreenLED.off();
    for (int i=0; i<NumLights; i++)
    {    
        LightOutput[i].off();
        CurrentLightSetting[i] = OFF;
    }
}
