// IR line-sensor maths (logbook section 7, equation 32).
//
// Each emitter/receiver pair gives a value proportional to how much line
// is under it. The line's position is the weighted average
//     sum(value(i) * weight(i)) / sum(value(i))
// with the weights spread evenly from -1 (leftmost sensor) to +1
// (rightmost), so 0 means the line is centred.
//
// Plain C++ with no Arduino dependency, so it can be unit-tested on a PC.
#pragma once

// Rescale a raw reading to 0..1000 using calibration readings taken over
// the floor (offLine) and over the line (onLine). Works whichever of the two
// is larger, so it handles both "line reads high" and "line reads low" boards.
int normaliseReading(int raw, int offLine, int onLine);

// Weighted-average line position in [-1, 1] from numSensors normalised
// values (0..1000). Returns false, leaving position unchanged, when the
// total is below minTotal, i.e. no sensor can see the line.
bool linePosition(const int* values, int numSensors, float& position, long minTotal = 300);
