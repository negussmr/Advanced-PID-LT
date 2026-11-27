Advanced PID Line Tracker

«A 16-sensor line follower with PID, recovery logic, and a programmable movement planner.
Because sometimes following a line is easy, until the line decides to disappear.»

What is this?

Advanced PID LT is an Arduino-based line-following robot firmware built around a 16-channel analog sensor array (read through a multiplexer). It combines:

- PID line tracking to keep the robot centered on the line.
- Sensor calibration for black and white surfaces.
- Line-loss recovery to search for the line when it disappears.
- A movement planner for timed or marker-triggered actions.
- Button control to start and stop a run.
- Buzzer feedback for startup, calibration, and run toggles.

The planner is optional. Leave it empty for normal PID line following—or give it a plan and let it temporarily take the wheel.

---

Hardware / Pin Map

Function| Arduino pin
Multiplexer SIG| A0
MUX S0| A4
MUX S1| A3
MUX S2| A2
MUX S3| A1
Start/stop button| D11
Buzzer| D6
Motor A direction 1| D7
Motor A direction 2| D8
Motor A PWM| D9
Motor B direction 1| D5
Motor B direction 2| D4
Motor B PWM| D10

Libraries: No extra Arduino library is required by the included source files. "Planner.h", "Planner.cpp", and "Plan.h" are part of this project.

«Check your motor driver, sensor multiplexer, motor polarity, and wiring before powering the robot. The pin map is firmware-specific; your actual hardware must match it.»

---

Getting Started

1. Open "Main.ino" in the Arduino IDE.
2. Keep "Planner.cpp", "Planner.h", and "Plan.h" in the same sketch folder.
3. Select your Arduino board and port.
4. Upload the sketch.
5. Let the robot power up and finish its startup buzzer.
6. Calibrate when prompted.
7. Tap the start/stop button to begin or stop the run.

---

Calibration

The robot needs to learn what black and white look like before it can follow a line.

1. Black calibration: Place the sensors over the black line or black surface, then tap the button to capture.
2. White calibration: Place the sensors over the white background, then tap to capture.
3. The firmware checks the difference between the black and white readings of each sensor.
4. If fewer than 8 of the 16 sensors have enough separation, calibration repeats.

Try to keep the sensor array over representative black and white surfaces during calibration.

The robot is not psychic; it only knows the readings you teach it.

---

Movement Planner

Movement instructions are configured in "Plan.h".

Each plan entry follows this format:

{ TRIGGER, VALUE, ACTION, PARAM, DURATION_MS }

Field| Meaning
"TRIGGER"| "AT_MS" or "AT_MARKER"
"VALUE"| Time in milliseconds, or marker number
"ACTION"| The action to execute
"PARAM"| Action parameter, usually speed
"DURATION_MS"| How long a timed movement action runs

Triggers

Trigger| Description
"AT_MS"| Executes when elapsed run time reaches "VALUE"
"AT_MARKER"| Executes when the planner has counted at least "VALUE" markers

Available Actions

Action| Description
"GO_STRAIGHT"| Runs both motors at "PARAM" for the specified duration
"TURN_LEFT"| Runs Motor B at "PARAM", Motor A at zero, for the duration
"TURN_RIGHT"| Runs Motor A at "PARAM", Motor B at zero, for the duration
"SET_SPEED"| Changes the base PID-following speed to "PARAM"
"STOP"| Stops both motors and ends the run
"END"| Terminates the plan

Example Plan

const Step plan[] = {
  // Trigger       Value  Action        Param  Duration
  { AT_MS,        1000,  GO_STRAIGHT, 180,   500 },
  { AT_MARKER,       1,  TURN_LEFT,   180,   300 },
  { AT_MARKER,       2,  SET_SPEED,   150,     0 },
  { AT_MS,        5000,  STOP,          0,      0 },
  { AT_MS,           0,  END,           0,      0 }
};

Plan steps execute in order.

- "SET_SPEED" changes the base speed used by PID.
- "STOP" stops the planner and ends the run.
- "END" is the plan terminator.
- Marker width defaults to 8 active sensors.

