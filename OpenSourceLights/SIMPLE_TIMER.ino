// FUNCTIONS RELATED TO THE SIMPLE TIMER 


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// BLINKLIGHTS - This flip/flops our blinker variable
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void BlinkLights()
{
    Blinker = !Blinker;
}



// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// FASTBLINKLIGHTS - This flip/flops our fast blinker variable
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
void FastBlinkLights()
{
    FastBlinker = !FastBlinker;
}


// ------------------------------------------------------------------------------------------------------------------------------------------------>  
// STARTBLINKING_MS - Used by the Radio_Setup routines, this will blink a given LED a certain number of times and then stop
// ------------------------------------------------------------------------------------------------------------------------------------------------>  
unsigned int StartBlinking_ms(int LED, int BlinkTimes, int ms)
{
    switch (BlinkTimes)
    {
        case 1:
            if (LED == GreenLED) { return timer.setInterval(ms, GreenBlinkOne); }
            if (LED == RedLED)   { return timer.setInterval(ms, RedBlinkOne);   }
            break;
        case 2:
            if (LED == GreenLED) { return timer.setInterval(ms, GreenBlinkTwo); }
            if (LED == RedLED)   { return timer.setInterval(ms, RedBlinkTwo);   }
            break;
        case 3:
            if (LED == GreenLED) { return timer.setInterval(ms, GreenBlinkThree); }
            if (LED == RedLED)   { return timer.setInterval(ms, RedBlinkThree);   }
            break;
        default:
            break;        
    }
}



void StopBlinking(unsigned int TimerID)
{
    timer.deleteTimer(TimerID);
}



unsigned int StartWaiting_mS(int mS)
{
    TimeUp = false;
    return timer.setTimeout(mS, SetTimeUp);    // will call function once after ms duration
}



unsigned int StartWaiting_sec(int seconds)
{
    return StartWaiting_mS(seconds*1000);
}



void SetTimeUp()
{
    TimeUp = true;
}


void ExitChangeSchemeMode()
{
    ChangeSchemeMode = false;    // As soon as this gets set to false, Change-Scheme-Mode exits
}


