/* OSL_LedHandler.h     Led Handler - class for handling LEDs, requires use of elapsedMillis
 * Source:              https://github.com/OSRCL
 * Authors:             Luke Middleton
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */  


#include "OSL_LedHandler.h"


void OSL_LedHandler::begin(byte p, boolean i /*=false*/, boolean w /*=false*/)
{
    _pin = p;                   // Save pin number
    _invert = i;                // Save invert status
	_pwmable = w;				// Can we analog-write to this pin (pwm-able)
	_fadeType = FADE_TYPE_EXP;	// Default fade type
    pinMode(_pin, OUTPUT);      // Set pin to OUTPUT
    _ledPriorState = LED_STATE_OFF;
	_ledCurState = LED_STATE_OFF;
	this->off();                // Start with Led off, this also will call clearUpdateProcess()
}

void OSL_LedHandler::clearUpdateProcess()
{
    _curProcessStep = 0;
	_numProcessSteps = 0;
	_curStep = 0;
	_numSteps = 0;    
	_nextWait = 0;
	_fadeToTarget = false;
	_pwmTarget = 0;
}

void OSL_LedHandler::changeLEDState(uint8_t changeState)
{	
	_ledPriorState = _ledCurState;
	_ledCurState = changeState;

	// Xenon ends and the light is full on. But we want to keep the state on Xenon, not On
	if (_ledCurState == LED_STATE_ON    && _ledPriorState == LED_STATE_XENON)	  _ledCurState = LED_STATE_XENON;		// Xenon complete is kept at Xenon instead of On

	// There are other similar checks at the beginning of certain functions, for example, Xenon()
}

void OSL_LedHandler::on(void)
{
	this->clearUpdateProcess();
	this->pinOn();
	changeLEDState(LED_STATE_ON);
	_pwm = MAX_PWM;	
}

boolean OSL_LedHandler::isOn(void)
{
	if (_ledCurState == LED_STATE_ON) return true;
	else						      return false;
}

void OSL_LedHandler::off(void)
{
	offWithExtra(true);
}

void OSL_LedHandler::offWithExtra(boolean includeExtra /* = false */)
{
	if (includeExtra)
	{
		if (_ledCurState == LED_STATE_SOFTBLINK)
		{	// In this case, rather than going directly to off, we'd like to continue the softblink until it naturally reaches off and then let it stop
			// We accomplish this by setting the _fadeToTarget flag, set _pwmTarget to zero, and SoftBlink will stop when it reaches it
			_pwmTarget = 0;
			_fadeToTarget = true;
			// Or we could do this: 
			// FadeTo(level);
			// But taking care of it within softblink ends up looking nicer. 
		}		
		else
		{
			this->clearUpdateProcess();
			changeLEDState(LED_STATE_OFF);	
			
			if (_ledPriorState == LED_STATE_XENON)
			{	// If the prior state was xenon, fade off instead of turning off abruptly
				this->Fade(FADE_OUT, DEFAULT_FADE_TIME, FADE_TYPE_EXP);
			}
			else
			{	// Otherwise we can go directly to off
				this->pinOff();
				_pwm = MIN_PWM;	
			}
		}
	}
	else
	{	// No extra checks, just go off
		this->clearUpdateProcess();
		changeLEDState(LED_STATE_OFF);	

		this->pinOff();
		_pwm = MIN_PWM;		
	}
}

void OSL_LedHandler::pinOn(void)
{
    _invert ? digitalWrite(_pin, LOW) : digitalWrite(_pin, HIGH);
}

void OSL_LedHandler::pinOff(void)
{
    _invert ? digitalWrite(_pin, HIGH) : digitalWrite(_pin, LOW);
}

void OSL_LedHandler::toggle(void)
{
	// This does nothing to the state, it just toggles the pin
	digitalWrite(_pin, !digitalRead(_pin)); 
}
    
void OSL_LedHandler::setPWM(float level)
{
	// Assumed you have already done a check to see if this pin is pwm-able
	_pwm = level;
	analogWrite(_pin, (uint8_t)_pwm);
}
	
