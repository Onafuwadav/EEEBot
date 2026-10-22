#include "LineSensor.h"

int normaliseReading(int raw, int offLine, int onLine) {
  long span = (long)onLine - offLine;
  if (span == 0) return 0;
  long scaled = ((long)raw - offLine) * 1000 / span;
  if (scaled < 0) return 0;
  if (scaled > 1000) return 1000;
  return (int)scaled;
}

bool linePosition(const int* values, int numSensors, float& position, long minTotal) {
  if (numSensors < 2) return false;

  long total = 0;
  float weighted = 0.0f;
  for (int i = 0; i < numSensors; i++) {
    float weight = -1.0f + 2.0f * i / (numSensors - 1);  // -1 .. +1
    total += values[i];
    weighted += values[i] * weight;
  }
  if (total < minTotal) return false;

  position = weighted / total;
  return true;
}
