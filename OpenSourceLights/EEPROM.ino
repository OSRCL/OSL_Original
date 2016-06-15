
void Initialize_EEPROM() 
{   // If EEPROM has not been used before, we initialize to some sensible, yet conservative, default values.
    // The first time a radio setup is performed, these will be overwritten with actual values, and never referred to agian. 
    // Because the  radio setup is the first thing a user should do, these in fact should not come into play. 
    int PulseMin    = 1400;
    int PulseCenter = 1500;
    int PulseMax    = 1600;

    eeprom_write(PulseMin, E_ThrottlePulseMin);
    eeprom_write(PulseMax, E_ThrottlePulseMax);
    eeprom_write(PulseCenter, E_ThrottlePulseCenter);
    eeprom_write(PulseMin, E_TurnPulseMin);
    eeprom_write(PulseMax, E_TurnPulseMax);
    eeprom_write(PulseCenter, E_TurnPulseCenter);
    eeprom_write(PulseMin, E_Channel3PulseMin);
    eeprom_write(PulseMax, E_Channel3PulseMax);
    eeprom_write(PulseCenter, E_Channel3PulseCenter);
    
    eeprom_write(false, E_ThrottleChannelReverse);
    eeprom_write(false, E_TurnChannelReverse);
    eeprom_write(false, E_Channel3Reverse);    

    eeprom_write(1, E_CurrentScheme);    // Default to Scheme #1
    
    // This is our initialization constant
    eeprom_write(EEPROM_Init, E_InitNum);

    // Finally - we still need to set up our variables, so now we call Load_EEPROM
    Load_EEPROM();
}

void Load_EEPROM()
{   // If EEPROM has been used before, we run this routine to load all our saved values at startup
    eeprom_read(ThrottlePulseMin, E_ThrottlePulseMin);
    eeprom_read(ThrottlePulseMax, E_ThrottlePulseMax);
    eeprom_read(ThrottlePulseCenter, E_ThrottlePulseCenter);
    eeprom_read(TurnPulseMin, E_TurnPulseMin);    
    eeprom_read(TurnPulseMax, E_TurnPulseMax);    
    eeprom_read(TurnPulseCenter, E_TurnPulseCenter);
    eeprom_read(Channel3PulseMin, E_Channel3PulseMin);    
    eeprom_read(Channel3PulseMax, E_Channel3PulseMax);    
    eeprom_read(Channel3PulseCenter, E_Channel3PulseCenter);

    eeprom_read(ThrottleChannelReverse, E_ThrottleChannelReverse);    
    eeprom_read(TurnChannelReverse, E_TurnChannelReverse); 
    eeprom_read(Channel3Reverse, E_Channel3Reverse); 
    
    eeprom_read(CurrentScheme, E_CurrentScheme);
}


void SaveScheme_To_EEPROM()
{  // Save the current scheme to EEPROM
    eeprom_write(CurrentScheme, E_CurrentScheme);
    return;   
}

