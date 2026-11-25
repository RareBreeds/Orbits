#include "OrbitsRhythm.hpp"
#include "OrbitsRhythmTable.h"

#include <bitset>

// One bit per step, step 0 is the lowest bit.
typedef std::bitset<kOrbitsRhythmMaxLength> OrbitsRhythm;

static unsigned int sumTo(unsigned int n)
{
        return n * (n + 1) / 2;
}

// Index into near_even_counts and near_even_offsets. The tables hold an entry for every
// number of hits from 0 to length, for each length from 1 upwards.
static unsigned int tableIndex(unsigned int length, unsigned int hits)
{
        return (sumTo(length) - 1) + hits;
}

static OrbitsRhythm nearEvenRhythm(unsigned int length, unsigned int hits, unsigned int variation)
{
        return near_even_rhythms[near_even_offsets[tableIndex(length, hits)] + variation];
}

unsigned int orbitsNearEvenRhythmCount(unsigned int length, unsigned int hits)
{
        return near_even_counts[tableIndex(length, hits)];
}

bool orbitsNearEvenBeat(unsigned int length, unsigned int hits, unsigned int variation, unsigned int shift,
                        unsigned int step)
{
        const unsigned int unrotated_step = step >= shift ? step - shift : step + length - shift;
        return nearEvenRhythm(length, hits, variation)[unrotated_step];
}

bool orbitsEuclideanBeat(unsigned int length, unsigned int hits, unsigned int shift, unsigned int step,
                         bool invert)
{
        return orbitsNearEvenBeat(length, hits, 0, shift, step) != invert;
}
