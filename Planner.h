#ifndef PLANNER_H
#define PLANNER_H

#include <Arduino.h>

enum Trigger : byte { AT_MS, AT_MARKER };
enum Action : byte { GO_STRAIGHT, TURN_LEFT, TURN_RIGHT, SET_SPEED, STOP, END };

struct Step {
  Trigger trigger;
  unsigned long value;
  Action action;
  int param;
  unsigned long durationMs;
};

typedef void (*MotorFn)(int left, int right);

class Planner {
public:
  Planner(const Step *steps, byte length, MotorFn motors, int defaultSpeed, int markerWidth = 8);

  void reset();
  bool update(int activeSensors);

  int baseSpeed() const;
  bool stopRequested() const;
  bool consumeStepDone();
  int markers() const;

private:
  const Step *_steps;
  byte _length;
  MotorFn _motors;
  int _defaultSpeed;
  int _markerWidth;

  unsigned long _startMs;
  unsigned long _endMs;
  byte _index;
  int _markerCount;
  int _speed;
  bool _markerSeen;
  bool _active;
  bool _stop;
  bool _stepDone;
};

#endif
