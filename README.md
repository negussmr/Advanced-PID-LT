Advanced PID Line Tracker

<p align="center">
  <b>A 16-channel PID line follower with calibration, line recovery, and a programmable movement planner.</b>
  <br>
  <i>It follows the line. Hopefully the line cooperates.</i>
</p><p align="center">
  <img src="https://img.shields.io/badge/Platform-Arduino-00979D?style=for-the-badge&logo=arduino" />
  <img src="https://img.shields.io/badge/Control-PID-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Sensors-16-orange?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Language-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus" />
</p>---

About

Advanced PID LT is an Arduino-based line-following robot designed to track lines using 16 analog sensors connected through a multiplexer.

Unlike a basic line follower, this project includes a PID control system, sensor calibration, line-loss recovery, and a programmable planner that allows predefined actions to be executed based on elapsed time or detected markers.

The planner is optional. Without a custom plan, the robot operates as a regular PID line follower.

«It has 16 sensors, PID control, and a movement planner. Meanwhile, we still can't guarantee it'll behave at intersections.»

Features

- [x] 16-channel analog sensor reading
- [x] Black and white sensor calibration
- [x] PID line-following algorithm
- [x] Proportional, integral, and derivative control
- [x] Adaptive speed reduction based on line error
- [x] Line-loss detection
- [x] Gap handling and line-search recovery
- [x] Marker detection
- [x] Time-triggered planner actions
- [x] Marker-triggered planner actions
- [x] Adjustable base speed
- [x] Start/stop button
- [x] Buzzer feedback
- [x] Serial sensor debugging

---

Table of Contents

