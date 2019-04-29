    // ------------------------------------------------------------------------------------------------------------------------------------------------>
// LED FUNCTIONS - For controlling the LEDs on the board
// ------------------------------------------------------------------------------------------------------------------------------------------------>

void RedLedOn()
{
    digitalWrite(RedLED, HIGH);
}
void RedLedOff()
{
    digitalWrite(RedLED, LOW);
}
void GreenLedOn()
{
    digitalWrite(GreenLED, HIGH);
}
void GreenLedOff()
{
    digitalWrite(GreenLED, LOW);
}

void GreenBlink()
{
    static boolean Status; 
    Status ? GreenLedOn() : GreenLedOff();
    Status = !Status;
}

void RedBlink()
{
    static boolean Status; 
    Status ? RedLedOn() : RedLedOff();
    Status = !Status;
}

void ToggleLEDs()
{
    static boolean Status; 
    Status ? RedLedOn() : RedLedOff();
    Status ? GreenLedOff() : GreenLedOn();
    Status = !Status;
}

void ToggleAllLights()
{
    static boolean Status; 

    // Flip flop the Red and Green LEDs
    Status ? RedLedOn() : RedLedOff();
    Status ? GreenLedOff() : GreenLedOn();

    // Blink the eight outputs, but only if the user wants it
    if (BLINK_LIGHTS_RX_LOST)
    {
        // Flip flop every other external light
        for (int j=0; j<NumLights; j += 2)
        {    
            Status ? TurnOnLight(j) : TurnOffLight(j);
            Status ? TurnOffLight(j+1) : TurnOnLight(j+1);
        }
    }
    
    // Flop the flip
    Status = !Status;
}

void ToggleSelectLight(uint8_t NumLight)
{
    static boolean Status; 
    Status ? TurnOnLight(NumLight-1) : TurnOffLight(NumLight-1);
    Status = !Status;
}

void GreenBlinkSlow(int HowMany)
{
    for (int i=1; i<=HowMany; i++)
    {
        GreenLedOn();
        delay(750);
        GreenLedOff();
        if (i < HowMany)
        { delay(500);  }
    }
}

void RedBlinkFast(int HowMany)
{
    for (int i=1; i<=HowMany; i++)
    {
        RedLedOn();
        delay(100);
        RedLedOff();
        if (i < HowMany)
        { delay(90); }
    }
}

void GreenBlinkFast(int HowMany)
{
    for (int i=1; i<=HowMany; i++)
    {
        GreenLedOn();
        delay(100);
        GreenLedOff();
        if (i < HowMany)
        { delay(90); }
    }
}

// If these seem redundant it is because SimpleTimer can only call a void function
void GreenBlinkOne()
{ GreenBlinkFast(1); }

void GreenBlinkTwo()
{ GreenBlinkFast(2); }

void GreenBlinkThree()
{ GreenBlinkFast(3); }

void RedBlinkOne()
{ RedBlinkFast(1); }

void RedBlinkTwo()
{ RedBlinkFast(2); }

void RedBlinkThree()
{ RedBlinkFast(3); }
