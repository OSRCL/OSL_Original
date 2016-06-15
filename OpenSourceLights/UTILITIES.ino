

void PrintSpaceDash()
{
    Serial.print(F(" - "));
}

void PrintSpace()
{
    Serial.print(F("  "));
}    

void PrintHorizontalLine()
{
    Serial.println(F("-----------------------------------"));
}

void PrintTrueFalse(boolean boolVal)
{
    if (boolVal == true) { Serial.println(F("TRUE")); } else { Serial.println(F("FALSE")); }
}
