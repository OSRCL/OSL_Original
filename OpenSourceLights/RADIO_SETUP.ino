
void RadioSetup()
{
unsigned long TotThrottlePulse = 0;
unsigned long TotTurnPulse = 0;
unsigned long TotChannel3Pulse = 0;
float TempFloat;
int Count;
#define _line_width 40
BlinkStream bs;     // Used for some blinking effects

       
// RUN SETUP
// -------------------------------------------------------------------------------------------------------------------------------------------------->        
    Serial.println();
    Serial.println();
    Serial.println(F("ENTERING SETUP...")); 
    Serial.println();

    // While in setup, Red LED remains on:
        RedLED.on();
        delayWhilePolling(2000);


    // STAGE 1 = Read max travel values from radio, save to EEPROM
    // ------------------------------------------------------------------------------------------------------------------------------------------>                  
        // Transition to Stage 1:
        // Green LED on steady for two seconds
        PrintLine(_line_width);
        Serial.println(F("STAGE 1 - STORE MAX TRAVEL VALUES"));
        PrintLine(_line_width);
        Serial.println(F("Move all controls to maximum values"));
        Serial.println(F("while green LED blinks"));
        Serial.println();
        GreenLED.on();
        delayWhilePolling(2000);
        GreenLED.off();
        delayWhilePolling(2000);

        // Start green LED blinking for stage one: one blink every 1200 ms
        GreenLED.startBlinking(100, 1200);
        StartWaiting_sec(15);
        Serial.println(F("Reading..."));

        // We initialize every min and max value to TypicalPulseCenter. In the loop below we will record deviations from the center. 
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            RC_Channel[i].pulseMin = PULSE_WIDTH_TYP_CENTER;
            RC_Channel[i].pulseMax = PULSE_WIDTH_TYP_CENTER;
        }

        // Repeat until StartWaiting timer is up
        do
        {
            // Read the radio
            PerLoopUpdates();

            // Read channel while the user moves the sticks to the extremes
            // Each time through the loop, only save the extreme values if they are greater than the last time through the loop. 
            // At the end we should have the true min and max for each channel.
            for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
            {
                if (RC_Channel[i].pulse > RC_Channel[i].pulseMax) RC_Channel[i].pulseMax = RC_Channel[i].pulse;
                if (RC_Channel[i].pulse > 0 && RC_Channel[i].pulse < RC_Channel[i].pulseMin) RC_Channel[i].pulseMin = RC_Channel[i].pulse;
            }
        }
        while (!TimeUp);    // Keep looping until time's up
        GreenLED.stopBlinking();
        
        // Sanity check in case something weird happened (like Tx turned off during setup, or some channels disconnected)
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            if (RC_Channel[i].pulseMin < PULSE_WIDTH_ABS_MIN) RC_Channel[i].pulseMin = PULSE_WIDTH_ABS_MIN;
            if (RC_Channel[i].pulseMax > PULSE_WIDTH_ABS_MAX) RC_Channel[i].pulseMax = PULSE_WIDTH_ABS_MAX;
        }

        // Save values to EEPROM
        eeprom_write(RC_Channel[0].pulseMin, E_ThrottlePulseMin);
        eeprom_write(RC_Channel[0].pulseMax, E_ThrottlePulseMax);
        eeprom_write(RC_Channel[1].pulseMin, E_TurnPulseMin);
        eeprom_write(RC_Channel[1].pulseMax, E_TurnPulseMax);
        eeprom_write(RC_Channel[2].pulseMin, E_Channel3PulseMin);
        eeprom_write(RC_Channel[2].pulseMax, E_Channel3PulseMax);

        Serial.println();
        Serial.println(F("Stage 1 Results: Min & Max pulse values"));
        Serial.println(F("Channel       Min       Max"));
        PrintLine(_line_width);
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            PrintChannelName(i, true);  // True for padding after the word
            Serial.print(RC_Channel[i].pulseMin);
            if (RC_Channel[i].pulseMin < 1000) PrintSpaces(7); else PrintSpaces(6);
            Serial.println(RC_Channel[i].pulseMax);
        }
        Serial.println();
        Serial.println();
        Serial.println();
        
     
    // Stage 2 = Set Channel center values (I've found they are not always equal to one half of max travel)
    // For Channel 3, if you have a 3-position switch, set it to center. If you have a 2 position switch, set it to ON
    // This routine will determine whether a 3 position switch is implemented or not. 
    // ------------------------------------------------------------------------------------------------------------------------------------------>       
        // Transition to Stage 2:
        // Off for two seconds, two slow blinks
        GreenLED.off();
        delayWhilePolling(2000);        
        PrintLine(_line_width);
        Serial.println(F("STAGE 2 - STORE CENTER VALUES"));
        PrintLine(_line_width);
        Serial.println(F("Place throttle and steering in NEUTRAL."));
        Serial.println(F("If Channel 3 is a 3-position switch, set it to CENTER."));
        Serial.println();
        GreenLED.Blink(2, 750, 500);    // Blink twice, 750mS on, 500mS off
        delayWhilePolling(2000);

        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;    
        TotChannel3Pulse = 0;       
        Count = 0;

        // Start green LED blinking for stage two: two blinks every 1200 ms
        bs.interval[0] = 100;               // On
        bs.interval[1] = 90;                // Off
        bs.interval[2] = 100;               // On
        bs.interval[3] = 1200;              // Off
        bs.repeat = true;
        GreenLED.StreamBlink(bs, 4);        // 4 steps in the stream

        StartWaiting_sec(6); // For the first bit of time we don't take any readings, this lets the user get the sticks centered
        Serial.println(F("Reading..."));
        while (!TimeUp)
        {    PerLoopUpdates(); }

        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {        
            delayWhilePolling(100);
            TotThrottlePulse += RC_Channel[0].pulse;  
            TotTurnPulse += RC_Channel[1].pulse;
            TotChannel3Pulse += RC_Channel[2].pulse;
            // Increment reading count
            Count++;
        }
        while (!TimeUp);    // Keep looping until time's up
        GreenLED.stopBlinking();

        // Finally we record our readings
        TempFloat = (float)TotThrottlePulse / (float)Count;
        RC_Channel[0].pulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotTurnPulse / (float)Count;
        RC_Channel[1].pulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotChannel3Pulse / (float)Count;
        RC_Channel[2].pulseCenter = (int)lround(TempFloat);

        // Sanity check in case something weird happened (like Tx turned off during setup, or some channels disconnected)
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {        
            if ((RC_Channel[i].pulseCenter < PULSE_WIDTH_TYP_MIN) || (RC_Channel[i].pulseCenter > PULSE_WIDTH_TYP_MAX))   {RC_Channel[i].pulseCenter = PULSE_WIDTH_TYP_CENTER; }
        }

        // Save values to EEPROM
        eeprom_write(RC_Channel[0].pulseCenter, E_ThrottlePulseCenter);
        eeprom_write(RC_Channel[1].pulseCenter, E_TurnPulseCenter);
        eeprom_write(RC_Channel[2].pulseCenter, E_Channel3PulseCenter);

        Serial.println();
        Serial.println(F("Stage 2 Results - Pulse center values"));
        Serial.println(F("Channel       Center"));
        PrintLine(_line_width);
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            PrintChannelName(i, true);  // True for padding after the word
            Serial.println(RC_Channel[i].pulseCenter);
        }
        Serial.println();
        Serial.println();        
        Serial.println();


    // Stage 3 = Set channel reversing. 
    // ------------------------------------------------------------------------------------------------------------------------------------------>                                 
        // Method used here is to ask the user to:
        // Hold throttle stick for full forward, 
        // Hold steering wheel to full right,
        // Move Channel 3 to full ON
        // We take a string of readings and average them. We see if the pulse lengths are long or short, and knowing where the sticks are physically, 
        // allows us to determine if we need to reverse any channels in software. 

        // Transition to Stage 3:
        GreenLED.off();
        delayWhilePolling(2000);            
        PrintLine(_line_width);
        Serial.println(F("STAGE 3 - STORE CHANNEL DIRECTIONS"));
        PrintLine(_line_width);
        Serial.println(F("Hold trigger down (full forward), hold steering wheel full right, set Channel 3 to ON"));
        Serial.println();
        GreenLED.Blink(2, 750, 500);    // Blink three times, 750mS on, 500mS off
        delayWhilePolling(2000);                
        
        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;                        
        TotChannel3Pulse = 0;                
        Count = 0;

        // Clear reverse flag to start
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            RC_Channel[0].reversed = false;
        }

        // Start green LED blinking for stage three: three blinks every 1200 ms
        bs.interval[0] = 100;               // On
        bs.interval[1] = 90;                // Off
        bs.interval[2] = 100;               // On
        bs.interval[3] = 90;                // Off
        bs.interval[4] = 100;               // On
        bs.interval[5] = 1200;              // Off
        bs.repeat = true;
        GreenLED.StreamBlink(bs, 6);        // 6 steps in the stream
        
        StartWaiting_sec(6); // For the first bit of time we don't take any readings, this lets the user get the sticks centered
        Serial.println(F("Reading..."));
        while (!TimeUp)
        {    PerLoopUpdates(); }

        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {    
            delayWhilePolling(100);
            TotThrottlePulse += RC_Channel[0].pulse;  
            TotTurnPulse += RC_Channel[1].pulse;
            TotChannel3Pulse += RC_Channel[2].pulse;
            // Increment reading count
            Count++;
        }
        while (!TimeUp);    // Keep looping until time's up
        GreenLED.stopBlinking();

        // Get the average of our readings
        TotThrottlePulse /= Count;
        TotTurnPulse /= Count;
        TotChannel3Pulse /= Count;

        /*
        if (DEBUG) 
        {                        
            Serial.print(F("Throttle Avg: "));
            Serial.println(TotThrottlePulse);
            Serial.print(F("Turn Avg: "));
            Serial.println(TotTurnPulse);
            Serial.print(F("Channel3 Avg: "));
            Serial.println(TotChannel3Pulse);
        }
        */
        
        // Decide if the channel is reversed, and do a sanity check as well
        // Throttle stick was held up, should have been long pulse. If not, reverse
        if ((TotThrottlePulse < 1300) && (TotThrottlePulse > PULSE_WIDTH_ABS_MIN))  { RC_Channel[0].reversed = true; }
        // Turn stick was held right, should have been long pulse. If not, reverse
        if ((TotTurnPulse < 1300) && (TotTurnPulse > PULSE_WIDTH_ABS_MIN))          { RC_Channel[1].reversed = true;     }
        // We consider on to be high, should have been long pulse. If not, reverse
        if ((TotChannel3Pulse < 1300) && (TotChannel3Pulse > PULSE_WIDTH_ABS_MIN))  { RC_Channel[2].reversed = true;        }
        
       // Save values to EEPROM
        eeprom_write(RC_Channel[0].reversed, E_ThrottleChannelReverse);
        eeprom_write(RC_Channel[1].reversed, E_TurnChannelReverse);
        eeprom_write(RC_Channel[2].reversed, E_Channel3Reverse);     

        Serial.println();
        Serial.println(F("Stage 3 Results - Channel reversed"));
        Serial.println(F("Channel       Reversed"));
        PrintLine(_line_width);
        for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
        {
            PrintChannelName(i, true);  // True for padding after the word
            PrintLineYesNo(RC_Channel[i].reversed);
        }
        Serial.println();
        Serial.println();        
        Serial.println();


        // Transition to Stage End:
        GreenLED.off();
        delayWhilePolling(2000);
        GreenLED.on();
        delayWhilePolling(3000);
        GreenLED.off();

    // End Setup
    Serial.println(F("--- END SETUP ---")); 
    Serial.println();
    Serial.println();
    RedLED.off();       

}



