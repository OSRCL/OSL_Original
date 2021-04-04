
void Initialize_EEPROM() 
{   // If EEPROM has not been used before, we initialize to some sensible, yet conservative, default values.
    // The first time a radio setup is performed, these will be overwritten with actual values, and never referred to again. 
    // Because the radio setup is the first thing a user should do, these in fact should not come into play. 

    eeprom_write(PULSE_WIDTH_TYP_MIN,    E_ThrottlePulseMin);
    eeprom_write(PULSE_WIDTH_TYP_MAX,    E_ThrottlePulseMax);
    eeprom_write(PULSE_WIDTH_TYP_CENTER, E_ThrottlePulseCenter);
    eeprom_write(PULSE_WIDTH_TYP_MIN,    E_TurnPulseMin);
    eeprom_write(PULSE_WIDTH_TYP_MAX,    E_TurnPulseMax);
    eeprom_write(PULSE_WIDTH_TYP_CENTER, E_TurnPulseCenter);
    eeprom_write(PULSE_WIDTH_TYP_MIN,    E_Channel3PulseMin);
    eeprom_write(PULSE_WIDTH_TYP_MAX,    E_Channel3PulseMax);
    eeprom_write(PULSE_WIDTH_TYP_CENTER, E_Channel3PulseCenter);
    
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
{   
    // If EEPROM has been used before, we run this routine to load all our saved values at startup
    eeprom_read(CurrentScheme, E_CurrentScheme);
}

void SaveScheme_To_EEPROM()
{  // Save the current scheme to EEPROM
    eeprom_write(CurrentScheme, E_CurrentScheme);
    return;   
}