void OSL_LedHandler::dim(uint8_t level)
{
	if (_pwmable) 
	{
		if (_ledCurState == LED_STATE_SOFTBLINK)
		{	// In this case, rather than going directly to dim, we'd like to fade to it.
			// We accomplish this by setting a _pwmTarget and SoftBlink will stop when it reaches it
			_pwmTarget = level;
			_fadeToTarget = false;		// Start with this set to false, it will get set to true at the appropriate point in the softblink process
			
			// Or we could do this: 
			// FadeTo(level);
			// But taking care of it within softblink ends up looking nicer. 
		}
		else
		{	// Here we can go directly to dim
			this->clearUpdateProcess();
			changeLEDState(LED_STATE_DIM);
			this->setPWM(level);
		}
	}
	else
	{
		this->clearUpdateProcess();
		if (level > 0)
		{
			this->on();
		}
		else
		{
			this->offWithExtra(false);
		}
	}
}

void OSL_LedHandler::Blink(uint16_t interval /*=DEFAULT_BLINK_INTERVAL*/)
{
    this->Blink(1, interval, 0);
}

void OSL_LedHandler::Blink(uint8_t times, uint16_t interval /*=DEFAULT_BLINK_INTERVAL*/)
{
    clearUpdateProcess();
    _fixedInterval = true;              // Fixed interval means the on and off time are the same
    _nextWait = interval;
    _numSteps = (times * 2) - 1;        // multiply by two and minus one to add spaces between the blinks where the LED is off
	this->pinOn();                      // Start with the Led on. User needs to call the update() function to update the next steps 
    _time = 0;
	changeLEDState(LED_STATE_BLINK);
}

void OSL_LedHandler::Blink(uint8_t times, uint16_t on_interval, uint16_t off_interval /*=DEFAULT_BLINK_INTERVAL*/)
{
    BlinkStream Blinker;    
    Blinker.interval[0] = on_interval;      // On
    Blinker.interval[1] = off_interval;     // Off
    Blinker.repeat = false;					// Non repeating
    this->StreamBlink(Blinker, 2);          // 2 steps
}

void OSL_LedHandler::stopBlinking(void)
{
    this->pinOff();
    clearUpdateProcess();
}

void OSL_LedHandler::startBlinking(uint16_t on_interval, uint16_t off_interval)
{
	BlinkStream Blinker;    
	Blinker.interval[0] = on_interval;      // On
	Blinker.interval[1] = off_interval;     // Off
	Blinker.repeat = true;
	this->StreamBlink(Blinker, 2);          // 2 steps
}

void OSL_LedHandler::StreamBlink(BlinkStream bs, uint8_t numSteps)
{
    clearUpdateProcess();
    _fixedInterval = false;
    _blinkStream = bs;
    _nextWait = _blinkStream.interval[0];
    if (numSteps > MAX_STREAM_STEPS) numSteps = MAX_STREAM_STEPS; 
    _numSteps = numSteps; 
    this->pinOn();                     // Start with the Led on. User needs to call the update() function to update the next steps 
    _time = 0; 
	changeLEDState(LED_STATE_BLINK);
}

// Will fade a LED in or out (use FADE_IN or FADE_OUT for dir)
// Span is in milliseconds and is the length of time the fade will take,
void OSL_LedHandler::Fade(uint8_t dir, uint16_t span, char fadeType)
{
    // If dir = 1 then fade in starting from off, otherwise if dir = 2 fade out starting from on

	if (_ledCurState == LED_STATE_SOFTBLINK && dir == FADE_OUT)
	{	// In this case, rather than going directly to off, we'd like to continue the softblink until it naturally reaches off and then let it stop
		// We accomplish this by setting the _fadeToTarget flag, set _pwmTarget to zero, and SoftBlink will stop when it reaches it
		_pwmTarget = 0;
		_fadeToTarget = true;
		// Or we could do this: 
		// FadeTo(level);
		// But taking care of it within softblink ends up looking nicer. 
		return;
	}

	// Ok, this is not a fadeoff from softblink, this is a regular fadeoff. 

	// Constrain
    if (dir != FADE_IN && dir != FADE_OUT) dir = FADE_OUT;

	// Can we even analog-write to this pin? 
	if (_pwmable == false)
	{	// If we are not able to analog-write to the pin, just turn on or off directly
		if (dir == FADE_IN)  this->on();
		if (dir == FADE_OUT) this->off();
		return;
	}
	
	// Do we even need to do anything? 
	if (dir == FADE_OUT && _pwm == MIN_PWM) 
	{
		this->off();	// Just to get our status variables set correctly
		return;
	}
	if (dir == FADE_IN  && _pwm == MAX_PWM) 
	{
		this->on();		// Just to get our status variables set correctly
		return;
	}

    // Start clear
    clearUpdateProcess();
	changeLEDState(LED_STATE_FADE);
    
    // Set some flags
    _fadeDirection = dir;
	_fadeType = fadeType;
	
    // Determine delay between updates
    _numSteps = NUM_FADE_UPDATES;           // See LedHandler.h
    if (span < _numSteps) span = _numSteps; // Don't allow fades shorter than the number of updates, it would just result in 1 mS or 0 mS updates
    _nextWait = span / _numSteps;           // Interval between each fade update

	// We need to do some initiliazitions for the sine wave fade
	if (_fadeType == FADE_TYPE_SINE)
	{
		if (dir == FADE_IN)  _invert ? _fadeAdjustment = SINE_START_ANGLE_ON  : _fadeAdjustment = SINE_START_ANGLE_OFF;
		if (dir == FADE_OUT) _invert ? _fadeAdjustment = SINE_START_ANGLE_OFF : _fadeAdjustment = SINE_START_ANGLE_ON;
    }
	else
	{
		if (dir == FADE_IN)  _fadeAdjustment = FADE_ON_R_VAL;
		if (dir == FADE_OUT) _fadeAdjustment = FADE_OFF_RATIO;
	}

    // Reset the ellapsedMillis time
    _time = 0;
}

