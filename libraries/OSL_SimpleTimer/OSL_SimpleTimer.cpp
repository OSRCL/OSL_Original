/* Open Source Lights - Simple Timer Library
 * 
 * This library has been modified to fix some bugs. It now returns 
 * a unique ID for each timer created (unsigned int). This prevents
 * routines from inadvertently deleting timers that are not theirs,
 * but which have re-used the same slot number. 
 *
 * The library has also been renamed from SimpleTimer to OSL_SimpleTimer
 * to prevent any conflicts with other SimpleTimer libraries you may
 * have installed. 
 * 
 * May 2015 - Luke Middleton
 */

/*
 * SimpleTimer.cpp
 *
 * SimpleTimer - A timer library for Arduino.
 * Author: mromani@ottotecnica.com
 * Copyright (c) 2010 OTTOTECNICA Italy
 *
 * This library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser
 * General Public License along with this library; if not,
 * write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include "OSL_SimpleTimer.h"


// Select time function:
//static inline unsigned long elapsed() { return micros(); }
static inline unsigned long elapsed() { return millis(); }


OSL_SimpleTimer::OSL_SimpleTimer() {
    unsigned long current_millis = elapsed();

	NextID = 1; // Initialize Next ID

    for (int i = 0; i < MAX_TIMERS; i++) {
        enabled[i] = false;
        callbacks[i] = 0;                   // if the callback pointer is zero, the slot is free, i.e. doesn't "contain" any timer
        prev_millis[i] = current_millis;
        numRuns[i] = 0;
		timerID[i] = 0;						// Initialize IDs to Zero, which is an invalid ID
    }

    numTimers = 0;
}


void OSL_SimpleTimer::run() {
    int i;
    unsigned long current_millis;

    // get current time
    current_millis = elapsed();

    for (i = 0; i < MAX_TIMERS; i++) {

        toBeCalled[i] = DEFCALL_DONTRUN;

        // no callback == no timer, i.e. jump over empty slots
        if (callbacks[i]) {

            // is it time to process this timer ?
            // see http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592

            if (current_millis - prev_millis[i] >= delays[i]) {

                // update time
                //prev_millis[i] = current_millis;
                prev_millis[i] += delays[i];

                // check if the timer callback has to be executed
                if (enabled[i]) {

                    // "run forever" timers must always be executed
                    if (maxNumRuns[i] == RUN_FOREVER) {
                        toBeCalled[i] = DEFCALL_RUNONLY;
                    }
                    // other timers get executed the specified number of times
                    else if (numRuns[i] < maxNumRuns[i]) {
                        toBeCalled[i] = DEFCALL_RUNONLY;
                        numRuns[i]++;

                        // after the last run, delete the timer
                        if (numRuns[i] >= maxNumRuns[i]) {
                            toBeCalled[i] = DEFCALL_RUNANDDEL;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < MAX_TIMERS; i++) {
        switch(toBeCalled[i]) {
            case DEFCALL_DONTRUN:
                break;

            case DEFCALL_RUNONLY:
                (*callbacks[i])();
                break;

            case DEFCALL_RUNANDDEL:
                (*callbacks[i])();
                deleteTimer(timerID[i]);	// Pass the unique ID, not the Timer Number
                break;
        }
    }
}


// find the first available slot
// return -1 if none found
int OSL_SimpleTimer::findFirstFreeSlot() {
    int i;

    // all slots are used
    if (numTimers >= MAX_TIMERS) {
        return -1;
    }

    // return the first slot with no callback (i.e. free)
    for (i = 0; i < MAX_TIMERS; i++) {
        if (callbacks[i] == 0) {
            return i;
        }
    }

    // no free slots found
    return -1;
}


unsigned int OSL_SimpleTimer::setTimer(long d, timer_callback f, int n) {
    unsigned int returnID;
	int freeTimer;

    freeTimer = findFirstFreeSlot();
    if (freeTimer < 0) {
		return -1;
    }

    if (f == NULL) {
        return -1;
    }

    delays[freeTimer] = d;
    callbacks[freeTimer] = f;
    maxNumRuns[freeTimer] = n;
    enabled[freeTimer] = true;
    prev_millis[freeTimer] = elapsed();
	timerID[freeTimer] = NextID;

    // Increment number of timers
	numTimers++;				
	
	// Increment timer ID
	returnID = NextID;
	if (NextID + 1 == 0) { NextID = 1; } // rollover
	else { NextID++; }					  // increment

    //return freeTimer; // OLD	
//	Serial.print(F("Created ")); Serial.print(returnID); Serial.print(" ("); Serial.print(freeTimer); Serial.println(F(")"));
	return (returnID);
}


unsigned int OSL_SimpleTimer::setInterval(long d, timer_callback f) {
    return setTimer(d, f, RUN_FOREVER);
}


unsigned int OSL_SimpleTimer::setTimeout(long d, timer_callback f) {
    return setTimer(d, f, RUN_ONCE);
}


void OSL_SimpleTimer::deleteTimer(unsigned int ID) 
{
    int timerNum;
	
    // nothing to delete if no timers are in use
    if (numTimers == 0) {
        return;
    }

	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return;
	}

    // don't decrease the number of timers if the
    // specified slot is already empty
    if (callbacks[timerNum] != NULL) {
        callbacks[timerNum] = 0;
        enabled[timerNum] = false;
        toBeCalled[timerNum] = DEFCALL_DONTRUN;
        delays[timerNum] = 0;
        numRuns[timerNum] = 0;
		timerID[timerNum] = 0;

        // update number of timers
        numTimers--;
		
		//Serial.print(F("Deleted ")); Serial.print(ID); Serial.print(" ("); Serial.print(timerNum); Serial.println(F(")"));
    }
}


void OSL_SimpleTimer::restartTimer(unsigned int ID) 
{

    int timerNum;
	
	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return;
	}
	
    prev_millis[timerNum] = elapsed();
}


boolean OSL_SimpleTimer::isEnabled(unsigned int ID) 
{
    int timerNum;
	
	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return false;
	}
	
    return enabled[timerNum];
}


void OSL_SimpleTimer::enable(unsigned int ID) 
{
    int timerNum;
	
	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return;
	}

    enabled[timerNum] = true;
}


void OSL_SimpleTimer::disable(unsigned int ID) 
{
    int timerNum;
	
	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return;
	}
	
    enabled[timerNum] = false;
}


void OSL_SimpleTimer::toggle(unsigned int ID) 
{
    int timerNum;
	
	timerNum = getTimerNum(ID);
	
	if (timerNum == -1) {
		return;
	}
	
	enabled[timerNum] = !enabled[timerNum];
}


int OSL_SimpleTimer::getNumTimers() {
    return numTimers;
}

int OSL_SimpleTimer::getTimerNum(unsigned int ID)
{
	int timerNum = -1;
	
	for (int i = 0; i < MAX_TIMERS; i++) 
	{
		if (timerID[i] == ID) { timerNum = i; break;}
	}
	
	return timerNum;
}