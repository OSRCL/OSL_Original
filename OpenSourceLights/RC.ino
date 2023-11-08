
// -------------------------------------------------------------------------------------------------------------------------------------------------->
// RC INPUTS
// -------------------------------------------------------------------------------------------------------------------------------------------------->

void InitializeRCChannels(void)
{

    // Assign pins
    if (HardwareVersion == 1)
    {
        RC_Channel[0].pin = pin_HW1_Throttle;
        RC_Channel[1].pin = pin_HW1_Steering;
        RC_Channel[2].pin = pin_HW1_Ch3;
    }
    else if (HardwareVersion == 2)
    {
        RC_Channel[0].pin = pin_HW2_Throttle;
        RC_Channel[1].pin = pin_HW2_Steering;
        RC_Channel[2].pin = pin_HW2_Ch3;
    }
    
    // Settings common to all channels
    for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
    {
        pinMode(RC_Channel[i].pin, INPUT_PULLUP);
        RC_Channel[i].state = RC_SIGNAL_ACQUIRE;
        RC_Channel[i].rawPulseWidth = 1500;
        RC_Channel[i].pulse = 1500;
        RC_Channel[i].readyForUpdate = false;
        RC_Channel[i].updated = false;
        RC_Channel[i].mappedCommand = 0;            // For throttle and steering channels, initialize to 0
        RC_Channel[i].lastEdgeTime = 0;
        RC_Channel[i].lastGoodPulseTime = 0;
        RC_Channel[i].acquireCount = 0;
        RC_Channel[i].numSwitchPos = 5;             // We can read up to a 5 position switch
        RC_Channel[i].switchPos = Pos1;             // For switch channels, start in the first position
    }

    // Settings distinct for individual channels
    // Throttle
    RC_Channel[0].channel = 0;                      // Throttle
    RC_Channel[0].Digital = false;                  
    RC_Channel[0].deadband = ThrottleDeadband;
    RC_Channel[0].smooth = SmoothThrottle; 
    RC_Channel[0].smoothedValue = 1500;
    eeprom_read(RC_Channel[0].pulseMin, E_ThrottlePulseMin);
    eeprom_read(RC_Channel[0].pulseMax, E_ThrottlePulseMax);
    eeprom_read(RC_Channel[0].pulseCenter, E_ThrottlePulseCenter);
    eeprom_read(RC_Channel[0].reversed, E_ThrottleChannelReverse);    
    // Steering
    RC_Channel[1].channel = 1;                      // Steering
    RC_Channel[1].Digital = false;                  
    RC_Channel[1].deadband = TurnDeadband;
    RC_Channel[1].smooth = SmoothSteering;
    RC_Channel[1].smoothedValue = 1500;
    eeprom_read(RC_Channel[1].pulseMin, E_TurnPulseMin);    
    eeprom_read(RC_Channel[1].pulseMax, E_TurnPulseMax);    
    eeprom_read(RC_Channel[1].pulseCenter, E_TurnPulseCenter);
    eeprom_read(RC_Channel[1].reversed, E_TurnChannelReverse); 
    // Channel 3
    RC_Channel[2].channel = 2;                      // Channel 3
    RC_Channel[2].Digital = true;                   // Channel 3 is treated as a switch
    RC_Channel[2].deadband = 0;
    RC_Channel[2].smooth = SmoothChannel3;
    RC_Channel[2].smoothedValue = 0;
    RC_Channel[2].switchPos = Pos1;                 // Default to Position 1, which is the default position when no Channel 3 is attached
    RC_Channel[2].rawPulseWidth = 1000;             // Let's set the default pulse width to the equivalent of Pos1 just so it all matches
    RC_Channel[2].pulse = 1000;                     // idem
    eeprom_read(RC_Channel[2].pulseMin, E_Channel3PulseMin);    
    eeprom_read(RC_Channel[2].pulseMax, E_Channel3PulseMax);    
    eeprom_read(RC_Channel[2].pulseCenter, E_Channel3PulseCenter);
    eeprom_read(RC_Channel[2].reversed, E_Channel3Reverse); 
        
    
    // Now link some values from this array to discrete variables for ease of reference in code
    ThrottleCommand = RC_Channel[0].mappedCommand;
    TurnCommand = RC_Channel[1].mappedCommand;
    Channel3Command = RC_Channel[2].switchPos;
}

