int ReturnDriveMode(int ThrottleCMD)
{
    if (ThrottleCMD       >  ThrottleDeadband) { return FWD;  }
    else if (ThrottleCMD  < -ThrottleDeadband) { return REV;  }
    else                                       { return STOP; }
}

boolean ReturnBrakeFlag(int DriveModePrev, int DriveModeCMD) 
{   
    boolean Brake; 
    Brake = false;
    
    // This function basically compares the drive mode we currently exist in (conveniently captured by DriveModePrev variable)
    // with the Commands being received ( conveniently summarized by DriveModeCMD)
    // We then determine if a braking command is being given

    // Change of direction from forwad to reverse
    if (DriveModePrev == FWD && DriveModeCMD == REV)
    {
        Brake = true;
    }
    // Change of direction from reverse to forward
    if (DriveModePrev == REV && DriveModeCMD == FWD)
    {
        Brake = true;
    }

    // If we have DragBrake = true, then the Brake state will also be active anytime the throttle stick is near center. 
    if ((DragBrake == true) && (DriveModePrev != STOP) && (DriveModeCMD != FWD) && (DriveModeCMD != REV))
    {
        Brake = true;
    }

    return Brake;
}    




