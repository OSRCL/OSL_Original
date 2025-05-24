
void PerLoopUpdates()
{
    // Handle any radio pulses that have come in
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        // RC signals are measured through pin change ISRs (interrupt service routines). The signal starts on a rising edge and ends on a falling edge, the time between them is recorded 
        // and a flag is then set. ProcessChannelPulses checks each channel for the presence of this flag, checks the pulse width and if valid takes whatever action is required. 
        ProcessChannelPulses();
        // The RC pin change ISRs will try to determine the status of each channel, but of course if a channel becomes disconnected its ISR won't even trigger. 
        // So we also force a check from the main loop, but only if we are not in shelf-queen mode
        if (!shelfQueenMode) CheckRCStatus();

    
    // Per loop updates that have to be polled
    // ------------------------------------------------------------------------------------------------------------------------------------------------>      
        timer.run();                            // SimpleTimer object, used for various timing tasks. Must be polled. 
        InputButton.read();                     // Button must be polled
        RedLED.update();                        // Led handlers must be polled
        GreenLED.update();                      // " "
        for (uint8_t i=0; i<NumLights; i++)    // " "
        {
            LightOutput[i].update();
        }
}

void delayWhilePolling(uint16_t waitTime)
{
    elapsedMillis wait; 
    do {
        PerLoopUpdates();
    } while (wait < waitTime);
}

void DumpSystemInfo()
{
    // Give the radio some time to detect channels
    elapsedMillis wait_a_bit;
    do { 
        PerLoopUpdates();
    } while (wait_a_bit < 250);
    
    // Hardware version
    Serial.println();
    Serial.println();
    PrintLine(80);
    Serial.print(F("HARDWARE VERSION: ")); Serial.println(HardwareVersion);
    PrintLine(80);
    Serial.println(); 
        
    // Channel pulse values 
    Serial.println();
    Serial.println(F("CHANNEL SETTINGS"));
    PrintLine(80);
    Serial.println(F("Channel       Min       Center    Max       Reversed    Smoothed    Status"));
    PrintLine(80);
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        PerLoopUpdates();
        PrintChannelName(i, true);  // True for padding after the word
        Serial.print(RC_Channel[i].pulseMin);
        if (RC_Channel[i].pulseMin < 1000) PrintSpaces(7); else PrintSpaces(6);
        Serial.print(RC_Channel[i].pulseCenter);
        if (RC_Channel[i].pulseCenter < 1000) PrintSpaces(7); else PrintSpaces(6);
        Serial.print(RC_Channel[i].pulseMax);
        if (RC_Channel[i].pulseMax < 1000) PrintSpaces(7); else PrintSpaces(6);
        PrintYesNo(RC_Channel[i].reversed);
        if (RC_Channel[i].reversed == true) PrintSpaces(9); else PrintSpaces(10);
        PrintYesNo(RC_Channel[i].smooth);
        if (RC_Channel[i].smooth == true) PrintSpaces(9); else PrintSpaces(10);
        Serial.println(printRadioState(RC_Channel[i].state));
    }
   
    Serial.println();
    Serial.println();

    DumpLightSchemeToSerial(CurrentScheme);

    Serial.println();
    Serial.println();
}

// Show each setting for each state for each light in tabular format out the serial port
void DumpLightSchemeToSerial(int WhatScheme)
{
    uint8_t i;
    uint8_t j;
    uint8_t whatSetting;
    uint8_t padding;
    
    if (WhatScheme <= NumSchemes)
    {   // Schemes are zero-based, so if they pass Scheme 1 what we really want to show is Scheme 0
        WhatScheme -= 1;
        Serial.print(F("Active Scheme: "));
        Serial.print(WhatScheme+1);
        Serial.println();
        PrintLine(80);
        Serial.print(F("Light   ")); 
        Serial.print(F("Pos 1           Pos 2           Pos 3           Pos 4           Pos 5           "));
        PerLoopUpdates();
        Serial.print(F("Forward         Reverse         Stop            StopDelay       Brake           "));
        PerLoopUpdates();
        Serial.print(F("Right Turn      Left Turn       No Turn         Accelerating    Decelerating    "));
        PerLoopUpdates();
        Serial.println();
        // If we try to print the full line all at once, we will lose track of the radio. 
        // This is kind of silly but we break it up. Would be better to program the ability to temporarily suspend radio
        // reads without changing RC_State but I'm too lazy.
        PrintLineWOLineBreak(50);
        PerLoopUpdates();
        PrintLineWOLineBreak(50);
        PerLoopUpdates();
        PrintLineWOLineBreak(50);
        PerLoopUpdates();
        PrintLineWOLineBreak(50);
        PerLoopUpdates();
        PrintLine(44);      // We want 244 dashes
        for (i=0; i<NumLights; i++)
        {
            PerLoopUpdates();
            Serial.print(F(" "));
            Serial.print(i+1);
            Serial.print(F("      "));
            for (j=0; j<NumStates; j++)
            {
                PerLoopUpdates();
                whatSetting = pgm_read_word_near(&(Schemes[WhatScheme][i][j]));
                padding = pgm_read_word_near(&(_SettingNamesPadding[whatSetting]));
                // Serial.print(whatSetting,DEC);
                // Serial.print(padding, DEC);
                Serial.print(ptrLightSettingCap(whatSetting));
                PrintSpaces(padding);
            }
            Serial.println();
        }
    }
    return;
}

void PrintChannelName(uint8_t channel, boolean pad)
{
    switch (channel)
    {
        case 0:     { Serial.print(F("Throttle"));  if (pad) PrintSpaces(6); } break;
        case 1:     { Serial.print(F("Steering"));  if (pad) PrintSpaces(6); } break;
        case 2:     { Serial.print(F("Channel 3"));  if (pad) PrintSpaces(5); } break;
        default:    { Serial.print(F("Unknown"));  if (pad) PrintSpaces(7); } break;
    }
}

void PrintLightSetting(uint8_t light, uint8_t setting)
{
    Serial.print(F("Light "));
    Serial.print(light + 1);
    Serial.print(F(": ")); 
    Serial.println(ptrLightSetting(setting));
}

void PrintSpaceDash()
{
    Serial.print(F(" - "));
}

void PrintSpace()
{
    Serial.print(F(" "));
}    

void PrintSpaces(uint8_t spaces)
{
    for (uint8_t i=0; i<spaces; i++)
    {
        PrintSpace();
    }
}


void PrintLineWOLineBreak(uint8_t len)
{
    for (uint8_t i=0; i<len; i++)
    {
        Serial.print(F("-"));
    }    
}

void PrintLine(uint8_t len)
{
    PrintLineWOLineBreak(len);
    Serial.println();
}

void PrintTrueFalse(boolean boolVal)
{
    if (boolVal == true) { Serial.print(F("True")); } else { Serial.print(F("False")); }
}

void PrintLineTrueFalse(boolean boolVal)
{
    PrintTrueFalse(boolVal);
    Serial.println();
}

void PrintYesNo(boolean boolVal)
{
    if (boolVal == true) { Serial.print(F("Yes")); } else { Serial.print(F("No")); }
}

void PrintLineYesNo(boolean boolVal)
{
    PrintYesNo(boolVal);
    Serial.println();
}