void OSL_LedHandler::FadeTo(uint8_t desiredLevel)
{
#define ignoreRange 10

	// Start clear
	clearUpdateProcess();

	if ((uint8_t)_pwm > (desiredLevel - ignoreRange) && (uint8_t)_pwm < (desiredLevel + ignoreRange))
	{
		// We're close enough, just go straight to the desired level
		this->clearUpdateProcess();
		changeLEDState(LED_STATE_DIM);
		this->setPWM(desiredLevel);
	}
	else
	{
		if ((uint8_t)_pwm > desiredLevel) 
		{
			_fadeDirection = FADE_OUT;
			_fadeAdjustment = FADE_TO_RATIO;
		}
		else
		{
			_fadeDirection = FADE_IN;
			_fadeAdjustment = 1.0 - FADE_TO_RATIO;
			if (_pwm == 0.0) _pwm = 1.0;	// We need to start at something greater than zero
		}
		
		// Fade to desired level
		changeLEDState(LED_STATE_FADE);
		_fadeToTarget = true;
		_pwmTarget = desiredLevel;
		_fadeType = FADE_TYPE_EXP;
		// We don't specify a number of steps here, because it will just take 
		//however many it takes to reach the new level. We just set the 
		// delay between steps
		_nextWait = 10;
		// Reset the ellapsedMillis time
		_time = 0;		
	}
}

void OSL_LedHandler::stopFading(void)
{
    clearUpdateProcess();
}

void OSL_LedHandler::randomBlink(void)
{
	// This effect blinks a light randomly. Each "flicker" (on or off) is randomly 
	// calculated as some interval between BF_Short and BF_Long (defined in AA_UserConfig.h)
	// The sketch will take care of stopping the random blinking, and that stop command will itself
	// come at some randomly calculated time. But here in the led handler class, we just need to calculate
	// random flickering. 
	this->pinOn();								// Start by turning on the output
	_nextWait = random(BFF_Short, BFF_Long);    // Calculate a random length of time between our short and long bookmarks
	_time = 0;									// Start the clock
	changeLEDState(LED_STATE_RANDOMBLINK);		// Change state to random blink
}

void OSL_LedHandler::Xenon(void)
{
	// The Xenon effect will only work with pins capable of PWM
	if (_pwmable == false)
	{	// If we are not able to analog-write to the pin, just turn on directly
		this->on();
		return;
	}

	switch (_ledCurState) 	// Current state, meaning, what is the state now before we start the Xenon effect
	{
		case LED_STATE_OFF:
		case LED_STATE_FADE:
			// We start the xenon effect by turning on the LED to full brightness briefly. The rest of the steps will be taken care of in update()
			_curProcessStep = 1;
			_curStep = 0;
			_nextWait = XENON_STEP_1_ON_TIME;
			this->pinOn();                      
			_time = 0;
			changeLEDState(LED_STATE_XENON);
			break;
		
		default:
			// Don't re-do the xenon effect if we were already on
			_ledCurState = LED_STATE_XENON;
			this->on();
			break;
	}
}

