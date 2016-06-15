void GetRxCommands()
{
    static int LastThrottleCommand;
    
    // We always check the Throttle channel
    ThrottleCommand = GetThrottleCommand();

//    if (SanityCheck()) { LastThrottleCommand = ThrottleCommand;}
//    else    { ThrottleCommand = LastThrottleCommand; }
    
    while(Failsafe)
    {
        if (DEBUG) Serial.println(F("RX Disconnected!"));
        ToggleAllLights();                                       // If the receiver isn't connected, pause the program and flash all the lights
        delay(50);
        GetThrottleCommand();
    }    

    // But to save time, we only check the Steering and Channel 3 channels 
    // if they were detected at startup. Otherwise we ignore them. 
    if (SteeringChannelPresent) { TurnCommand = GetTurnCommand(); }
    else                        { TurnCommand = 0;                }    // We set Turn to nothing if not being used

    if (Channel3Present)        { Channel3 = GetChannel3Command();}
    else                        { Channel3 = Pos1;                }    // We set Channel 3 to Position 1 if not being used
}

boolean CheckChannel3()
{
    Channel3Pulse = pulseIn(Channel3_Pin, HIGH, ServoTimeout);
    if (Channel3Pulse == 0) { return false; }
    else { return true; }
}

boolean CheckSteeringChannel()
{
    TurnPulse = pulseIn(SteeringChannel_Pin, HIGH, ServoTimeout);
    if (TurnPulse == 0) { return false; }
    else { return true; }
}    


// This probably isn't needed anymore... was tested to filter out glitching
#define GoodFrames 3
boolean SanityCheck()
{
    static byte Dir = STOP;
    static byte LastDir = STOP;
    static int TCount = 0;
    
    if (ThrottleCommand > 0) {Dir = FWD;}
    if (ThrottleCommand < 0) {Dir = REV;}
    else {Dir = STOP;}
    
    if ((Dir != LastDir) && (TCount < GoodFrames)) 
    {   TCount += 1; 
        return false;    
    }
    else 
    {   TCount = 0;
        LastDir = Dir;
        return true;
    }
    
    Serial.print(Dir);
    PrintSpace();
    Serial.print(LastDir);
    PrintSpace();
    Serial.println(TCount);
}


int GetThrottleCommand()
{
    int ThrottleCommand;
//    ThrottlePulse = fir_basic(pulseIn(ThrottleChannel_Pin, HIGH, ServoTimeout));
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
        return constrain(TurnCommand, MaxLeftTurn, MaxRightTurn);
        // After all this, TurnCommand is now some value from -100 to +100 where negative equals LEFT and positive equals RIGHT.
    }
}



int GetChannel3Command()
{
    int Channel3Command;
        Channel3Pulse = pulseIn(Channel3_Pin, HIGH, ServoTimeout);
        if (Channel3Pulse == 0)
        {   // In this case, there was no signal found
            // Channel3Present = false;
            Channel3Command = Pos1;    // If no Channel3, we always set the mode to 1
        }
        else 
        {
            Channel3Present = true;
            
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
                else if (Channel3Command == Pos1) Channel3Command = Pos1;
            }
    
        }
    return Channel3Command;
}


