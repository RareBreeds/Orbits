#pragma once

// A rhythm has `length` steps, `hits` of which are on beats, spread as evenly as possible.
// Valid arguments: length from 1 to kOrbitsRhythmMaxLength, hits from 0 to length, and shift
// and step below length.

// The longest supported rhythm, in steps.
constexpr unsigned int kOrbitsRhythmMaxLength = 32;

// Whether `step` of the Euclidean rhythm, rotated by `shift` steps, is on a beat.
// `invert` swaps on and off beats.
bool orbitsEuclideanBeat(unsigned int length, unsigned int hits, unsigned int shift, unsigned int step,
                         bool invert = false);

// How many near-even variations there are of a rhythm with this length and number of hits.
// orbitsNearEvenBeat() takes a variation from 0 to this count - 1.
unsigned int orbitsNearEvenRhythmCount(unsigned int length, unsigned int hits);

// Whether `step` of a near-even rhythm variation, rotated by `shift` steps, is on a beat.
// Variation 0 is the Euclidean rhythm.
bool orbitsNearEvenBeat(unsigned int length, unsigned int hits, unsigned int variation, unsigned int shift,
                        unsigned int step);