void OSL_LedHandler::softBlink(void)
{
	this->softBlinkWithStartFlag(true);		// True, meaning this is the start of the SoftBlink
}

void OSL_LedHandler::softBlinkWithStartFlag(boolean start /* =false */)
{
	// The Xenon effect will only work with pins capable of PWM
	if (_pwmable == false)
	{	// If we are not able to analog-write to the pin, just blink it regularly instead
		this->startBlinking(DEFAULT_BLINK_INTERVAL, DEFAULT_BLINK_INTERVAL);
	}
	else
	{
		// Start the softblink effect. The rest of the steps will be taken care of in update()
		changeLEDState(LED_STATE_SOFTBLINK);
		_curProcessStep = 1;
		_curStep = 0;
		_numSteps = SOFTBLINK_STEP_1_FADE_ON_STEPS; 
		_nextWait = SOFTBLINK_STEP_1_FADE_ON_TIME / _numSteps;
		_fadeAdjustment = SINE_START_ANGLE_OFF;
		_pwm = 1;
		if (start)
		{
			_fadeToTarget = false;	// If at any point the _fadeToTarget flag gets set to true, softblink will continue until it reaches _pwmTarget and then automatically stop. 
			_pwmTarget = -1;		// Set target to a non-valid number, we will also check to see if it gets set to something valid
		}
		_time = 0;
		// For calculating how long a cycle takes
		// TEST = millis();
	}
}

