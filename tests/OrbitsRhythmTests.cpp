#include "doctest.h"

#include "OrbitsRhythm.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <vector>

// A rhythm as a string starting at step 0, 'x' for an on beat and '.' for an off beat.
static std::string nearEvenPattern(unsigned int length, unsigned int hits, unsigned int variation, unsigned int shift)
{
        std::string pattern;
        for(unsigned int step = 0; step < length; ++step)
        {
                pattern += orbitsNearEvenBeat(length, hits, variation, shift, step) ? 'x' : '.';
        }
        return pattern;
}

static std::string euclideanPattern(unsigned int length, unsigned int hits, unsigned int shift, bool invert)
{
        std::string pattern;
        for(unsigned int step = 0; step < length; ++step)
        {
                pattern += orbitsEuclideanBeat(length, hits, shift, step, invert) ? 'x' : '.';
        }
        return pattern;
}

static unsigned int countHits(const std::string &pattern)
{
        return static_cast<unsigned int>(std::count(pattern.begin(), pattern.end(), 'x'));
}

TEST_CASE("Every length and number of hits has at least one near-even variation")
{
        // Polygene picks a variation from 0 to count - 1, so a count of 0 would underflow.
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        CAPTURE(length);
                        CAPTURE(hits);
                        CHECK(orbitsNearEvenRhythmCount(length, hits) >= 1);
                }
        }
}

TEST_CASE("Near-even rhythms have the requested number of hits at every shift")
{
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        const unsigned int count = orbitsNearEvenRhythmCount(length, hits);
                        for(unsigned int variation = 0; variation < count; ++variation)
                        {
                                for(unsigned int shift = 0; shift < length; ++shift)
                                {
                                        CAPTURE(length);
                                        CAPTURE(hits);
                                        CAPTURE(variation);
                                        CAPTURE(shift);
                                        CHECK(countHits(nearEvenPattern(length, hits, variation, shift)) == hits);
                                }
                        }
                }
        }
}

TEST_CASE("Near-even variations are all different")
{
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        const unsigned int count = orbitsNearEvenRhythmCount(length, hits);
                        std::set<std::string> patterns;
                        for(unsigned int variation = 0; variation < count; ++variation)
                        {
                                patterns.insert(nearEvenPattern(length, hits, variation, 0));
                        }

                        CAPTURE(length);
                        CAPTURE(hits);
                        CHECK(patterns.size() == count);
                }
        }
}

TEST_CASE("Shift moves every beat later by that many steps, wrapping at the length")
{
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        const unsigned int count = orbitsNearEvenRhythmCount(length, hits);
                        for(unsigned int variation = 0; variation < count; ++variation)
                        {
                                const std::string unshifted = nearEvenPattern(length, hits, variation, 0);
                                for(unsigned int shift = 0; shift < length; ++shift)
                                {
                                        // Rotate right: the last `shift` steps move to the start.
                                        std::string expected = unshifted;
                                        std::rotate(expected.begin(), expected.end() - shift, expected.end());

                                        CAPTURE(length);
                                        CAPTURE(hits);
                                        CAPTURE(variation);
                                        CAPTURE(shift);
                                        CHECK(nearEvenPattern(length, hits, variation, shift) == expected);
                                }
                        }
                }
        }
}

TEST_CASE("Euclidean rhythms are near-even variation 0")
{
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        for(unsigned int shift = 0; shift < length; ++shift)
                        {
                                CAPTURE(length);
                                CAPTURE(hits);
                                CAPTURE(shift);
                                CHECK(euclideanPattern(length, hits, shift, false) ==
                                      nearEvenPattern(length, hits, 0, shift));
                        }
                }
        }
}

TEST_CASE("Invert swaps on and off beats")
{
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 0; hits <= length; ++hits)
                {
                        for(unsigned int shift = 0; shift < length; ++shift)
                        {
                                std::string expected = euclideanPattern(length, hits, shift, false);
                                for(char &step : expected)
                                {
                                        step = step == 'x' ? '.' : 'x';
                                }

                                CAPTURE(length);
                                CAPTURE(hits);
                                CAPTURE(shift);
                                CHECK(euclideanPattern(length, hits, shift, true) == expected);
                        }
                }
        }
}

TEST_CASE("Euclidean rhythms are maximally even")
{
        // The gaps between consecutive hits, wrapping around, differ by at most one step.
        for(unsigned int length = 1; length <= kOrbitsRhythmMaxLength; ++length)
        {
                for(unsigned int hits = 2; hits <= length; ++hits)
                {
                        const std::string pattern = euclideanPattern(length, hits, 0, false);
                        std::vector<unsigned int> onsets;
                        for(unsigned int step = 0; step < length; ++step)
                        {
                                if(pattern[step] == 'x')
                                {
                                        onsets.push_back(step);
                                }
                        }

                        unsigned int smallest_gap = length;
                        unsigned int largest_gap = 0;
                        for(size_t i = 0; i < onsets.size(); ++i)
                        {
                                const unsigned int next = onsets[(i + 1) % onsets.size()];
                                const unsigned int gap = (next + length - onsets[i] - 1) % length + 1;
                                smallest_gap = std::min(smallest_gap, gap);
                                largest_gap = std::max(largest_gap, gap);
                        }

                        CAPTURE(length);
                        CAPTURE(hits);
                        CAPTURE(pattern);
                        CHECK(largest_gap - smallest_gap <= 1);
                }
        }
}

TEST_CASE("Known rhythms")
{
        // Exact patterns, so a change to the tables or the indexing that would change how
        // existing patches sound is caught.
        CHECK(euclideanPattern(8, 3, 0, false) == "x.x..x..");
        CHECK(euclideanPattern(16, 5, 0, false) == "x..x..x..x..x...");
        CHECK(euclideanPattern(12, 7, 0, false) == "xx.x.x.xx.x.");
        CHECK(euclideanPattern(4, 0, 0, false) == "....");
        CHECK(euclideanPattern(4, 4, 0, false) == "xxxx");
        CHECK(euclideanPattern(1, 1, 0, false) == "x");

        CHECK(euclideanPattern(8, 3, 1, false) == ".x.x..x.");
        CHECK(euclideanPattern(8, 3, 7, false) == ".x..x..x");
        CHECK(euclideanPattern(8, 3, 0, true) == ".x.xx.xx");

        CHECK(orbitsNearEvenRhythmCount(8, 3) == 7);
        CHECK(nearEvenPattern(8, 3, 1, 0) == "xx...x..");
        CHECK(nearEvenPattern(8, 3, 6, 0) == "x.....xx");
}
