// ---------------------------------------------------------------------------------------------------------------------------------------------------->
// THIS IS WHERE YOU ADJUST THE PROGRAM FUNCTIONALITY
// ---------------------------------------------------------------------------------------------------------------------------------------------------->

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// NUMBER OF SCHEMES
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
        #define NumSchemes                   2          // The number of lighting schemes implemented. Theoretically it can be anything up the memory limit. Defaults to 2. 
                                                        // MAKE SURE THIS NUMBER MATCHES THE NUMBER OF SCHEMES DEFINED IN AA_LIGHT_SETUP !!


// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// RADIO AND ESC 
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
    
    // Electronic Speed Controller
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        #define DoubleTapReverse          true          // Most ESCs require you to tap reverse twice before the car actually goes into reverse. 
                                                        // If yours is like this, set it to true. Most touring cars operate like this. 
                                                        // But if you can shift directly into reverse from forward, set this to false - this is typical of crawlers.
    // Channel Deadband
    // -------------------------------------------------------------------------------------------------------------------------------------------->
    // Because the receiver signal will tend to drift even when your radio controls are resting at center, you may find your turn signals or other lights 
    // turning on randomly. To prevent this from happening you can adjust the deadband, and if that is insufficient, you can enabled channel smoothing (later below). 
    // Start with deadband first, it reduces the sensitivity around the channel center point. 
    // The channel values (as used in this program) range from 0 (center) to 100 (full travel). If you set the deadband to 10, that means any command 
    // of 10 or less will be ignored. This will reduce sensitivity, but since we are only controlling lights here and not the actual vehicle, that's fine. 
    // Experiment with the values, but keep them as low as practical. 
    // If that doesn't work, try channel smoothing below. Another thing is to try running through Radio Setup again, to make sure OSL really does know your 
    // radio's actual center points. 
        
        #define ThrottleDeadband            10          // Throttle channel hysteresis. Values below this will be ignored. Default is 10, number should be small. 
        #define TurnDeadband                20          // Same thing, but for steering channel. 

    
    // Channel Smoothing
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        // Unlike deadband which ignores minor RC changes around the center point, smoothing will average the incoming signals. This will eliminate 
        //random glitching, but comes once again at the expense of decreased sensitivity. 
        
        #define smoothingStrength         1             // Number from 0-4, the higher the number the greater the smoothing. Use minimum acceptable value.
        #define SmoothThrottle            false
        #define SmoothSteering            false
        #define SmoothChannel3            false


// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// STATE ADJUSTMENTS
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
    
    // StopDelay (activates after the vehicle has been stopped for some time)
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        #define LongStopTime_mS         30000L          // The Stop Delay state only occurs when the vehicle has been stopped for some length of time, which is set here. 
                                                        // Recall that 1000 mS = 1 second (default value is 30 seconds) (for large define numbers, we put an "L" after the number)
    
    // Brake Lights at Low Throttle
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        #define BrakeAtThrottlePctBelow     0           // Background: Normally the Brake state can only occur if DoubleTapReverse = true and you hit reverse for the first time. This doesn't put you into reverse, 
                                                        // instead it turns on the brake lights. Only the second time you command reverse will the reverse state actually occur. 
                                                        // If DoubleTapReverse = false, there is no way for the brake state to ever occur, because you can instantly transition from forward to reverse and vice versa. 
                                                        // Of course you also have the option of setting your brake lights to ON in the Stop state, but then they are always on when stopped, which you may or may not want. 
                                                        // What this setting does: here you can set a percent and whenever the throttle command is above zero but less than or equal to that percent, 
                                                        // the Brake state will occur. This allows your brake lights to come on when you slow down, but you are still able to have them turn off when stopped. 
                                                        // Set this value to 0 (zero) to disable the low throttle brake lights effect. 
    // Turn Signals
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        #define BlinkTurnOnlyAtStop       true          // If you only want your turn signal blinkers to come on when the car is Stopped, set this to true. 
                                                        // Turn signals are cool, but they look silly when they start blinking every time you turn the steering wheel while driving. 
                                                        // For that reason, you will probably want to keep this "true"
                                                        // NOTE: This only applies to a BLINK or SOFTBLINK setting in the "RightTurn" or "LeftTurn" states. 
                                                        // Any setting other than BLINK or SOFTBLINK in the "RightTurn" or "LeftTurn" column will NOT be affected. 
        #define AllTurnSettingsMatch     false          // Set to true to restrict all other left or right turn settings (not just BLINK and SOFTBLINK) to the same conditions imposed by BlinkTurnOnlyAtStop
    
        #define TurnSignalDelay_mS        3000          // If BlinkTurnOnlyAtStop = true, this setting further refines when the turn signals can come on. Instead of coming on right when the 
                                                        // car reaches a stop, you can set a delay in milliseconds (1000 mS = 1 second) before they will be enabled. This way, if you come 
                                                        // to a stop while the wheels are turned the turn signals will not come on instantly, which looks very strange. 
                                                        // Instead there will be a delay of TurnSignalDelay_mS milliseconds after which you can hold the wheels over and the turn signals will then come on. 
                                                        // Once again we are trying to prevent the unrealistic engagement of turn signals, but rather have them only engaged when you specifically
                                                        // want to for display purposes. 
        #define TurnFromStartContinue_mS  1000          // If BlinkTurnOnlyAtStop = true, this setting determines the length of time the turn signal will continue to blink when you begin moving from 
                                                        // a stop with the wheels turned. In a real car, the blinker remains on through the turn but then is cancelled after the steering wheel returns
                                                        // to center. That is the effect we are trying to mimic, but we don't do it by checking the steering wheel, we simply set a length of time for the 
                                                        // turn signal to continue blinking. If you don't want this effect to happen, set this to 0 (zero). 

    // Acceleration and Deceleration
    // -------------------------------------------------------------------------------------------------------------------------------------------->
        #define AccelPct                    35          // How much does the throttle have to increase (1-100 pct) to be considered a sharp acceleration.
                                                        // This will trigger the OvertakeTime timer set below, during which your lights will do whatever
                                                        // setting you put in the Accelerating column (BLINK or FASTBLINK makes sense, like they do in 
                                                        // 24hr Le Mans when overtaking)
        #define OvertakeTime               500          // How long should the overtake event last in  ms (1000ms = 1 second)
    
        #define DecelPct                    20          // How much does the throttle need to decrease (1-100 pct) to be considered a sharp deceleration.
                                                        // This will trigger the Backfire effect for any light in the Decelerating column with the setting
                                                        // of BACKFIRE. You can put other settings in the Decelerating column besides Backfire, and they will 
                                                        // work, but they will only be enabled for the same length of time as the backfire event
        #define BF_Time_Short              200          // How long in milliseconds (1000 ms = 1 second) on average should a backfire event last. It will actually be 
        #define BF_Time_Long               600          // a random length of time spanning from BF_Time_Short to BF_Time_Long
        
        #define BFF_Short                   30          // The two defines above determine the total length of time the backfire event will take, these two defines determine the upper and lower
        #define BFF_Long                   100          // limits to the time each individual blink (or flicker) will take within the overall backfire event. In other words, while backfiring 
                                                        // the light will blink randomly on and off for some values between BFF_Short and BFF_Long

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// LIGHT SETTINGS
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->

    // Blinking
    // ------------------------------------------------------------------------------------------------------------------------------------------------>    
        #define BlinkInterval              378          // A value in milliseconds that sets the blink rate for blinking lights set to "BLINK" or "BLINK_ALT" (for example, turn signals). 1000 = 1 second
        #define FastBlinkInterval           50          // A value in milliseconds that sets the fast blink rate for lights set to "FASTBLINK" or "FASTBLINK_ALT"
                                                        // The "_ALT" versions blink just like the regular versions, but in an alternating fashion - that is, if you have one output set to FASTBLINK, 
                                                        // and another to FASTBLINK_ALT, they will each blink at the same rate (FastBlinkInterval) but when one is on the other will be off, and vice-versa. 
                                                        // This can be used to create effects such as those seen on police cars, fire trucks, and other emergency vehicles.
        

    // Dim
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        #define DimLevel                    50          // The level of brightness for the DIM setting, this is a number from 0-255, with 0 being off, 255 being full on. 
                                                        // Often numbers much greater than half (128) are hard to distinguish from full on. Experiment to get the number 
                                                        // that makes your lights as dim as you want them. 
        
    // Fadein and Fadeout
    // ------------------------------------------------------------------------------------------------------------------------------------------------>  
        #define FadeOutTime                300         // Length of time the fade out takes, in milliseconds (1000 mS = 1 second)
        #define FadeInTime                  50         // Length of time the fade-in takes. For realism this should usually be quite short, but the minimum 
                                                       // value is 50 (1/20th of a second). If you set it lower than that, it will automatically be changed to 50 at runtime.
    
    // "Safety Lights"
    // ------------------------------------------------------------------------------------------------------------------------------------------------>
        // "Safety Lights" are the kind of alternating blinking that are seen on ambulances, police cars and other emergency vehicles. It consists of a pattern of blinks
        // on one side of the vehicle, then that same pattern being displayed on the other side, alternating back and forth. This effect is created by using the 
        // SAFETYBLINK and SAFETYBLINK_ALT settings in AA_LightSetup. 
        #define SafetyBlinkRate             40         // Rate of blinking for the SafetyBlink effect in milliseconds - small numbers are fast, large numbers are slow. 
        #define SafetyBlinkCount             3         // The number of blinks in a row, followed by a pause while the "ALT" side performs the same series.
        #define SafetyBlink_Pause           80         // The length of time to pause between a series of blinks on one side followed by a series on the other side, in milliseconds. 
                                                       // If you don't want a pause, set this to zero. 



// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// SHELF-QUEEN MODE
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
    // Shelf-queen mode allows the OSL to operate even without radio signals. You can use this for models that will be put on display (aka, "shelf queens"),
    // just apply power to the OSL without connecting that radio signal wire. 
    // If enableShelfQueenMode = true, and the OSL does not detect a radio signal, it will set all lights to their settings in "Pos 1" (first position) of 
    // Channel 3 for the scheme number specified ("shelfQueenSchemeNumber")
    // NOTE: the shelf-queen check will only happen once at startup. If a radio signal is later detected, the OSL will return to normal operation and it will 
    // initialize itself to the prior active scheme regardless of what the shelf queen scheme number is set to below. If the radio signal is lost after that, 
    // OSL will go into failsafe mode as usual, it will not attempt to return to shelf-queen mode until the next startup. 
    // NOTE: if you specify a scheme number greater than 1, you need to make sure you have that scheme defined in AA_LIGHT_SETUP!

        #define enableShelfQueenMode      true
        #define shelfQueenSchemeNumber       2

    // We can also add a random delay before the lights come on in shelf queen mode, this may be desirable if you have multiple models on display with a single
    // power source to all of them, the random delay will mean the lights on each model will turn on at a different time. To enable this feature set 
    // "enableShelfQueenDelay" to true and then define the range of time within which you want the random delay to occur ("sqd_Time_Short" and "sqd_Time_Long").
    // If you want the lights to come on immediately, set "enableShelfQueenDelay" to false. 

        #define enableShelfQueenDelay     true
        #define sqd_Time_Short             100
        #define sqd_Time_Long             6000


// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
// DEBUGGING
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------->
    #define DEBUG                    false          // Set this to true to receive debugging messages out the serial port. 
    #define LED_DEBUG                 true          // If this is set to true, the Green LED on the board will be on whenever the car is moving forward, 
                                                    // the Red LED will come on whenever the car is moving in reverse, 
                                                    // both LEDs will turn OFF when the car is stopped,  
                                                    // both LEDs will turn ON when the car is braking,
                                                    // the Red LED will blink if you are turning left, and
                                                    // the Green LED will blink if you are turning right.
                                                    // You can use these to verify the board is working correctly without having any external lights connected.
    #define BLINK_LIGHTS_RX_LOST      false         // If true, all eight LED outputs will blink rapidly when the radio signal has been lost. 
                                                    // If set to false, only the onboard Red and Green LEDs will blink when the radio signal has been lost.