void EnableRCInterrupts(void)
{   // Pin change interrupts
    attachPCINT(digitalPinToPCINT(RC_Channel[0].pin), RC0_ISR, CHANGE);
    attachPCINT(digitalPinToPCINT(RC_Channel[1].pin), RC1_ISR, CHANGE);
    attachPCINT(digitalPinToPCINT(RC_Channel[2].pin), RC2_ISR, CHANGE);
}

void DisableRCInterrupts(void)
{
    detachPCINT(digitalPinToPCINT(RC_Channel[0].pin));
    detachPCINT(digitalPinToPCINT(RC_Channel[1].pin));
    detachPCINT(digitalPinToPCINT(RC_Channel[2].pin));
}

void RC0_ISR()
{
    ProcessRCPulse(0);
}

void RC1_ISR()
{
    ProcessRCPulse(1);
}

void RC2_ISR()
{
    ProcessRCPulse(2);
}

void ProcessRCPulse(uint8_t ch)
{
    uint32_t    uS;
    uS =  micros();    

    // If an input voltage on one of the RC pins has changed, an interrupt is automaically generated and we end up here. 
    // We want to measure the length of a pulse, starting at the rising edge and ending at the falling edge. 
    // When a falling edge is detected we record the length of time and set a flag so that when the  main loop calls ProcessChannelPulses()
    // we can decide what to do with it. We could have processed and acted upon the pulse here but best practice is to keep time within ISRs as
    // brief as possible, so we do the bare minimum and let the loop handle the rest outside of the ISR
    if (RC_Channel[ch].readyForUpdate == false)                 // Don't bother if we haven't yet processed the last pulse. 
    {
        if (digitalRead(RC_Channel[ch].pin))
        {   
            RC_Channel[ch].lastEdgeTime = uS;                   // Rising edge - save the time
        }
        else
        {   // Falling edge - completed pulse received
            RC_Channel[ch].rawPulseWidth = (uS - RC_Channel[ch].lastEdgeTime);   // Save the pulse width, but we dont know yet if it's valid
            RC_Channel[ch].lastEdgeTime = uS;                   // Save the time
            RC_Channel[ch].readyForUpdate = true;               // Flag so the loop knows to complete the process of validation/action, but we don't do it here in order that we keep our time in the ISR to a minimum. 
        }
    }
}

