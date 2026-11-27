#include "Planner.h"

Planner::Planner(const Step *steps, byte length, MotorFn motors, int defaultSpeed, int markerWidth)
  : _steps(steps),
    _length(length),
    _motors(motors),
    _defaultSpeed(defaultSpeed),
    _markerWidth(markerWidth),
    _startMs(0),
    _endMs(0),
    _index(0),
    _markerCount(0),
    _speed(defaultSpeed),
    _markerSeen(false),
    _active(false),
    _stop(false),
    _stepDone(false) {}

void Planner::reset() {
  _startMs = millis();
  _endMs = 0;
  _index = 0;
  _markerCount = 0;
  _speed = _defaultSpeed;
  _markerSeen = false;
  _active = false;
  _stop = false;
  _stepDone = false;
}

bool Planner::update(int activeSensors) {
  bool nowSeen = activeSensors >= _markerWidth;
  if (nowSeen && !_markerSeen) _markerCount++;
  _markerSeen = nowSeen;

  if (_index >= _length) return false;

  const Step &s = _steps[_index];

  if (s.action == END) return false;

  unsigned long now = millis();

  if (!_active) {
    bool fire = (s.trigger == AT_MS)
      ? (now - _startMs >= s.value)
      : (_markerCount >= (int)s.value);

    if (!fire) return false;

    if (s.action == SET_SPEED) {
      _speed = s.param;
      _index++;
      return false;
    }

    if (s.action == STOP) {
      _motors(0, 0);
      _stop = true;
      _index++;
      return true;
    }

    _active = true;
    _endMs = now + s.durationMs;
  }

  switch (s.action) {
    case GO_STRAIGHT:
      _motors(s.param, s.param);
      break;
    case TURN_LEFT:
      _motors(0, s.param);
      break;
    case TURN_RIGHT:
      _motors(s.param, 0);
      break;
    default:
      break;
  }

  if (now >= _endMs) {
    _active = false;
    _index++;
    _stepDone = true;
  }

  return true;
}

int Planner::baseSpeed() const {
  return _speed;
}

bool Planner::stopRequested() const {
  return _stop;
}

bool Planner::consumeStepDone() {
  bool d = _stepDone;
  _stepDone = false;
  return d;
}

int Planner::markers() const {
  return _markerCount;
}
