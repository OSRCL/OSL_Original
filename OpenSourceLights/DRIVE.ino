
int ReturnDriveMode(int ThrottleCMD)
{
    if (ThrottleCMD       >  ThrottleDeadband) { return FWD;  }
    else if (ThrottleCMD  < -ThrottleDeadband) { return REV;  }
    else                                       { return STOP; }
}