void ProcessChannelPulses(void)
{
    // Here we check for a channel with a readyForUpdate flag set, meaning a pulse has been measured. We determine whether this pulse is valid 
    // and if so we act upon it, if not we change the state of this channel to RC_SIGNAL_LOST

    for (uint8_t ch=0; ch<NUM_RC_CHANNELS; ch++)
    {
        if (RC_Channel[ch].readyForUpdate)
        {
            if (RC_Channel[ch].rawPulseWidth >= PULSE_WIDTH_ABS_MIN && RC_Channel[ch].rawPulseWidth <= PULSE_WIDTH_ABS_MAX)
            {
                // rawPulseWidth is valid, transfer it to actual pulse variable
                RC_Channel[ch].pulse = RC_Channel[ch].rawPulseWidth;

                // Appply smoothing if specified on this channel
                if (RC_Channel[ch].smooth) 
                {
                    // Smoothing code submitted by Wombii 
                    // https://www.rcgroups.com/forums/showthread.php?1539753-Open-Source-Lights-Arduino-based-RC-Light-Controller/page57#post41145245
                    // Takes difference between current and old value, divides difference by none/2/4/8/16 and adds difference to old value (a quick and simple way of averaging)
                    RC_Channel[ch].smoothedValue = RC_Channel[ch].smoothedValue + ((RC_Channel[ch].pulse - RC_Channel[ch].smoothedValue) >> smoothingStrength);
                    RC_Channel[ch].pulse = RC_Channel[ch].smoothedValue;
                }
                
                RC_Channel[ch].lastGoodPulseTime = RC_Channel[ch].lastEdgeTime;
                // Update the channel's state if needed 
                switch (RC_Channel[ch].state)
                {
                    case RC_SIGNAL_SYNCHED:
                        // Do something with the pulse
                        ProcessRCCommand(RC_Channel[ch]);
                        break;
                            
                    case RC_SIGNAL_LOST:
                        RC_Channel[ch].state = RC_SIGNAL_ACQUIRE;
                        RC_Channel[ch].acquireCount = 1;
                        break;
                    
                    case RC_SIGNAL_ACQUIRE:
                        if (++RC_Channel[ch].acquireCount >= RC_PULSECOUNT_TO_ACQUIRE)
                        {
                            RC_Channel[ch].state = RC_SIGNAL_SYNCHED;
                            // if (DEBUG) Serial.print(F("Channel ")); Serial.print(ch+1); Serial.println(F(" acquired")); 
                        }
                        break;
                }            
            }
            else 
            {
                // Invalid pulse. If we haven't had a good pulse for a while, set the state of this channel to SIGNAL_LOST. 
                if (RC_Channel[ch].lastEdgeTime - RC_Channel[ch].lastGoodPulseTime > RC_TIMEOUT_US)
                {
                    RC_Channel[ch].state = RC_SIGNAL_LOST;
                    RC_Channel[ch].acquireCount = 0;
                }            
            }
            
            // Clear the update flag since we are done processing this pulse
            RC_Channel[ch].readyForUpdate = false;      
       
            // We know what the individual channel's state is, but let's combine all channel's states into a single 'RC state' 
            // If all channels share the same state, then that is also the state of the overall RC system
            uint8_t countSame = 1;
            boolean anySynched = false;
            uint8_t i;
            for (i=1; i<NUM_RC_CHANNELS; i++)
            {
                if (RC_Channel[i-1].state == RC_Channel[i].state) countSame++;
                if (RC_Channel[i-1].state == RC_SIGNAL_SYNCHED) anySynched = true;
            }
            if (RC_Channel[i-1].state == RC_SIGNAL_SYNCHED) anySynched = true;
            
            // After that, if countSame = NUM_RC_CHANNELS then all states are the same
            if (countSame == NUM_RC_CHANNELS)   RC_State = RC_Channel[0].state;
            else
            {   // In this case, the channels have different states, so we need to condense. 
                // If even one channel is synched, we consider the system synched. 
                // Any other combination we consider signal lost. 
                anySynched ? RC_State = RC_SIGNAL_SYNCHED : RC_State = RC_SIGNAL_LOST;
            }
        
            // Has RC_State changed?
            if (RC_State != Last_RC_State)  ChangeRCState();    
        }
    }
}

void CheckRCStatus(void)
{
    uint32_t        uS;                         // Temp variable to hold the current time in microseconds                        
    static uint32_t TimeLastRCCheck = 0;        // Time we last did a watchdog check on the RC signal
    uint8_t         countOverdue = 0;           // How many channels are overdue (disconnected)

    // The RC pin change ISRs will try to determine the status of each channel, but of course if a channel becomes disconnected the ISR won't even trigger. 
    // So we have the main loop poll this function to do an overt check once every so often (RC_TIMEOUT_MS) 
    if (millis() - TimeLastRCCheck > RC_TIMEOUT_MS)
    {
        TimeLastRCCheck = millis();
        uS = micros();                          // Current time
        cli();                                  // We need to disable interrupts for this check, otherwise value of (uS - LastGoodPulseTime) could return very big number if channel updates in the middle of the check
            for (uint8_t i=0; i<NUM_RC_CHANNELS; i++)
            {
                if ((uS - RC_Channel[i].lastGoodPulseTime) > RC_TIMEOUT_US) 
                {
                    countOverdue += 1;
                    // If this channel had previously been synched, set it now to lost
                    if (RC_Channel[i].state == RC_SIGNAL_SYNCHED)
                    {
                        RC_Channel[i].state = RC_SIGNAL_LOST;
                        RC_Channel[i].acquireCount = 0;
                        // if (DEBUG) Serial.print(F("Channel ")); Serial.print(i+1); Serial.println(F(" lost!")); 
                    }
                }
            }
        sei();                                  // Resume interrupts
        
        if (countOverdue == NUM_RC_CHANNELS)
        {   
            RC_State = RC_SIGNAL_LOST;          // Ok, we've lost radio on all channels
        }

        // If state has changed, update the LEDs
        if (RC_State != Last_RC_State) ChangeRCState();
    }
}