void OSL_LedHandler::update(void)
{
static boolean skipme = true;

    switch (_ledCurState)
	{
		case LED_STATE_BLINK:
		{
			if (_nextWait > 0 && _time > _nextWait)
			{
				_curStep += 1; 
				if (_curStep < _numSteps)
				{
					// LED on or off
					if (_curStep & 1) { this->pinOff(); }  // Odd numbers get turned off
					else              { this->pinOn();  }  // Even numbers get turned on

					// Calculate time to next change
					if (!_fixedInterval)
					{
						if   (_curStep > MAX_STREAM_STEPS) { this->pinOff(); clearUpdateProcess(); }    // This shouldn't happen, but if it does, stop the blinker
						else { _nextWait = _blinkStream.interval[_curStep]; _time = 0;   }     // Otherwise reset the time and wait for the next interval
					}
					else
					{
						_time = 0;  // In this case we just leave the next interval to what it was before (it's a fixed interval). But we do need to reset the time
					}
				}
				else
				{
					// We're done
					if (_blinkStream.repeat) 
					{   // Start over
						_nextWait = _blinkStream.interval[0];
						_curStep = 0;
						this->pinOn();
						_time = 0;
					}
					else
					{   // Turn off lights and wrapup
						this->off(); 
					}
				}
			}		
		}
		break;

		case LED_STATE_RANDOMBLINK:
		{
			if (_nextWait > 0 && _time > _nextWait)
			{
				this->toggle();		// Toggle output
				// Calculate new random interval for the next flicker
				_nextWait = random(BFF_Short, BFF_Long);
				_time = 0;
			}
		}
		break;

		case LED_STATE_FADE:
		{
			if (_nextWait > 0 && _time > _nextWait)
			{	
				if (_fadeToTarget == false)
				{
					// We are fading either to full on or full off
					_curStep += 1; 
					if (_curStep < _numSteps)
					{
						if (_fadeType == FADE_TYPE_SINE)
						{
							// See: https://www.sparkfun.com/tutorials/329
							// Full brightness = 90  degress = 1.570 radians (pi * 0.5)
							// Half brightness = 180 degrees = 3.141 radians (pi * 1  )
							// Off             = 270 degrees = 4.712 radians (pi * 1.5)
							_fadeAdjustment = _fadeAdjustment + (PI / (float)_numSteps);
							_pwm = (sin(_fadeAdjustment) * 127.5) + 127.5;
						}
						else
						{
							// Log
							if (_fadeDirection== FADE_OUT)
							{	// Fade out
								if (_pwm > MIN_PWM_FLOAT) _pwm = _fadeAdjustment * _pwm; 
								else	
								{ 
									offWithExtra(false);
									return;
								}
							}
							else
							{	// Fade in
								_pwm = pow(2, ((float)_curStep / _fadeAdjustment)) - 1;
								if (_pwm > MAX_PWM_FLOAT)
								{
									this->clearUpdateProcess();
									changeLEDState(LED_STATE_ON);
									this->pinOn();
									_pwm = MAX_PWM;
									return;
								}
							}
						}

						this->setPWM(_pwm);

						_time = 0;
					}
					else
					{
						switch (_fadeDirection)
						{
							// Fading-out ends with off
							case FADE_OUT: 
								offWithExtra(false);
								break;   
							
							// Fading-in ends with on
							case FADE_IN:  
							default:
								this->on();
								break;                        
						}
					}
				}
				else
				{
					// Here we are fading to dim, which is somewhere between full on and off
					if (_fadeDirection == FADE_OUT)
					{
						// Decrease pwm
						_pwm = _fadeAdjustment * _pwm; 
						
						// Have we reached our target yet? 
						if ((uint8_t)_pwm > _pwmTarget) 
						{	// No, keep going
							this->setPWM(_pwm);
							_time = 0;
						}
						else
						{	// Yes, this decrease would take us below the target level. 
							// Set to target and finish.
							this->clearUpdateProcess();
							changeLEDState(LED_STATE_DIM);
							this->setPWM(_pwmTarget);						
						}
					}
					else
					{	
						// Increase pwm
						_pwm = _pwm + (_fadeAdjustment * _pwm);

						// Have we reached our target yet? 
						if ((uint8_t)_pwm < _pwmTarget)
						{	// No, keep going
							this->setPWM(_pwm);
							_time = 0;
						}
						else
						{	// Yes, this increase would take us over the target level.
							// Set to target and finish.
							this->clearUpdateProcess();
							changeLEDState(LED_STATE_DIM);
							this->setPWM(_pwmTarget);											
						}
					}					
				}
			}		
		}
		break;

		case LED_STATE_XENON:
		{
			if (_nextWait > 0 && _time > _nextWait)
			{		
				switch (_curProcessStep)
				{
					case 1: 
						// We have finished the first On flash of the xenon bulb. Now turn off, or at least very low, momentarily
						_curProcessStep = 2;
						_nextWait = XENON_STEP_2_DIM_TIME;
						this->setPWM(XENON_STEP_2_DIM_LEVEL);
						_time = 0;						
						break;
					
					case 2:
						// We have finished the brief pause between the first flash and now the gradual fade in
						// Start the fade portion (handled in step 3)
						_curProcessStep = 3;
						_numSteps = XENON_STEP_3_FADE_STEPS;  
						_nextWait = XENON_STEP_3_FADE_TIME / _numSteps;	
						_time = 0;
						break;
						
					case 3:
					default:
						if (_curStep > _numSteps)
						{	// We're done, turn all the way on if we aren't already
							this->on();		// This will also clear the update process flag
						}
						else
						{	// We're still fading, increment to next fade level
							
							// Very slow change at low levels
							if (_pwm < 30)
							{	// At really low levels, we only update the PWM by one every other step
								if (skipme == false) _pwm += 1;
								skipme = !skipme;
							}
							else
							{	// Past a certain point we always increase at least by one
								_pwm += 1;
							}
							
							// Rate of increase continues to grow the brigher we get
							if (_pwm > 60)  _pwm += 1;
							if (_pwm > 100) _pwm += 1;		// These are cumulative
							if (_pwm > 150) _pwm += 1;
							if (_pwm > 210) _pwm += 1;
							
							if (_pwm < MAX_PWM)
							{
								this->setPWM(_pwm);	
								_curStep += 1;
								_time = 0;
								//Serial.print(_curStep - 3); Serial.print(" "); Serial.println(_pwm);
							}
							else
							{
								// We're done
								this->on();		// This will also clear the update process flag
							}
						}
						break;
				}		
			}
		}
		break; 
	
		case LED_STATE_SOFTBLINK:
		{	
			if (_nextWait > 0 && _time > _nextWait)
			{		
				switch (_curProcessStep)
				{
					case 1: 
						// We are fading in
						
						if (SOFTBLINK_TO_TARGET_FADEDOWN_ONLY == false)
						{	// Check for transition to a target level. If SOFTBLINK_TO_TARGET_FADEDOWN_ONLY is false, that means we can stop at the desired dim level on the upswing (which is what we are in now). 
							if (_curStep == 0 && _pwmTarget >= 0 && _fadeToTarget == false)
							{	
								_fadeToTarget = true;	// This lets us start at the very beginning of the fade-in, ensuring that we are presently below the target. We will stop on the way up when target is reached. 
							}
						}
						
						if (_curStep < _numSteps)
						{	// We use a sine curve for the fade in
							_fadeAdjustment = _fadeAdjustment + (PI / (float)_numSteps);
							_pwm = (sin(_fadeAdjustment) * 127.5) + 127.5;

							// If a _pwmTarget has been set, it means we want to stop the softblink and end at some predetermined level when that level is reached.
							// We are in the fade-in phase, so we check to see if the new calculated _pwm is greater than target, if so, we are done.
							// If SOFTBLINK_TO_TARGET_FADEDOWN_ONLY is not false, this check won't happen, and we will only allow a stop at the desired target level on the fade-down portion of softblink (below)
							if (SOFTBLINK_TO_TARGET_FADEDOWN_ONLY == false && _fadeToTarget == true && _pwmTarget > MIN_PWM && _pwmTarget < MAX_PWM && ((int16_t)_pwm >= _pwmTarget))
							{	
								_pwm = _pwmTarget;		// clearUpdateProcess will clear _pwmTarget so we need to save it now
								this->clearUpdateProcess();
								changeLEDState(LED_STATE_DIM);
								this->setPWM(_pwm);	
							}
							else
							{	// No dim to go to, just keep fading in
								this->setPWM(_pwm);
								_curStep += 1;
								// Serial.print(_curStep); Serial.print(" "); Serial.println(_pwm);
							}
						}
						else
						{
							// We are done fading in, turn on and wait
							
							// Except if we want to stop at on. Unlikely for softblink, but we allow it
							if (_fadeToTarget == true && _pwmTarget == MAX_PWM)
							{
								// We want to turn on and stop
								this->clearUpdateProcess();
								changeLEDState(LED_STATE_ON);
								this->on();
							}										
							else
							{
								// In this case we still turn on, but we don't stop
								this->pinOn();
								_curProcessStep = 2;
								_nextWait = SOFTBLINK_STEP_2_ON_TIME;
							}
						}
						_time = 0;
						break;
					
					case 2:
						// We are done staying on, start fading out
						_curProcessStep = 3;
						_curStep = 0;
						_numSteps = SOFTBLINK_STEP_3_FADE_OFF_STEPS;
						_nextWait = SOFTBLINK_STEP_3_FADE_OFF_TIME / _numSteps;
						_fadeAdjustment = SOFTBLINK_FADE_OFF_RATIO;
						_time = 0;
						// Check for transition to dim
						if (_fadeToTarget == false && _pwmTarget >= 0)
						{	
							_fadeToTarget = true;	// This lets us start at the very beginning of the fade-out, ensuring that we are above the target. We will stop and change to Dim when target is reached. 
						}
						break;
						
					case 3:
						// We are fading out
						if (_curStep < _numSteps)
						{	// We use an exponential curve for the fade out
							_pwm = _pwm * _fadeAdjustment;
							
							// If _fadeToTarget flag has been set, it means we want to stop the softblink and end at some predetermined point 
							// Here we are only checking the case where _pwmTarget is something above zero, ie, Dim. We handle the _pwmTarget = 0 case below
							if (_fadeToTarget == true && _pwmTarget > 0 && ((int16_t)_pwm <= _pwmTarget))	
							{	
								_pwm = _pwmTarget;		// clearUpdateProcess will clear _pwmTarget so we need to save it now
								this->clearUpdateProcess();
								changeLEDState(LED_STATE_DIM);
								this->setPWM(_pwm);	
							}
							else
							{	// Otherwise we are just blinking like normal
								this->setPWM(_pwm);
								_curStep += 1;
								// Serial.print(_curStep); Serial.print(" "); Serial.println(_pwm);	
								_time = 0;	
							}
						}
						else
						{
							// How long does a cycle actually take, which is always a bit longer than we specify. 
							// Default settings is about 810 mS
							// Serial.print("T- ");
							// Serial.println(millis() - TEST);
							// TEST = millis();	
							
							if (_fadeToTarget == true && _pwmTarget == 0)
							{
								// We want to turn off
								this->clearUpdateProcess();
								changeLEDState(LED_STATE_OFF);
								this->off();
							}							
							else
							{
								// We're done fading out, start over and fade in
								this->softBlinkWithStartFlag(false);		// False, meaning this is a repetition of SoftBlink, not the start
							}
						}
						break;
				
				}
			}
		}
			
		default: 
			break;
	}
}




