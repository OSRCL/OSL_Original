
// This update makes sure every loop is as fast as possible because we only read one channel per loop - Wombii
// Loop speed gain is up to 2-3x
void GetRxCommands() 
{
    static int LastThrottleCommand;
    static uint8_t failsafeCounter = 0;

    if (Failsafe)
    {
        failsafeCounter++;
        //if (DEBUG) Serial.println(F("RX: Lost packet"));
        while(failsafeCounter > 4)                                  // Don't go into failsafe until we've missed 4 sequential packets
        {
            if (DEBUG) Serial.println(F("RX Disconnected!"));
            ToggleAllLights();                                      // If the receiver isn't connected, pause the program and flash all the lights
            delay(50);
            GetThrottleCommand();                                   // If we can successfully read the throttle channel, we will go out of failsafe
            if (Failsafe == false) failsafeCounter = 0;             // We're out of failsafe, exit this while
        }
    }
    else
    {
        failsafeCounter = 0;
    }    

    byte channelSelector;
    channelSelector = runCount % 4;
    switch (channelSelector)
    {
        case 0:
            ThrottleCommand = GetThrottleCommand();
            break;
        case 1:
            TurnCommand = GetTurnCommand();
            break;
        case 2:
            ThrottleCommand = GetThrottleCommand();
            break;
        case 3:
            Channel3 = GetChannel3Command();
            break;
    }
}


boolean CheckChannel3()
{
    Channel3Pulse = pulseIn(Channel3_Pin, HIGH, ServoTimeout * 2);
    if (Channel3Pulse == 0) { return false; }
    else { return true; }
}


boolean CheckSteeringChannel()
{
    TurnPulse = pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout * 2);
    if (TurnPulse == 0) { return false; }
    else { return true; }
}    


int GetThrottleCommand()
{
    int ThrottleCommand;
    ThrottlePulse = pulseIn(ThrottleChannel_Pin, HIGH, ServoTimeout);  

    if ((ThrottlePulse == 0) || (ThrottlePulse > PulseMax_Bad) || (ThrottlePulse < PulseMin_Bad))
    {   // Timed out waiting for a signal, or measured a bad signal
        // Set Failsafe flag, set Throttle to 0
        Failsafe = true;
        ThrottleCommand = 0;
    }
    else
    {
        Failsafe = false;
        if (ThrottlePulse >= ThrottlePulseCenter + ThrottleDeadband) 
        { 
            if (ThrottleChannelReverse == false)
            {
                // Without channel reversing, we consider PPM pulse values greater than Center to be forward throttle commands. We now map the radio command to our forward throttle range
                ThrottleCommand = map(ThrottlePulse, (ThrottlePulseCenter + ThrottleDeadband), ThrottlePulseMax, ThrottleCenterAdjust, MaxFwdSpeed);
            }
            else
            {
                // With channel reversing, we consider PPM pulse values greater than Center to be reverse throttle commands. We now map the radio command to our reverse throttle range
                ThrottleCommand = map(ThrottlePulse, (ThrottlePulseCenter + ThrottleDeadband), ThrottlePulseMax, -ThrottleCenterAdjust, MaxRevSpeed);
            }
        }
        else if (ThrottlePulse <= (ThrottlePulseCenter - ThrottleDeadband))
        { 
            if (ThrottleChannelReverse == false)
            {
                // Without channel reversing, we consider PPM pulse values less than Center to be reverse throttle commands. We now map the radio command to our reverse throttle range
                ThrottleCommand = map(ThrottlePulse, ThrottlePulseMin, (ThrottlePulseCenter - ThrottleDeadband), MaxRevSpeed, -ThrottleCenterAdjust);
            }
            else
            {
                // With channel reversing, we consider PPM pulse values less than Center to be forward throttle commands. We now map the radio command to our forward throttle range
                ThrottleCommand = map(ThrottlePulse, ThrottlePulseMin, (ThrottlePulseCenter - ThrottleDeadband), MaxFwdSpeed, ThrottleCenterAdjust);
            }
        }
        else
        {   // In this case we are within the ThrottleDeadband setting, so Command is actually Zero (0)
            ThrottleCommand = 0;
        }

        // Average the command if user has this option enabled
        if (SmoothThrottle) ThrottleCommand = smoothThrottleCommand(ThrottleCommand);
        
        return constrain(ThrottleCommand, MaxRevSpeed, MaxFwdSpeed);
        // After all this, ThrottleCommand is now some value from -255 to +255 where negative equals REV and positive equals FWD. (The values can actually be less if top forward or 
        // reverse speeds have been limited by user)
    }    
}


