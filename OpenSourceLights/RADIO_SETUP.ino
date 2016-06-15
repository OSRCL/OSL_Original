void RadioSetup()
{
int Count;
unsigned long TotThrottlePulse = 0;
unsigned long TotTurnPulse = 0;
unsigned long TotChannel3Pulse = 0;
float TempFloat;
boolean RunSetup = false;
int TypicalPulseCenter = 1500;
int MaxPossiblePulse = 2250;
int MinPossiblePulse = 750;

int GreenBlinker;                        // We'll use this to create a SimpleTimer for the green led, that flashes to represent which stage we are in.     
    
       
// RUN SETUP
// -------------------------------------------------------------------------------------------------------------------------------------------------->        
    Serial.println(F("ENTERING SETUP...")); 
    Serial.println();

    // While in setup, Red LED remains on:
        RedLedOn();

    // STAGE 1 = Read max travel values from radio, save to EEPROM
    // ------------------------------------------------------------------------------------------------------------------------------------------>                  
        // Transition to Stage 1:
        // Green LED on steady for two seconds
        delay(2000);
        Serial.println(F("STAGE 1 - STORE MAX TRAVEL VALUES"));
        PrintHorizontalLine();
        Serial.println(F("Move all controls to maximum values"));
        Serial.println(F("while green LED blinks"));
        Serial.println();
        GreenLedOn();
        delay(2000);
        GreenLedOff();
        delay(2000);

        // Start green LED blinking for stage one: one blink every 1200 ms
        GreenBlinker = StartBlinking_ms(GreenLED, 1, 1200);    // Blip the GreenLED once every 1200ms
        StartWaiting_sec(15);
        Serial.println(F("Reading..."));

        // We initialize every min and max value to TypicalPulseCenter. In the loop below we will record deviations from the center. 
        ThrottlePulseMin = TypicalPulseCenter;
        ThrottlePulseMax = TypicalPulseCenter;
        TurnPulseMin     = TypicalPulseCenter;
        TurnPulseMax     = TypicalPulseCenter;
        Channel3PulseMin = TypicalPulseCenter;
        Channel3PulseMax = TypicalPulseCenter;

        // Repeat until StartWaiting timer is up
        do
        {
            // Read channel while the user moves the sticks to the extremes
            ThrottlePulse = pulseIn(ThrottleChannel_Pin, HIGH, ServoTimeout);  
            TurnPulse = pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout);
            Channel3Pulse = pulseIn(Channel3_Pin, HIGH, ServoTimeout);
            // Each time through the loop, only save the extreme values if they are greater than the last time through the loop. 
            // At the end we should have the true min and max for each channel.
            if (ThrottlePulse > ThrottlePulseMax)  { ThrottlePulseMax   = ThrottlePulse; }
            if (TurnPulse     > TurnPulseMax)      { TurnPulseMax       = TurnPulse;     }
            if (Channel3Pulse > Channel3PulseMax)  { Channel3PulseMax   = Channel3Pulse; }
            // However we don't save a min pulse if it is equal to zero, because that is not valid. 
            if (ThrottlePulse > 0 && ThrottlePulse < ThrottlePulseMin)  { ThrottlePulseMin   = ThrottlePulse; }
            if (TurnPulse     > 0 && TurnPulse     < TurnPulseMin    )  { TurnPulseMin       = TurnPulse;     }
            if (Channel3Pulse > 0 && Channel3Pulse < Channel3PulseMin)  { Channel3PulseMin   = Channel3Pulse; }
            
            // Refresh the timer
            timer.run();
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);
        
        // Sanity check in case something weird happened (like Tx turned off during setup, or some channels disconnected)
        if (ThrottlePulseMin < MinPossiblePulse)  {ThrottlePulseMin = MinPossiblePulse;}
        if (TurnPulseMin < MinPossiblePulse)      {TurnPulseMin = MinPossiblePulse;}
        if (Channel3PulseMin < MinPossiblePulse)  {Channel3PulseMin = MinPossiblePulse;}                
        if (ThrottlePulseMax > MaxPossiblePulse)  {ThrottlePulseMax = MaxPossiblePulse;}
        if (TurnPulseMax > MaxPossiblePulse)      {TurnPulseMax = MaxPossiblePulse;}
        if (Channel3PulseMax > MaxPossiblePulse)  {Channel3PulseMax = MaxPossiblePulse;}                

        // Save values to EEPROM
        eeprom_write(ThrottlePulseMin, E_ThrottlePulseMin);
        eeprom_write(ThrottlePulseMax, E_ThrottlePulseMax);
        eeprom_write(TurnPulseMin, E_TurnPulseMin);
        eeprom_write(TurnPulseMax, E_TurnPulseMax);
        eeprom_write(Channel3PulseMin, E_Channel3PulseMin);
        eeprom_write(Channel3PulseMax, E_Channel3PulseMax);

        Serial.println();
        Serial.println(F("Stage 1 Results: Min - Max pulse values"));
        Serial.print(F("Throttle: "));
        Serial.print(ThrottlePulseMin);
        PrintSpaceDash();
        Serial.println(ThrottlePulseMax);
        Serial.print(F("Turn: "));
        Serial.print(TurnPulseMin);
        PrintSpaceDash();
        Serial.println(TurnPulseMax);
        Serial.print(F("Ch3: "));
        Serial.print(Channel3PulseMin);
        PrintSpaceDash();
        Serial.println(Channel3PulseMax);

        Serial.println();
        Serial.println(F("STAGE 1 COMPLETE"));
        Serial.println();
        Serial.println();
        
     
    // Stage 2 = Set Channel center values (I've found they are not always equal to one half of max travel)
    // For Channel 3, if you have a 3-position switch, set it to center. If you have a 2 position switch, set it to ON
    // This routine will determine whether a 3 position switch is implemented or not. 
    // ------------------------------------------------------------------------------------------------------------------------------------------>       
        // Transition to Stage 2:
        // Off for two seconds, two slow blinks
        GreenLedOff();
        delay(2000);        
        Serial.println(F("STAGE 2 - STORE CENTER VALUES"));
        PrintHorizontalLine();
        Serial.println(F("Place throttle and steering in NEUTRAL."));
        Serial.println(F("If Channel 3 is a 3-position switch, set it to CENTER."));
        Serial.println();
        GreenBlinkSlow(2);
        delay(2000);

        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;    
        TotChannel3Pulse = 0;       
        Count = 0;

        // Start green LED blinking for stage two: two blinks every 1200 ms
        GreenBlinker = StartBlinking_ms(GreenLED, 2, 1200);    // Blip the GreenLED twice every 1200ms
        StartWaiting_sec(6); // For the first bit of time we don't take any readings, this lets the user get the sticks centered
        Serial.println(F("Reading..."));
        do
        {
            delay(100);
            timer.run();
        }        
        while (!TimeUp);
        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {        
            TotThrottlePulse += pulseIn(ThrottleChannel_Pin, HIGH, ServoTimeout);  
            TotTurnPulse += pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout);
            TotChannel3Pulse += pulseIn(Channel3_Pin, HIGH, ServoTimeout);
            // Increment reading count
            Count++;
            delay(10);
            // Refresh the timer
            timer.run();
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);

        // Finally we record our readings
        TempFloat = (float)TotThrottlePulse / (float)Count;
        ThrottlePulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotTurnPulse / (float)Count;
        TurnPulseCenter = (int)lround(TempFloat);
        TempFloat = (float)TotChannel3Pulse / (float)Count;
        Channel3PulseCenter = (int)lround(TempFloat);

        // Sanity check in case something weird happened (like Tx turned off during setup, or some channels disconnected)
        if ((ThrottlePulseCenter < MinPossiblePulse) || (ThrottlePulseCenter > MaxPossiblePulse))   {ThrottlePulseCenter = TypicalPulseCenter; }
        if ((TurnPulseCenter < MinPossiblePulse) || (TurnPulseCenter > MaxPossiblePulse))           {TurnPulseCenter = TypicalPulseCenter;     }
        if ((Channel3PulseCenter < MinPossiblePulse) || (Channel3PulseCenter > MaxPossiblePulse))   {Channel3PulseCenter = TypicalPulseCenter; }                

        // Save values to EEPROM
        eeprom_write(ThrottlePulseCenter, E_ThrottlePulseCenter);
        eeprom_write(TurnPulseCenter, E_TurnPulseCenter);
        eeprom_write(Channel3PulseCenter, E_Channel3PulseCenter);

        Serial.println();
        Serial.println(F("Stage 2 Results - Pulse center values"));
        Serial.print(F("Throttle: "));
        Serial.println(ThrottlePulseCenter);
        Serial.print(F("Turn: "));
        Serial.println(TurnPulseCenter);
        Serial.print(F("Ch3: "));
        Serial.println(Channel3PulseCenter);

        Serial.println();
        Serial.println(F("STAGE 2 COMPLETE"));
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
        GreenLedOff();
        delay(2000);            
        Serial.println(F("STAGE 3 - STORE CHANNEL DIRECTIONS"));
        PrintHorizontalLine();
        Serial.println(F("Hold trigger down (full forward), hold steering wheel full right, set Channel 3 to ON"));
        Serial.println();
        GreenBlinkSlow(3);
        delay(2000);                
        
        // Initialize some variables
        TotThrottlePulse = 0;
        TotTurnPulse = 0;                        
        TotChannel3Pulse = 0;                

        Count = 0;
        
        ThrottleChannelReverse = false;
        TurnChannelReverse = false;
        Channel3Reverse = false;

        // Start green LED blinking for stage three: three blinks every 1200 ms
        GreenBlinker = StartBlinking_ms(GreenLED, 3, 1200);    // Blip the GreenLED three times every 1200ms
        StartWaiting_sec(6); // For the first bit of time we don't take any readings, this lets the user get the sticks centered
        Serial.println(F("Reading..."));
        while (!TimeUp)
        {    timer.run(); }
        
        StartWaiting_sec(4); // Now for the next four seconds we check the sticks
        do
        {    
            // Add to our readings 
            TotThrottlePulse += pulseIn(ThrottleChannel_Pin, HIGH, ServoTimeout);  
            TotTurnPulse += pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout);  
            TotChannel3Pulse += pulseIn(Channel3_Pin, HIGH, ServoTimeout);  
            // Increment count
            Count++;
            delay(50);
           // Refresh the timer
            timer.run();
        }
        while (!TimeUp);    // Keep looping until time's up
        StopBlinking(GreenBlinker);

        // Get the average of our readings
        TotThrottlePulse /= Count;
        TotTurnPulse /= Count;
        TotChannel3Pulse /= Count;

        if (DEBUG) 
        {                        
            Serial.print(F("Throttle Avg: "));
            Serial.println(TotThrottlePulse);
            Serial.print(F("Turn Avg: "));
            Serial.println(TotTurnPulse);
            Serial.print(F("Channel3 Avg: "));
            Serial.println(TotChannel3Pulse);
        }
        
        // Decide if the channel is reversed, and do a sanity check as well
        // Throttle stick was held up, should have been long pulse. If not, reverse
        if ((TotThrottlePulse < 1300) && (TotThrottlePulse > MinPossiblePulse))  { ThrottleChannelReverse = true; }
        // Turn stick was held right, should have been long pulse. If not, reverse
        if ((TotTurnPulse < 1300) && (TotTurnPulse > MinPossiblePulse))          { TurnChannelReverse = true;     }
        // We consider on to be high, should have been long pulse. If not, reverse
        if ((TotChannel3Pulse < 1300) && (TotChannel3Pulse > MinPossiblePulse))  { Channel3Reverse = true;        }
        
       // Save values to EEPROM
        eeprom_write(ThrottleChannelReverse, E_ThrottleChannelReverse);
        eeprom_write(TurnChannelReverse, E_TurnChannelReverse);
        eeprom_write(Channel3Reverse, E_Channel3Reverse);     

        Serial.println();
        Serial.println(F("Stage 3 Results - Channel reversed"));
        Serial.print(F("Throttle: "));
        PrintTrueFalse(ThrottleChannelReverse);
        Serial.print(F("Turn: "));
        PrintTrueFalse(TurnChannelReverse);
        Serial.print(F("Ch3: "));
        PrintTrueFalse(Channel3Reverse);

        Serial.println();
        Serial.println(F("STAGE 3 COMPLETE"));
        Serial.println();      
        Serial.println();


        // Transition to Stage End:
        GreenLedOff();
        delay(2000);
        GreenLedOn();
        delay(3000);
        GreenLedOff();

    // End Setup
    Serial.println(F("--- END SETUP ---")); 
    Serial.println();
    RedLedOff();       

}


