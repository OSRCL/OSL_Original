
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// SETUPLIGHTS - This assigns the various settings to each of the light states. Run once on startup, and each time the scheme is changed. 
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void SetupLights(int WhatScheme)
{
    int i;
    int j;

    for (i=0; i<NumLights; i++)
    {
        for (j=0; j<NumStates; j++)
        {
            LightSettings[i][j] = pgm_read_word_near(&(Schemes[WhatScheme-1][i][j]));    // WhatScheme is minus -1 because Schemes are zero-based. We let the user use
                                                                                         // one-based numbers for convenience
            PriorLightSetting[i][j] = LightSettings[i][j];                               // Save this to "prior" too. In case we temporarily change a state,
        }                                                                                // this lets us know what we should revert back to. 
    }
    return;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// SETLIGHTS - the main function which assigns the appropriate setting to each light based on the current actual drive mode (different from commanded drive mode)
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
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


        // Least important - does this light have a setting related to Channel 3? 
        // --------------------------------------------------------------------------------------------------->>
        SaveSetting[j] = LightSettings[j][Channel3];


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
                if (LightSettings[j][StateStop] != NA) { SaveSetting[j] = LightSettings[j][StateStop]; }
                break;
        }


        // Next - does this light come on during deceleration (probably Backfiring?)
        // --------------------------------------------------------------------------------------------------->>        
        if (canBackfire)
        {
//            if (LightSettings[j][StateDecel] != NA) { SaveSetting[j] = BACKFIRE; } // Override setting - we assume the only setting they want during decel is BACKFIRE
            if (LightSettings[j][StateDecel] != NA) { SaveSetting[j] = LightSettings[j][StateDecel]; } // Or we can allow any setting during deceleration
        }

        // Next - 
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
        if (TurnCommand > 0)    // Right Turn
        {
            // If we have a blink command on right turn, and if we have the BlinkTurnOnlyAtStop = true, 
            // then we only appy the turn signal if we are stopped AND if the turn signal delay has expired (TurnSignal_Enable = true)
            if ((LightSettings[j][StateRT] == BLINK || LightSettings[j][StateRT] == SOFTBLINK) && (BlinkTurnOnlyAtStop == true))
            {
                if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateRT]; }
            }
            // Otherwise if it is any other setting, or if the BlinkTurnOnlyAtStop flag is not true, then we apply the setting normally
            else if (LightSettings[j][StateRT] != NA) { SaveSetting[j] = LightSettings[j][StateRT]; }
        }
        if (TurnSignalOverride > 0) // Artificial Right Turn
        {
            // In this case we want to artificially create a turn signal even though the wheel may or may not be turned.
            // We ignore driving state or TurnSignal_Enable state 
            if (LightSettings[j][StateRT] == BLINK || LightSettings[j][StateRT] == SOFTBLINK) { SaveSetting[j] = LightSettings[j][StateRT]; }
        }

        if (TurnCommand < 0 || TurnSignalOverride < 0)    // Left Turn
        {
            // If we have a blink command on left turn, and if we have the BlinkTurnOnlyAtStop = true, 
            // then we only appy the turn signal if we are stopped AND if the turn signal delay has expired (TurnSignal_Enable = true)
            if ((LightSettings[j][StateLT] == BLINK || LightSettings[j][StateLT] == SOFTBLINK) && (BlinkTurnOnlyAtStop == true))
            {
                if ((DriveMode == STOP) && (TurnSignal_Enable == true)) { SaveSetting[j] = LightSettings[j][StateLT]; }
            }
            // Otherwise if it is any other setting, or if the BlinkTurnOnlyAtStop flag is not true, then we apply the setting normally
            else if (LightSettings[j][StateLT] != NA) { SaveSetting[j] = LightSettings[j][StateLT]; }
        }
        if (TurnSignalOverride < 0) // Artificial Left Turn
        {
            // In this case we want to artificially create a turn signal even though the wheel may or may not be turned.
            // We ignore driving state or TurnSignal_Enable state 
            if (LightSettings[j][StateLT] == BLINK || LightSettings[j][StateLT] == SOFTBLINK) { SaveSetting[j] = LightSettings[j][StateLT]; }
        }
   
        // Light "j" now has a single setting = SaveSetting[j]
        // We call the function that will set this light to that setting
        SetLight(j, SaveSetting[j]);
    }

    // Now we have done looping through all the lights, and setting them all. 

    // There is an option in UserConfig.h to have the board LEDs reflect car movement, this 
    // can sometimes be useful for debugging. 

    // If moving forward, green led is on, if moving backwards, red led is on. 
    // If stopped, both are off.
    // If braking, both are on. 
    // If right turn, green LED blinks quickly
    // If left turn, red LED blinks quickly
    if (LED_DEBUG)
    {
        if (DriveMode == FWD)  { digitalWrite(RedLED , LOW ) ; digitalWrite(GreenLED, HIGH) ; }
        if (DriveMode == REV)  { digitalWrite(RedLED , HIGH) ; digitalWrite(GreenLED, LOW ) ; }
        if (DriveMode == STOP) { digitalWrite(RedLED , LOW ) ; digitalWrite(GreenLED, LOW ) ; }    
        if (Braking == true)   { digitalWrite(RedLED , HIGH) ; digitalWrite(GreenLED, HIGH) ; }
        if (TurnCommand > 0)   { GreenBlink(); }    // Right turn
        if (TurnCommand < 0)   { RedBlink();   }    // Left turn
    }

}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// DUMPLIGHTSCHEMETOSERIAL - Show each setting for each state for each light in tabular format out the serial port
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void DumpLightSchemeToSerial(int WhatScheme)
{
    int i;
    int j;
    int MySetting;
    
    if (WhatScheme <= NumSchemes)
    {   // Schemes are zero-based, so if they pass Scheme 1 what we really want to show is Scheme 0
        WhatScheme -= 1;
        Serial.print(F("SCHEME: "));
        Serial.print(WhatScheme+1);
        Serial.println();
        for (i=0; i<NumLights; i++)
        {
            Serial.print(F("Light #"));
            Serial.print(i+1);
            Serial.print(F(" States: "));
            for (j=0; j<NumStates; j++)
            {
                MySetting = pgm_read_word_near(&(Schemes[WhatScheme][i][j]));    
                Serial.print(MySetting, DEC);
                Serial.print(F("  "));
            }
            Serial.println();
        }
    }
    return;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// SETLIGHT - This sets an individual light to a specific setting
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void SetLight(int WhatLight, int WhatSetting)
{ 
    switch (WhatSetting)
    {
        case ON:
            TurnOnLight(WhatLight);
            break;
        case OFF:
            TurnOffLight(WhatLight);
            break;
        case BLINK:
            BlinkLight(WhatLight);
            break;
        case SOFTBLINK:
            SoftBlinkLight(WhatLight);
            break;
        case FASTBLINK:
            FastBlinkLight(WhatLight);
            break;
        case DIM:
            DimLight(WhatLight);
            break;
        case BACKFIRE:
            LightBackfire(WhatLight);
            break;            
        case XENON:
            TurnOnXENONLight(WhatLight);
            break;            
        case FADEOFF:
            FadeOffLight(WhatLight);
            break;            
    }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// TURNONLIGHT - Turns a light on
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void TurnOnLight(int WhatLight)
{
    digitalWrite(LightPin[WhatLight], HIGH);
    PWM_Step[WhatLight] = 255;
    FadeOffReset(WhatLight);
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// TURNOFFLIGHT - Turns a light off
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void TurnOffLight(int WhatLight)
{
    digitalWrite(LightPin[WhatLight], LOW);
    PWM_Step[WhatLight] = 0;
    XenonReset(WhatLight);
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// TURNONXENONLIGHT - Turns a light on with XENON effect
// All credit for Xenon effects code goes to Sergio Pizzotti
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void TurnOnXENONLight(int WhatLight)
{
    // In order to prevent a Xenon effect halfway-complete from interfering with the FadeOff effect, we clear the FadeOff flag at the beginning
    FadeOffReset(WhatLight);

    // Only certain lights work with the Xenon effect (lights 1-6). We do a check here to make sure. 
    // If it is not an option on this pin, but the command was still issued, we instead turn it ON. 
    if (Dimmable[WhatLight])
    {   // Ok, we can proceed
        if (Xenon_EffectDone[WhatLight] == 1) {
            // Xenon effect done, turn on light
            TurnOnLight(WhatLight);
        }
        else
        {
            if (Xenon_EffectDone[WhatLight] == -1)
            {   // We are coming to this effect from DIM. Unless we knew that, this effect woudln't begin from the beginning. 
                // Start from the beginning:
                Xenon_EffectDone[WhatLight] = 0;
                Xenon_Step[WhatLight] = 0;
            }
            if(millis() - Xenon_millis[WhatLight] > Xenon_interval) 
            {   // save the current time for the current light
                Xenon_millis[WhatLight] = millis();
            
                // Select the appropriate step for the Xenon light effect
                switch(Xenon_Step[WhatLight]) 
                {
                    case 0:  // first step: turn on the light
                        analogWrite(LightPin[WhatLight], 255);
                        break;
                
                    case 1: // second step: turn off the light for twice the time
                    case 2:
                        analogWrite(LightPin[WhatLight], 0);
                        break;
                
                    default: /// third step: fade the light to full (or quite full) light
                        analogWrite(LightPin[WhatLight], Xenon_Step[WhatLight]);
                        break;
                }
                if (PWM_Step[WhatLight] <= 250)
                {   // Next step in the Xenon effect
                    Xenon_Step[WhatLight]++;
                    // Save our current PWM value in case we transition to a different effect
                    PWM_Step[WhatLight] = Xenon_Step[WhatLight];
                } 
                else 
                {    // We are at full light, save state for next loop
                    Xenon_Step[WhatLight] = 0;
                    Xenon_millis[WhatLight] = 0;
                    Xenon_EffectDone[WhatLight] = 1;
                }
            }
        }
    }
    else
    {   // The Xenon effect is not an option on this pin. Just turn it on
        TurnOnLight(WhatLight);
        Xenon_Step[WhatLight] = 1;    // Effect done.
    }
}

void XenonReset(int WhatLight)
{
    // reset the light state
    Xenon_Step[WhatLight] = 0;
    Xenon_EffectDone[WhatLight] = 0;
    Xenon_millis[WhatLight] = 0;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// FADEOFFLIGHT - Slowly turn off light
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void FadeOffLight(int WhatLight)
{
    unsigned int brightness=0xffff;

    // Only certain lights can be faded (lights 1-6). We do a check here to make sure. 
    // If it is not an option on this pin, but the command was still issued, we instead turn it off. 
    if (Dimmable[WhatLight])
    {   // Ok, we can proceed
        if (FadeOff_EffectDone[WhatLight] == 0) 
        {   // This completely interrupts everything else while the fade is occuring, otherwise it looks too choppy and unrealistic. 
            // Downside, nothing else gets processed while this is happening. Even worse, if you have multiple lights set to FadeOff on the 
            // same state, they will go sequentially one-by-one, not all at the same time. This looks cool, but takes even more time. 
            // Using code from: http://forum.arduino.cc/index.php?topic=205417.msg1512121#msg1512121
            brightness = PWM_Step[WhatLight]*PWM_Step[WhatLight];
            while (brightness > 255) 
            {
                analogWrite(LightPin[WhatLight],brightness>>8);
                brightness -= brightness>>2;
                // About 20 steps from full brightness to off. With a delay of 25, that means 1/2 second to fade off
                delay(25);
            }
            TurnOffLight(WhatLight);    // TurnOffLight will also reset the PWM_Step 
            FadeOff_EffectDone[WhatLight] = 1;
        }
        else
        {   // Shouldn't come here, but if we do, the effect is done, turn the light off
            TurnOffLight(WhatLight);    // TurnOffLight will also reset the PWM_Step
        } 
    }
    else
    {   // No, this is not a PWM pin, we can't fade. Just turn it off. 
        TurnOffLight(WhatLight);        // TurnOffLight will also reset the PWM_Step
        FadeOff_EffectDone[WhatLight] = 1;
    }
}
void FadeOffReset(int WhatLight)
{
    // reset the light state
    FadeOff_EffectDone[WhatLight] = 0;
}

void FadeBlink(int WhatLight, boolean fadeUp, int mS)
{
    unsigned int brightness=0xffff;
    float fbrightness;
    int startLow = 275;

    // Only certain lights can be faded (lights 1-6). We do a check here to make sure. 
    // If it is not an option on this pin, but the command was still issued, we instead turn it off. 
    if (Dimmable[WhatLight])
    {   // Ok, we can proceed
        // This completely interrupts everything else while the fade is occuring, otherwise it looks too choppy and unrealistic. 
        // Downside, nothing else gets processed while this is happening. Even worse, if you have multiple lights set to FadeOff on the 
        // same state, they will go sequentially one-by-one, not all at the same time. This looks cool, but takes even more time. 

        if (fadeUp) // fade from off up to bright
        {
            fbrightness = startLow;
            while (fbrightness < 65535.0) 
            {
                brightness = (unsigned int)fbrightness>>8;
                analogWrite(LightPin[WhatLight],brightness);
                fbrightness += (fbrightness / 3);
                delay(mS);
            }
            TurnOnLight(WhatLight);             // We should now be at full brightness
        }
        else        // Fade from bright (or dim) down to off
        {
            // Start at whatever level the light is already at
            brightness = PWM_Step[WhatLight]*PWM_Step[WhatLight];
            while (brightness > 255) 
            {
                analogWrite(LightPin[WhatLight],brightness>>8);
                brightness -= brightness>>2;
                delay(mS);
            }
            TurnOffLight(WhatLight);    // TurnOffLight will also reset the PWM_Step 
        }
    }
    else
    {   // No, this is not a PWM pin, we can't fade. Just blink it on/off
        fadeUp ? TurnOnLight(WhatLight) : TurnOffLight(WhatLight);
    }
}

    


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// LIGHTBACKFIRE - briefly and randomly light a led
// All credit for backfiring code goes to Sergio Pizzotti
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void LightBackfire(int WhatLight)
{
    if (canBackfire) 
    {   // Has enough time passed to flicker the backfire LED?
        if(millis() - Backfire_millis > backfire_interval) 
        {   // Save time for next check
            Backfire_millis = millis();   
            // Change state of backfire LED
            for (int i=0; i<NumLights; i++)
            {    
                if (LightSettings[i][StateDecel] == BACKFIRE) {ReverseLight(i); }
            }
            // Calculate new random interval for the next flicker
            backfire_interval = random(BF_Short, BF_Long);
        }
    }
}

void BackfireOff()
{
    // Time up - stop backfire effect
    canBackfire = false;
    // Reset the random backfire timeout for the next event
    backfire_timeout = BF_Time + random(BF_Short, BF_Long);
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// OVERTAKEOFF - Overtaking time is up
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void OvertakeOff()
{
    // Time up - stop the overtake effects
    Overtaking = false;
    for (int i=0; i<NumLights; i++)
    {   // The overtaking effect can cause a Xenon effect to re-start, so in the event a Xenon effect is defined for this same light,
        // we go ahead and flag it complete. 
        if (LightSettings[i][StateAccel] != NA) { Xenon_EffectDone[i] = 1; }
    }
}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// TURN SIGNAL - Artificial turn signal cancel
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// If TurnSignalOverride is not zero, then it acts as an artificial turn signal command, keeping the turn signals on even after the car has begun
// to move forward. But it only does so for a brief period of time (set by the user in TurnFromStartContinue_mS in UserConfig.h). When that time
// runs out this function is called which sets TurnSignalOverride back to 0. 
void ClearBlinkerOverride(void)
{
    TurnSignalOverride = 0;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// BLINKLIGHT - This blinks a light
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void BlinkLight(int WhatLight)
{
    Blinker ? TurnOnLight(WhatLight) : TurnOffLight(WhatLight);
}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// SOFTBLINKLIGHT - This blinks a light but the light fades in and fades out
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void SoftBlinkLight(int WhatLight)
{
    // Only blink if blinker status has changed
    if (IndividualLightBlinker[WhatLight] != Blinker)
    {
        // FadeBlink takes three arguments:
        // 1. Light number
        // 2. Direction (true means fade up, false means fade down)
        // 3. delay per step in milli-seconds. 
        // The fade effect takes 20 steps to complete from full off to on (or vice versa). 
        // Tweak the delay for the total length of time you want the fade to take, ie FadeLength = (20 * delay)
        // For example, if you set the delay to 10 for both up and down, it will take (10*20) = 200 mS to fade in, and 200 mS to fade out, so 400mS total.
        // That would be very slow. Remember also that you shouldn't exceed the total time for "BlinkInterval", which is set in AA_UserConfig, 
        // Otherwise the light won't be done fading out before the next blink starts. 
        Blinker ? FadeBlink(WhatLight, true, SoftBlinkFadeInDelay) : FadeBlink(WhatLight, false, SoftBlinkFadeOutDelay);
        IndividualLightBlinker[WhatLight] = Blinker;
    }
}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// FASTBLINKLIGHT - This blinks a light at the fast rate
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void FastBlinkLight(int WhatLight)
{
    FastBlinker ? TurnOnLight(WhatLight) : TurnOffLight(WhatLight);
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// DIMLIGHT - This dims a light
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void DimLight(int WhatLight)
{

    // Clear the FadeOff flag at the beginning, so if we transition from DIM to FadeOff the fade will occur
    FadeOffReset(WhatLight);
    
    // Only certain lights can be dimmed. We do a check here to make sure. 
    // If dimming is not an option on this pin, but the command was still issued, 
    // we instead turn it off. 
    if (Dimmable[WhatLight])
    {
        // NOTE: Only use the ActualDimLevel variable in this function, do not use DimLevel from UserConfig.h
        SetLightLevel(WhatLight, ActualDimLevel);
        // Set a flag so that if we transition from DIM to XENON the Xenon effect will know to start from the beginning, 
        // instead of from the DIM level
        Xenon_EffectDone[WhatLight] = -1;
    }
    else
    {
        TurnOffLight(WhatLight);
    }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// FIXDIMLEVEL - Sets the ActualDimLevel and prevents the bug that occurs if the User strangely set DimLevel to less than 2 (unlikely)
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void FixDimLevel()
{
    if (DimLevel < 2) 
    { ActualDimLevel = 2; }
    else
    { ActualDimLevel = DimLevel; }
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// SETLIGHTLEVEL - Set an individual light to a PWM level, save that in PWM_Step
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void SetLightLevel(int WhatLight, int WhatLevel)
{
    if (Dimmable[WhatLight])
    {
        analogWrite(LightPin[WhatLight], WhatLevel);
        PWM_Step[WhatLight] = WhatLevel;
    }
} 



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// TWINKLELIGHTS - Sets all lights to FastBlink for Seconds seconds
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void TwinkleLights(int Seconds)
{
    int j;
    StartWaiting_sec(Seconds);
    do
    {
        for (j=0; j<NumLights; j++)
        {    
            FastBlinkLight(j);
        }
        timer.run();
    } 
    while(!TimeUp);
}
   


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// BLINKALLLIGHTS - Blinks every single light, onboard and off, HowManyTimes. This is used as an indicator during Change-Scheme-Mode
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void BlinkAllLights(int HowManyTimes)
{
    if (Blinking)
    {
        if (State)
        {    
            if (!PriorState)
            {    
                // Turn everything on
                GreenLedOn();
                RedLedOn();
                for (int j=0; j<NumLights; j++)
                {   TurnOnLight(j); } 
                PriorState = true;
                TimesBlinked += 1;
                timer.setTimeout(130, BlinkOn);
            }
        }
        else
        {
            if (PriorState)
            {
                // Turn everything off
                GreenLedOff();
                RedLedOff();
                for (int j=0; j<NumLights; j++)
                {   TurnOffLight(j); }        
                PriorState = false;
                BlinkOffID = timer.setTimeout(160, BlinkOff);
            }

            if (TimesBlinked >= HowManyTimes)
            {   TimesBlinked = 0;
                Blinking = false;
                timer.setTimeout(1000, BlinkWait);
                timer.deleteTimer(BlinkOffID);
            }

        }
    }
}

void BlinkOn()
{
    Blinking = true;
    State = false;
    PriorState = true;
}

void BlinkOff()
{
    Blinking = true;
    State = true;
    PriorState = false;
}

void BlinkWait()
{
    Blinking = true;
    State = true;
    PriorState = false;
}

void ReverseLight(int WhatLight)
{
    int OppositeVal;
    // If light is on, turn it off. But if light is off, turn it on.
    OppositeVal = !digitalRead(LightPin[WhatLight]);
    digitalWrite(LightPin[WhatLight], OppositeVal);
    PWM_Step[WhatLight] = OppositeVal;
}

void ReturnToPriorState(int WhatLight, int WhatState)
{
       LightSettings[WhatLight][WhatState] = PriorLightSetting[WhatLight][WhatState];
}
