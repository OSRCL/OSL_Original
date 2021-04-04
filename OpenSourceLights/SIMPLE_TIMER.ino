
// FUNCTIONS RELATED TO THE SIMPLE TIMER 

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