int GetTurnCommand()
{
    int TurnCommand;
    TurnPulse = pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout);

    if ((TurnPulse == 0) || (TurnPulse > PulseMax_Bad) || (TurnPulse < PulseMin_Bad))
    {   // In this case, there was no signal found on the turn channel
        TurnCommand = 0;    // If no TurnPulse, we set Turn to 0 (no turn)
    }
    else
    {
        if (TurnPulse >= TurnPulseCenter + TurnDeadband) 
        { 
            if (TurnChannelReverse == false)
            {
                // Without channel reversing, we consider PPM pulse values greater than Center to be Right Turn commands. We now map the radio command to our Right Turn range
                TurnCommand = map(TurnPulse, (TurnPulseCenter + TurnDeadband), TurnPulseMax, TurnCenterAdjust, MaxRightTurn);
            }
            else
            {
                // With channel reversing, we consider PPM pulse values greater than Center to be Left Turn commands. We now map the radio command to our Left Turn range
                TurnCommand = map(TurnPulse, (TurnPulseCenter + TurnDeadband), TurnPulseMax, -TurnCenterAdjust, MaxLeftTurn);
            }
        }
        else if (TurnPulse <= (TurnPulseCenter - TurnDeadband))
        { 
            if (TurnChannelReverse == false)
            {
                // Without channel reversing, we consider PPM pulse values less than Center to be Left Turn commands. We now map the radio command to our Left Turn range
                TurnCommand = map(TurnPulse, TurnPulseMin, (TurnPulseCenter - TurnDeadband), MaxLeftTurn, -TurnCenterAdjust);
            }
            else
            {
                // With channel reversing, we consider PPM pulse values less than Center to be Right Turn commands. We now map the radio command to our Right Turn range
                TurnCommand = map(TurnPulse, TurnPulseMin, (TurnPulseCenter - TurnDeadband), MaxRightTurn, TurnCenterAdjust);
            }
        }
        else
        {   // In this case we are within the TurnDeadband setting, so Command is actually Zero (0)
            TurnCommand = 0;
        }

        // Average the command if user has this option enabled
        if (SmoothSteering) TurnCommand = smoothSteeringCommand(TurnCommand);
    }

    return constrain(TurnCommand, MaxLeftTurn, MaxRightTurn);
    // After all this, TurnCommand is now some value from -100 to +100 where negative equals LEFT and positive equals RIGHT.
}


int GetChannel3Command()
{
    int Channel3Command;
    Channel3Pulse = pulseIn(Channel3_Pin, HIGH, ServoTimeout);
    
    if (Channel3Pulse == 0)
    {   // In this case, there was no signal found
        Channel3Command = Pos1;    // If no Channel3, we always set the mode to 1
    }
    else 
    {
        // Turn pulse into one of five possible positions
        if (Channel3Pulse >= Channel3PulseMax - 150)
        {    
            Channel3Command = Pos5;
        }
        else if ((Channel3Pulse >  (Channel3PulseCenter + 100)) && (Channel3Pulse < (Channel3PulseMax - 150)))
        {
            Channel3Command = Pos4;
        }
        else if ((Channel3Pulse >= (Channel3PulseCenter - 100)) && (Channel3Pulse <= (Channel3PulseCenter + 100)))
        {
            Channel3Command = Pos3;
        }
        else if ((Channel3Pulse <  (Channel3PulseCenter - 100)) && (Channel3Pulse > (Channel3PulseMin + 150)))
        {
            Channel3Command = Pos2;
        }
        else 
        {
            Channel3Command = Pos1;
        }

        // Swap positions if Channel 3 is reversed. 
        if (Channel3Reverse)
        {
            if      (Channel3Command == Pos1) Channel3Command = Pos5;
            else if (Channel3Command == Pos2) Channel3Command = Pos4;
            else if (Channel3Command == Pos4) Channel3Command = Pos2;
            else if (Channel3Command == Pos5) Channel3Command = Pos1;
        }

        // Average the command if user has this option enabled
        if (SmoothChannel3) Channel3Command = smoothChannel3Command(Channel3Command);

    }
                
    return Channel3Command;
}


// Smoothing code submitted by Wombii 
// https://www.rcgroups.com/forums/showthread.php?1539753-Open-Source-Lights-Arduino-based-RC-Light-Controller/page57#post41145245
// Takes difference between current and old value, divides difference by none/2/4/8/16 and adds difference to old value (a quick and simple way of averaging)

int smoothThrottleCommand(int rawVal)
{
    static int smoothedThrottle = 0;
    smoothedThrottle = smoothedThrottle + ((rawVal - smoothedThrottle) >> smoothingStrength);
    return smoothedThrottle;
}


int smoothSteeringCommand(int rawVal)
{
    static int smoothedSteer = 0;
    smoothedSteer = smoothedSteer + ((rawVal - smoothedSteer) >> smoothingStrength);
    return smoothedSteer;
}


int smoothChannel3Command(int rawVal)
{
    static int smoothedCh3 = 0;
    smoothedCh3 = smoothedCh3 + ((rawVal - smoothedCh3) >> smoothingStrength);
    return smoothedCh3;
}