- "Hardware" (#hardware)
- "Pin Configuration" (#pin-configuration)
- "Getting Started" (#getting-started)
- "Calibration" (#calibration)
- "PID Configuration" (#pid-configuration)
- "Movement Planner" (#movement-planner)
- "Triggers" (#triggers)
- "Available Actions" (#available-actions)
- "Plan Example" (#plan-example)
- "Function Reference" (#function-reference)
- "Debugging" (#debugging)
- "Limitations" (#limitations)

---

Hardware

The firmware is designed around:

- Arduino-compatible board
- 16 analog line sensors
- 16-channel analog multiplexer
- Dual motor driver
- 2 DC motors
- Start/stop push button
- Buzzer

Make sure your wiring matches the pin configuration below before uploading.

Pin Configuration

Component| Pin
MUX SIG| "A0"
MUX S0| "A4"
MUX S1| "A3"
MUX S2| "A2"
MUX S3| "A1"
Button| "D11"
Buzzer| "D6"
Motor A IN1| "D7"
Motor A IN2| "D8"
Motor A PWM| "D9"
Motor B IN1| "D5"
Motor B IN2| "D4"
Motor B PWM| "D10"

---

Getting Started

1. Download the repository

Clone the repository or download it as a ZIP.

2. Open the sketch

Open "Main.ino" using the Arduino IDE.

Make sure these files are located in the same sketch directory:

- "Main.ino"
- "Planner.h"
- "Planner.cpp"
- "Plan.h"

3. Select your board

Choose your Arduino board and the correct COM/USB port from the Arduino IDE.

4. Upload

Compile and upload the sketch.

5. Calibrate

Follow the black-and-white calibration procedure after startup.

6. Start the robot

Press the button to toggle the robot's running state.

That's it. The robot should now be following the line using PID.

---

Calibration

The robot needs to know the difference between the line and the background.

Calibration collects sensor readings from both surfaces and uses them to normalize future readings.

Calibration procedure

1. Position the sensors over the black surface.
2. Press the calibration button to capture readings.
3. Position the sensors over the white surface.
4. Press the button again to capture readings.
5. The firmware validates the sensors.
6. If calibration fails, the process repeats.

Each sensor is considered valid when the difference between its black and white readings is at least "50".

At least 8 valid sensors are required for calibration to succeed.

<details>
<summary>What happens during calibration?</summary>The firmware samples each sensor 200 times and averages the readings.

The averaged black and white values are used for normalization. Sensors that do not show sufficient difference between the two surfaces are excluded from valid sensor readings.

In other words, if a sensor can't distinguish black from white, the firmware politely refuses to trust it.

</details>---

PID Configuration

The PID constants are defined in "Main.ino".

float Kp = 0.6;
float Ki = 0.000001;
float Kd = 30.0;

Parameter| Role| Effect
"Kp"| Proportional| Reacts to current error
"Ki"| Integral| Accumulates past error
"Kd"| Derivative| Reacts to error changes

Other parameters

Variable| Default| Description
"defaultSpeed"| "250"| Initial base speed
"slowFactor"| "0.004"| Speed reduction factor based on error
"loopPeriodUs"| "2000"| Control-loop period
"noiseGate"| "150"| Sensor noise threshold
"minTotal"| "300"| Minimum signal for line detection
"lostConfirmMs"| "20"| Line-loss confirmation time
"gapTimeMs"| "250"| Gap-handling duration
"searchTimeoutMs"| "2000"| Search timeout
"gapSpeed"| "150"| Speed during gap handling
"searchSpeed"| "220"| Speed during line search
"minSeparation"| "50"| Minimum calibration separation
"minValidSensors"| "8"| Required valid sensors
"calSamples"| "200"| Calibration samples

---

Movement Planner

The movement planner allows the robot to execute programmed actions.

Planner configuration is located in "Plan.h".

Each instruction follows this structure:

{ TRIGGER, VALUE, ACTION, PARAM, DURATION_MS }

Field| Description
"TRIGGER"| Condition that activates the step
"VALUE"| Trigger value
"ACTION"| Action to perform
"PARAM"| Parameter passed to the action
"DURATION_MS"| Action duration

Triggers

Trigger| Description
"AT_MS"| Executes when the elapsed run time reaches the specified millisecond value
"AT_MARKER"| Executes after the specified marker count is reached

Available Actions

Action| Description
"GO_STRAIGHT"| Drives both motors at the specified parameter for a duration
"TURN_LEFT"| Drives Motor B while Motor A is stopped
"TURN_RIGHT"| Drives Motor A while Motor B is stopped
"SET_SPEED"| Changes the PID base speed
"STOP"| Stops the robot and ends the run
"END"| Terminates the planner

---

Plan Example

Your plan is configured inside "Plan.h".

const Step plan[] = {
    // Trigger       Value  Action        Param  Duration
    { AT_MS,        1000,  GO_STRAIGHT, 180,   500 },
    { AT_MARKER,       1,  TURN_LEFT,   180,   300 },
    { AT_MARKER,       2,  SET_SPEED,   150,     0 },
    { AT_MS,        5000,  STOP,          0,      0 },
    { AT_MS,           0,  END,           0,      0 }
};

Plan steps execute sequentially.

The planner evaluates the current step and moves to the next step once the current instruction is completed.

«"SET_SPEED" changes the PID base speed. It is not a timed motor movement.»

Default behavior

The included "Plan.h" contains only an "END" instruction.

This means the robot defaults to ordinary PID line following without executing additional planned actions.

---

Function Reference

Main.ino

Function| Description
"readAllSensors()"| Reads all 16 sensor channels
"normalized(byte ch)"| Normalizes sensor readings from calibration data
"sampleSensors(int *target)"| Samples and averages sensor values
"calibrate()"| Performs sensor calibration
"followLine()"| Calculates line position and PID motor correction
"computePID(long error)"| Calculates PID output
"searchForLine(unsigned long lostFor)"| Handles line loss and recovery
"setMotorA(int speed)"| Sets Motor A PWM
"setMotorB(int speed)"| Sets Motor B PWM
"setMotorsPair(int left, int right)"| Sets both motor speeds
"stopMotors()"| Stops both motors
"handleButton()"| Handles start/stop button behavior
"selectMuxChannel(byte channel)"| Selects a multiplexer channel
"printSensors()"| Prints raw sensor values to Serial

Planner.cpp / Planner.h

Function| Description
"Planner(...)"| Initializes the planner
"reset()"| Resets planner execution
"update(int activeSensors)"| Updates marker counting and step execution
"baseSpeed()"| Returns current base speed
"stopRequested()"| Checks whether a stop was requested
"consumeStepDone()"| Retrieves and clears the step-completed state
"markers()"| Returns the marker counter

Buzzer Functions

Function| Description
"playStartupSong()"| Startup tone sequence
"playTapSong(bool starting)"| Start/stop feedback tones
"beep(int times)"| Repeats a beep
"errorBeep()"| Error tone

---

Debugging

Serial debugging is disabled by default.

To enable it, change the following line in "Main.ino":

const bool debugPrint = true;

The robot will print the raw values of its 16 sensors through Serial.

Sensor output is separated by tab characters, making it easier to monitor sensor behavior.

Remember to disable debugging when you're done.

---

Limitations

- Calibration is not permanently saved. It must be repeated after power cycling.
- The planner executes a predefined list of instructions.
- Marker detection is based on active sensor count, not complex pattern recognition.
- The default plan contains no movement instructions.
- Motor behavior depends on the physical wiring and motor orientation.
- The robot cannot autonomously map or solve a maze.

---

License

Distributed under the license included in this repository. See ""LICENSE"" (LICENSE) for details.

---

<p align="center">
  <i>Built to follow lines, not life advice.</i>
</p>
