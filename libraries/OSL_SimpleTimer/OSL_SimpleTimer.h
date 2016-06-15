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
 * OSL_SimpleTimer.h
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
 *
 */


#ifndef OSL_SIMPLETIMER_H
#define OSL_SIMPLETIMER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


typedef void (*timer_callback)(void);

class OSL_SimpleTimer {

public:
    // maximum number of timers
    const static int MAX_TIMERS = 10;

    // setTimer() constants
    const static int RUN_FOREVER = 0;
    const static int RUN_ONCE = 1;

    // constructor
    OSL_SimpleTimer();

    // this function must be called inside loop()
    void run();

    // call function f every d milliseconds
    unsigned int setInterval(long d, timer_callback f);

    // call function f once after d milliseconds
    unsigned int setTimeout(long d, timer_callback f);

    // call function f every d milliseconds for n times
    unsigned int setTimer(long d, timer_callback f, int n);

    // destroy the specified timer
    void deleteTimer(unsigned int ID);

    // restart the specified timer
    void restartTimer(unsigned int ID);

    // returns true if the specified timer is enabled
    boolean isEnabled(unsigned int ID);

    // enables the specified timer
    void enable(unsigned int ID);

    // disables the specified timer
    void disable(unsigned int ID);

    // enables the specified timer if it's currently disabled,
    // and vice-versa
    void toggle(unsigned int ID);

    // returns the number of used timers
    int getNumTimers();

    // returns the number of available timers
    int getNumAvailableTimers() { return MAX_TIMERS - numTimers; };
	
	// Gets the timer number (0-MAX_TIMERS) by ID
	int getTimerNum(unsigned int ID);

private:
    // deferred call constants
    const static int DEFCALL_DONTRUN = 0;       // don't call the callback function
    const static int DEFCALL_RUNONLY = 1;       // call the callback function but don't delete the timer
    const static int DEFCALL_RUNANDDEL = 2;     // call the callback function and delete the timer

    // find the first available slot
    int findFirstFreeSlot();

    // value returned by the millis() function
    // in the previous run() call
    unsigned long prev_millis[MAX_TIMERS];

    // pointers to the callback functions
    timer_callback callbacks[MAX_TIMERS];

    // delay values
    long delays[MAX_TIMERS];

    // number of runs to be executed for each timer
    int maxNumRuns[MAX_TIMERS];

    // number of executed runs for each timer
    int numRuns[MAX_TIMERS];

    // which timers are enabled
    boolean enabled[MAX_TIMERS];

    // deferred function call (sort of) - N.B.: this array is only used in run()
    int toBeCalled[MAX_TIMERS];

	// IDs for each timer (not equal to the timer number)
	unsigned int timerID[MAX_TIMERS];
	unsigned int NextID;	

    // actual number of timers in use
    int numTimers;
};

#endif