The planner is a sequence executor, not an autonomous route solver. It won't magically understand a maze. Yet.

---

PID Tuning

PID values are near the top of "Main.ino".

float Kp = 0.6;
float Ki = 0.000001;
float Kd = 30.0;

Parameter| Meaning
"Kp"| Proportional response to current line error
"Ki"| Accumulates error over time
"Kd"| Responds to changes in error

- Too much Kp can cause zigzagging.
- Too little Kp can make the robot sluggish.
- Too much Ki can cause overshoot and accumulated error.
- Too much Kd can make the response twitchy.

Change one value at a time and test on a consistent track.

PID tuning is engineering, not a Ouija board.

Other Parameters

Variable| Value| Purpose
"defaultSpeed"| 250| Initial PID/planner base speed
"slowFactor"| 0.004| Slows down based on error magnitude
"loopPeriodUs"| 2000| Control-loop interval
"noiseGate"| 150| Ignores weak normalized sensor readings
"minTotal"| 300| Minimum signal for line detection
"lostConfirmMs"| 20| Confirmation time before declaring line lost
"gapTimeMs"| 250| Brief straight behavior during a gap
"searchTimeoutMs"| 2000| Maximum line-search duration
"gapSpeed"| 150| Speed during short gap handling
"searchSpeed"| 220| Speed used during line searching
"minSeparation"| 50| Minimum black/white sensor difference
"minValidSensors"| 8| Minimum valid sensors for calibration
"calSamples"| 200| Samples averaged during calibration

---

Function Reference

"Main.ino"

Function| Description
"readAllSensors()"| Reads all 16 MUX channels into "sensorValues[]"
"normalized(byte ch)"| Converts raw readings into calibrated values from 0–1000
"sampleSensors(int *target)"| Averages repeated sensor readings into a calibration array
"calibrate()"| Performs black/white calibration and validates the sensors
"followLine()"| Calculates line position, error, PID correction, and motor speeds
"computePID(long error)"| Calculates PID correction
"searchForLine(unsigned long lostFor)"| Handles gaps and line-loss searching
"setMotorA(int speed)"| Sets Motor A forward PWM speed
"setMotorB(int speed)"| Sets Motor B forward PWM speed
"setMotorsPair(int left, int right)"| Sends speed commands to both motors
"stopMotors()"| Stops both motors
"handleButton()"| Toggles running state and resets the controller/planner when starting
"selectMuxChannel(byte channel)"| Selects a MUX channel using the select pins
"playStartupSong()"| Plays the startup buzzer sequence
"playTapSong(bool starting)"| Plays feedback for run-state changes
"beep(int times)"| Plays a repeated beep
"errorBeep()"| Signals an error
"waitForTap()"| Waits for the calibration button sequence
"printSensors()"| Prints sensor readings to Serial

"Planner.h" / "Planner.cpp"

Function| Description
"Planner(...)"| Creates the planner using a plan, motor callback, default speed, and marker width
"reset()"| Resets the planner to the beginning
"update(int activeSensors)"| Counts markers and advances plan execution
"baseSpeed()"| Returns the current PID base speed
"stopRequested()"| Indicates that the planner requested a stop
"consumeStepDone()"| Returns and clears the completed-step flag
"markers()"| Returns the current marker count

---

Serial Debugging

Enable Serial debugging in "Main.ino":

const bool debugPrint = true;

When enabled, the sketch prints the 16 raw sensor values separated by tabs.

Set it to "false" during normal operation.

---

Limitations

- Requires 16 analog sensors connected through the configured multiplexer.
- Calibration values are stored in RAM and must be redone after power cycling.
- The planner executes a fixed sequence of steps.
- Marker detection is based on the number of active sensors, not complex marker recognition.
- Turning behavior depends on motor wiring and direction configuration.
- "END" terminates the plan; it does not calculate routes.
- The default "Plan.h" is empty apart from the "END" entry, so the robot operates as a normal PID line follower by default.

---

License

See ""LICENSE"" (LICENSE).

---

If It Works First Try

Suspicious.

Check the wiring, then celebrate.