void ChangeRCState(void)
{
    switch (RC_State)
    {
        case RC_SIGNAL_SYNCHED:
            if (Failsafe)
            {
                Failsafe = false;
                StopFailsafeLights();
            }
            break;
                
        case RC_SIGNAL_LOST:
            if (!Failsafe)    
            {
                Failsafe = true;
                StartFailsafeLights();
            }
            break;
        
        case RC_SIGNAL_ACQUIRE:
            // do nothing
            break;        
    }

    Last_RC_State = RC_State;

    if (DEBUG) 
    {
        Serial.print(F("Radio state change: "));
        Serial.print(printRadioState(RC_State));
        if (RC_State == RC_SIGNAL_LOST) Serial.print(F("!"));
        Serial.println();
    }
}

void ProcessRCCommand(_rc_channel &ch)
{
    uint8_t pos;
    boolean WasSomething;

    if (ch.Digital)     // This is a switch
    {
        pos = PulseToMultiSwitchPos(ch);                                                            // Calculate switch position
        if (pos != ch.switchPos)                                                                    // Proceed only if switch position has changed
        {
            ch.switchPos = pos;                                                                     // Update switch position
            ch.updated = true;                                                                      // And the updated flag...
        }            
    }
    else    // Variable input 
    {
        // If the last command was zero, this will be false, otherwise true. 
        WasSomething = ch.mappedCommand;

        if      (ch.pulse >= (ch.pulseCenter + ch.deadband))
        {
            if  (ch.reversed) ch.mappedCommand = map(ch.pulse, ch.pulseCenter, ch.pulseMax, 0, COMMAND_MAX_REVERSE);
            else              ch.mappedCommand = map(ch.pulse, ch.pulseCenter, ch.pulseMax, 0, COMMAND_MAX_FORWARD);
        }
        else if (ch.pulse <= (ch.pulseCenter - ch.deadband))
        {
            if  (ch.reversed) ch.mappedCommand = map(ch.pulse, ch.pulseMin, ch.pulseCenter, COMMAND_MAX_FORWARD, 0);
            else              ch.mappedCommand = map(ch.pulse, ch.pulseMin, ch.pulseCenter, COMMAND_MAX_REVERSE, 0);
        }
        else
        {   
            ch.mappedCommand = 0;
            if (!WasSomething) ch.updated = false;  // In this case, it was zero to start with, and is still zero. Even though the pulse might have changed slightly, 
                                                    // the command didn't really update (basically we are still within deadband).
        }
    
        // Keep the command in limits
        if (ch.mappedCommand != 0)
        {   
            ch.mappedCommand = constrain(ch.mappedCommand, COMMAND_MAX_REVERSE, COMMAND_MAX_FORWARD);
        }        
    }

    // Ok great, we've been manipulating the channel with an array variable to save on code bloat, but now we really need to know what is what
    switch (ch.channel)
    {
        case 0:     // Throttle
            ThrottleCommand = ch.mappedCommand;
            break;
        
        case 1:     // Steering      
            TurnCommand = ch.mappedCommand;         
            if      (TurnCommand > 0) Direction = RIGHT_TURN;
            else if (TurnCommand < 0) Direction = LEFT_TURN;
            else                      Direction = NO_TURN;
            break;

        case 2:     // Channel 3
            Channel3Command = ch.switchPos;
            break;
        
        default:
            break;
    }
}

uint8_t PulseToMultiSwitchPos(_rc_channel &ch)
{
    int POS;
    
    if (ch.pulse == 0)
    {   // In this case, there was no signal found
        POS = Pos1;    // Default to position 1
    }
    else 
    {
        // Turn pulse into one of five possible positions
        if (ch.pulse >= ch.pulseMax - 150)
        {    
            POS = Pos5;
        }
        else if ((ch.pulse >  (ch.pulseCenter + 100)) && (ch.pulse < (ch.pulseMax - 150)))
        {
            POS = Pos4;
        }
        else if ((ch.pulse >= (ch.pulseCenter - 100)) && (ch.pulse <= (ch.pulseCenter + 100)))
        {
            POS = Pos3;
        }
        else if ((ch.pulse <  (ch.pulseCenter - 100)) && (ch.pulse > (ch.pulseMin + 150)))
        {
            POS = Pos2;
        }
        else 
        {
            POS = Pos1;
        }

        // Swap positions if channel is reversed.
        if (ch.reversed)
        {
            if      (POS == Pos1) POS = Pos5;
            else if (POS == Pos2) POS = Pos4;
            else if (POS == Pos4) POS = Pos2;
            else if (POS == Pos5) POS = Pos1;
        }
    }
                
    return POS;
}
