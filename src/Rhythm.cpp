#include <cassert>

#include "Rhythm.hpp"
#include "rhythm_table.h"
namespace rhythm
{

static uint32_t sumTo(uint32_t n)
{
        return n * (n + 1) / 2;
}

Rhythm rhythm(uint32_t length, uint32_t on_beats)
{
        return nearEvenRhythm(length, on_beats, 0);
}

Rhythm rhythm(uint32_t length, uint32_t on_beats, uint32_t shift)
{
        return rotate(rhythm(length, on_beats), length, shift);
}

Rhythm rotate(Rhythm val, uint32_t size, uint32_t amount)
{
        return (val << amount | val >> (size - amount));
}

bool beat(uint32_t length, uint32_t on_beats, uint32_t shift, uint32_t num)
{
        return rhythm(length, on_beats, shift)[num];
}

uint32_t numNearEvenRhythms(uint32_t length, uint32_t density)
{
        uint32_t off = (sumTo(length) - 1) + density;
        return neareven_count[off];
}

Rhythm nearEvenRhythm(uint32_t length, uint32_t density, uint32_t variation)
{
        uint32_t off = (sumTo(length) - 1) + density;
        return neareven_rhythms[neareven_offsets[off] + variation];
}

bool nearEvenRhythmBeat(uint32_t length, uint32_t on_beats, uint32_t variation, uint32_t shift, uint32_t num)
{
        return rotate(nearEvenRhythm(length, on_beats, variation), length, shift)[num];
}

} // namespace rhythm
