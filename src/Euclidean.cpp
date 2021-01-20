#include <cassert>

#include "Euclidean.hpp"

namespace euclidean
{

#if 0
Rhythm rhythm(unsigned int length_of_sequence, unsigned int number_of_hits)
{
	assert(number_of_hits <= length_of_sequence);

	if(0 == number_of_hits)
	{
		return Rhythm();
	}

	if(number_of_hits == length_of_sequence)
	{
		return ~Rhythm();
	}

	auto left_blocks = number_of_hits;
	Rhythm left_pattern;
	left_pattern[0] = true;
	unsigned int left_pattern_length = 1;

	auto right_blocks = length_of_sequence - left_blocks;
	Rhythm right_pattern;
	right_pattern[0] = false;
	unsigned int right_pattern_length = 1;

	while(right_blocks)
	{
		unsigned int remainder_blocks;
		Rhythm remainder_pattern;
		unsigned int remainder_pattern_length;

		if(left_blocks > right_blocks)
		{
			remainder_blocks = left_blocks - right_blocks;
			remainder_pattern = left_pattern;
			remainder_pattern_length = left_pattern_length;
			left_blocks = right_blocks;
		}
		else
		{
			remainder_blocks = right_blocks - left_blocks;
			remainder_pattern = right_pattern;
			remainder_pattern_length = right_pattern_length;
		}
		for(unsigned int i = 0; i < right_pattern_length; ++i)
		{
			left_pattern[left_pattern_length] = right_pattern[i];
			++left_pattern_length;
		}
		right_blocks = remainder_blocks;
		right_pattern = remainder_pattern;
		right_pattern_length = remainder_pattern_length;
	}

	for(unsigned int i = 1; i < left_blocks; ++i)
	{
		for(unsigned int j = 0; j < left_pattern_length; ++j)
		{
			left_pattern[left_pattern_length * i + j] = left_pattern[j];
		}
	}

	assert(left_pattern_length * left_blocks == length_of_sequence);
	assert(left_pattern.count() == number_of_hits);
	return left_pattern;
}
#endif

// clang-format off
// When the number of hits is 0 or 1 the rhythm is always 0x0 or 0x1
// respectively, no need to store them in the look up table.
// When the number of hits is equal to the length we know all beats
// are on, so also no need to store them in the look up table.
static const uint8_t euclidean_rhythm_u8[] =
{
        // Y = Beat Length - 3
        // X = Number of on-beats - 2
        /* 3 */ 0x5u,
        /* 4 */ 0x5u,  0xdu,
        /* 5 */ 0x9u,  0xdu,  0x1du,
        /* 6 */ 0x9u,  0x15u, 0x2du, 0x3du,
        /* 7 */ 0x11u, 0x29u, 0x2du, 0x5du, 0x7du,
        /* 8 */ 0x11u, 0x29u, 0x55u, 0xadu, 0xddu, 0xfdu
};

static const uint16_t euclidean_rhythm_u16[] =
{
        // Y = Beat Length - 9
        // X = Number of on-beats - 2
        /* 9  */ 0x21u,  0x49u,  0xa9u,   0xadu,   0x16du,  0x1bdu,  0x1fdu,
        /* 10 */ 0x21u,  0x91u,  0x129u,  0x155u,  0x1adu,  0x2ddu,  0x3bdu,  0x3fdu,
        /* 11 */ 0x41u,  0x91u,  0x129u,  0x2a9u,  0x2adu,  0x5adu,  0x6ddu,  0x77du,  0x7fdu,
        /* 12 */ 0x41u,  0x111u, 0x249u,  0x4a9u,  0x555u,  0x6adu,  0xb6du,  0xdddu,  0xf7du,  0xffdu,
        /* 13 */ 0x81u,  0x221u, 0x491u,  0x929u,  0xaa9u,  0xaadu,  0xdadu,  0x16ddu, 0x1bbdu, 0x1efdu, 0x1ffdu,
        /* 14 */ 0x81u,  0x221u, 0x891u,  0x929u,  0x14a9u, 0x1555u, 0x16adu, 0x2dadu, 0x2eddu, 0x3bbdu, 0x3efdu, 0x3ffdu,
        /* 15 */ 0x101u, 0x421u, 0x891u,  0x1249u, 0x2529u, 0x2aa9u, 0x2aadu, 0x35adu, 0x5b6du, 0x6eddu, 0x77bdu, 0x7dfdu, 0x7ffdu,
        /* 16 */ 0x101u, 0x841u, 0x1111u, 0x2491u, 0x2929u, 0x52a9u, 0x5555u, 0x5aadu, 0xadadu, 0xb6ddu, 0xddddu, 0xef7du, 0xfdfdu, 0xfffdu
};

static const uint32_t euclidean_rhythm_u32[] =
{
        // Y = Beat Length - 17
        // X = Number of on-beats - 2
        /* 17 */ 0x201u,   0x841u,    0x2221u,    0x4491u,    0x4929u,    0x94a9u,     0xaaa9u,     0xaaadu,     0xd6adu,     0x16dadu,    0x176ddu,    0x1bbbdu,    0x1ef7du,    0x1fbfdu,    0x1fffdu,
        /* 18 */ 0x201u,   0x1041u,   0x4221u,    0x8891u,    0x9249u,    0x12929u,    0x152a9u,    0x15555u,    0x15aadu,    0x1adadu,    0x2db6du,    0x2eeddu,    0x37bbdu,    0x3df7du,    0x3fbfdu,    0x3fffdu,
        /* 19 */ 0x401u,   0x2081u,   0x4221u,    0x8891u,    0x12491u,   0x14929u,    0x294a9u,    0x2aaa9u,    0x2aaadu,    0x2d6adu,    0x56dadu,    0x5b6ddu,    0x6eeddu,    0x77bbdu,    0x7befdu,    0x7f7fdu,    0x7fffdu,
        /* 20 */ 0x401u,   0x2081u,   0x8421u,    0x11111u,   0x24491u,   0x24929u,    0x4a529u,    0x54aa9u,    0x55555u,    0x56aadu,    0x6b5adu,    0xb6dadu,    0xb76ddu,    0xdddddu,    0xef7bdu,    0xfbefdu,    0xff7fdu,    0xffffdu,
        /* 21 */ 0x801u,   0x4081u,   0x10841u,   0x22221u,   0x44891u,   0x49249u,    0x52929u,    0xa54a9u,    0xaaaa9u,    0xaaaadu,    0xb56adu,    0x15adadu,   0x16db6du,   0x176eddu,   0x1bbbbdu,   0x1def7du,   0x1f7efdu,   0x1feffdu,   0x1ffffdu,
        /* 22 */ 0x801u,   0x8101u,   0x20841u,   0x42221u,   0x48891u,   0x92491u,    0x94929u,    0x1294a9u,   0x154aa9u,   0x155555u,   0x156aadu,   0x1ad6adu,   0x2d6dadu,   0x2db6ddu,   0x36eeddu,   0x37bbbdu,   0x3bef7du,   0x3efdfdu,   0x3feffdu,   0x3ffffdu,
        /* 23 */ 0x1001u,  0x8101u,   0x20841u,   0x84221u,   0x88891u,   0x122491u,   0x124929u,   0x252929u,   0x2952a9u,   0x2aaaa9u,   0x2aaaadu,   0x2d5aadu,   0x35adadu,   0x5b6dadu,   0x5bb6ddu,   0x6eeeddu,   0x6f7bbdu,   0x7bef7du,   0x7efdfdu,   0x7fdffdu,   0x7ffffdu,
        /* 24 */ 0x1001u,  0x10101u,  0x41041u,   0x84221u,   0x111111u,  0x224491u,   0x249249u,   0x292929u,   0x4a94a9u,   0x552aa9u,   0x555555u,   0x55aaadu,   0x6ad6adu,   0xadadadu,   0xb6db6du,   0xbb76ddu,   0xddddddu,   0xef7bbdu,   0xf7df7du,   0xfdfdfdu,   0xffdffdu,   0xfffffdu,
        /* 25 */ 0x2001u,  0x20201u,  0x82081u,   0x108421u,  0x222221u,  0x448891u,   0x492491u,   0x4a4929u,   0x94a529u,   0xa952a9u,   0xaaaaa9u,   0xaaaaadu,   0xad5aadu,   0xd6b5adu,   0x16b6dadu,  0x16db6ddu,  0x176eeddu,  0x1bbbbbdu,  0x1def7bdu,  0x1efbefdu,  0x1fbfbfdu,  0x1ffbffdu,  0x1fffffdu,
        /* 26 */ 0x2001u,  0x20201u,  0x102081u,  0x210841u,  0x442221u,  0x488891u,   0x922491u,   0x924929u,   0x1252929u,  0x14a94a9u,  0x1552aa9u,  0x1555555u,  0x155aaadu,  0x16ad6adu,  0x1b5adadu,  0x2db6dadu,  0x2dbb6ddu,  0x36eeeddu,  0x377bbbdu,  0x3bdef7du,  0x3dfbefdu,  0x3fbfbfdu,  0x3ffbffdu,  0x3fffffdu,
        /* 27 */ 0x4001u,  0x40201u,  0x102081u,  0x410841u,  0x844221u,  0x888891u,   0x1224491u,  0x1249249u,  0x1494929u,  0x25294a9u,  0x2a552a9u,  0x2aaaaa9u,  0x2aaaaadu,  0x2b55aadu,  0x35ad6adu,  0x56d6dadu,  0x5b6db6du,  0x5bb76ddu,  0x6eeeeddu,  0x6f77bbdu,  0x77def7du,  0x7dfbefdu,  0x7f7fbfdu,  0x7ff7ffdu,  0x7fffffdu,
        /* 28 */ 0x4001u,  0x80401u,  0x204081u,  0x820841u,  0x884221u,  0x1111111u,  0x2244891u,  0x2492491u,  0x24a4929u,  0x4a52929u,  0x52a54a9u,  0x554aaa9u,  0x5555555u,  0x556aaadu,  0x5ab56adu,  0x6b5adadu,  0xb6b6dadu,  0xb6db6ddu,  0xbb76eddu,  0xdddddddu,  0xeef7bbdu,  0xefbef7du,  0xfbf7efdu,  0xfeff7fdu,  0xfff7ffdu,  0xffffffdu,
        /* 29 */ 0x8001u,  0x80401u,  0x408101u,  0x820841u,  0x1084221u, 0x2222221u,  0x2448891u,  0x4912491u,  0x4924929u,  0x5252929u,  0x95294a9u,  0xa954aa9u,  0xaaaaaa9u,  0xaaaaaadu,  0xad56aadu,  0xd5ad6adu,  0x15b5adadu, 0x16db6dadu, 0x16ddb6ddu, 0x1b76eeddu, 0x1bbbbbbdu, 0x1def7bbdu, 0x1efbef7du, 0x1f7efdfdu, 0x1feff7fdu, 0x1ffefffdu, 0x1ffffffdu,
        /* 30 */ 0x8001u,  0x100401u, 0x808101u,  0x1041041u, 0x2108421u, 0x4422221u,  0x4488891u,  0x9124491u,  0x9249249u,  0x9494929u,  0x1294a529u, 0x14a952a9u, 0x1554aaa9u, 0x15555555u, 0x1556aaadu, 0x16ad5aadu, 0x1ad6b5adu, 0x2d6d6dadu, 0x2db6db6du, 0x2ddb76ddu, 0x376eeeddu, 0x377bbbbdu, 0x3bdef7bdu, 0x3df7df7du, 0x3efefdfdu, 0x3fdff7fdu, 0x3ffefffdu, 0x3ffffffdu,
        /* 31 */ 0x10001u, 0x200801u, 0x808101u,  0x2082081u, 0x4210841u, 0x8442221u,  0x8888891u,  0x11224491u, 0x12492491u, 0x12524929u, 0x24a52929u, 0x254a94a9u, 0x2a954aa9u, 0x2aaaaaa9u, 0x2aaaaaadu, 0x2ad56aadu, 0x356ad6adu, 0x36b5adadu, 0x5b5b6dadu, 0x5b6db6ddu, 0x5dbb76ddu, 0x6eeeeeddu, 0x6f77bbbdu, 0x77bdef7du, 0x7befbefdu, 0x7efefdfdu, 0x7fbfeffdu, 0x7ffdfffdu, 0x7ffffffdu,
        /* 32 */ 0x10001u, 0x200801u, 0x1010101u, 0x4082081u, 0x8410841u, 0x10884221u, 0x11111111u, 0x22448891u, 0x24912491u, 0x24924929u, 0x29292929u, 0x4a5294a9u, 0x52a952a9u, 0x5552aaa9u, 0x55555555u, 0x555aaaadu, 0x5aad5aadu, 0x6b5ad6adu, 0xadadadadu, 0xb6db6dadu, 0xb6ddb6ddu, 0xbb76eeddu, 0xddddddddu, 0xdeef7bbdu, 0xef7def7du, 0xf7efbefdu, 0xfdfdfdfdu, 0xffbfeffdu, 0xfffdfffdu, 0xfffffffdu
};
// clang-format on

static uint32_t sumTo(unsigned int n)
{
        return n * (n + 1) / 2;
}

static unsigned int euclideanRhythmIndex(unsigned int length, unsigned int on_beats_m2, unsigned int first_entry_length)
{
        unsigned int first_entry_elements = first_entry_length - 2;
        unsigned int length_off = length - first_entry_length;
        return length_off * first_entry_elements + sumTo(length_off - 1) + on_beats_m2;
}

Rhythm rhythm(unsigned int length, unsigned int on_beats)
{
#if ORBITS_DEBUG
        assert(length >= 1);
        assert(length <= 32);
        assert(on_beats <= length);
#endif // ORBITS_DEBUG

        // All euclidean rhythms with on beats of 0 or 1 are just 0 or 1 respectively.
        if(on_beats <= 1)
        {
                return on_beats;
        }

        // All beats are on beats when on_beats == length.
        if(on_beats == length)
        {
                // Technically the top bits should be 0's as they are not on beats.
                // i.e. return (~0U) >> (32 - length);
                // But we don't access bits outside the length of the rhythm so we can set them to whatever we want.
                return ~0;
        }

        // On beats 0 and 1 aren't stored in the look up table.
        unsigned int on_beats_m2 = on_beats - 2;
        // Rhythms are stored in the smallest native type available.
        if(length <= 8)
        {
                // Rhythm lengths 0, 1 and 2 aren't stored in the look up table.
                unsigned int length_m3 = length - 3;

                // Index for this length starts at the sum of lengths up to this
                // number, then add the number of on beats.
                unsigned int index = sumTo(length_m3) + on_beats_m2;

                return (uint32_t)euclidean_rhythm_u8[index];
        }
        else if(length <= 16)
        {
                // Rhythm lengths less than 9 aren't stored in the look up table.
                unsigned int index = euclideanRhythmIndex(length, on_beats_m2, 9);
                return (uint32_t)euclidean_rhythm_u16[index];
        }
        else // length <= 32
        {
                // Rhythm lengths less than 17 aren't stored in the look up table.
                unsigned int index = euclideanRhythmIndex(length, on_beats_m2, 17);
                return euclidean_rhythm_u32[index];
        }
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

} // namespace euclidean
