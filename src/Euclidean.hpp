#pragma once

#include <bitset>

namespace euclidean {

static const auto max_length = 32u;

typedef std::bitset<max_length> Rhythm;

Rhythm rhythm(unsigned int length, unsigned int on_beats);
Rhythm rotate(Rhythm val, uint32_t size, uint32_t amount);

}