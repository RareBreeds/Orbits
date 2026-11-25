#include "doctest.h"

#include "OrbitsStepCounter.hpp"

#include <string>

// The steps played by `clocks` clocks, one digit per step, e.g. "0123".
static std::string playSteps(OrbitsStepCounter &counter, unsigned int length, bool reverse, unsigned int clocks)
{
        std::string steps;
        for(unsigned int clock = 0; clock < clocks; ++clock)
        {
                steps += std::to_string(counter.advance(length, reverse).index);
        }
        return steps;
}

TEST_CASE("Forwards plays every step in order, starting from step 0")
{
        OrbitsStepCounter counter;
        CHECK(playSteps(counter, 4, false, 8) == "01230123");
}

TEST_CASE("Backwards plays every step in reverse order, starting from the last step")
{
        OrbitsStepCounter counter;
        CHECK(playSteps(counter, 4, true, 8) == "32103210");
}

TEST_CASE("First and last are the ends of the cycle in the direction of play")
{
        SUBCASE("Forwards")
        {
                OrbitsStepCounter counter;
                for(unsigned int index = 0; index < 4; ++index)
                {
                        const OrbitsStepCounter::Step step = counter.advance(4, false);
                        CHECK(step.is_first == (step.index == 0));
                        CHECK(step.is_last == (step.index == 3));
                }
        }
        SUBCASE("Backwards")
        {
                OrbitsStepCounter counter;
                for(unsigned int index = 0; index < 4; ++index)
                {
                        const OrbitsStepCounter::Step step = counter.advance(4, true);
                        CHECK(step.is_first == (step.index == 3));
                        CHECK(step.is_last == (step.index == 0));
                }
        }
}

TEST_CASE("A rhythm of one step plays step 0, which is both first and last")
{
        for(bool reverse : {false, true})
        {
                OrbitsStepCounter counter;
                for(int clock = 0; clock < 3; ++clock)
                {
                        const OrbitsStepCounter::Step step = counter.advance(1, reverse);
                        CAPTURE(reverse);
                        CHECK(step.index == 0);
                        CHECK(step.is_first);
                        CHECK(step.is_last);
                }
        }
}

TEST_CASE("A position beyond a shorter new length starts again from step 0")
{
        SUBCASE("Forwards")
        {
                OrbitsStepCounter counter;
                CHECK(playSteps(counter, 8, false, 6) == "012345");
                CHECK(playSteps(counter, 4, false, 3) == "012");
        }
        SUBCASE("Backwards")
        {
                OrbitsStepCounter counter;
                CHECK(playSteps(counter, 8, false, 6) == "012345");
                CHECK(playSteps(counter, 4, true, 3) == "321");
        }
}

TEST_CASE("Switching direction plays the most recent step again")
{
        OrbitsStepCounter counter;
        CHECK(playSteps(counter, 4, false, 3) == "012");
        CHECK(playSteps(counter, 4, true, 3) == "210");
        CHECK(playSteps(counter, 4, false, 3) == "012");
}

TEST_CASE("Reset starts the rhythm again")
{
        OrbitsStepCounter forwards;
        CHECK(playSteps(forwards, 4, false, 2) == "01");
        forwards.reset();
        CHECK(playSteps(forwards, 4, false, 2) == "01");

        OrbitsStepCounter backwards;
        CHECK(playSteps(backwards, 4, true, 2) == "32");
        backwards.reset();
        CHECK(playSteps(backwards, 4, true, 2) == "32");
}

TEST_CASE("The displayed position is the next step forwards, or 0 if it's beyond the length")
{
        OrbitsStepCounter counter;
        CHECK(counter.position(4) == 0);
        counter.advance(4, false);
        counter.advance(4, false);
        counter.advance(4, false);
        CHECK(counter.position(4) == 3);
        CHECK(counter.position(2) == 0);
}
