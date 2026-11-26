#include "doctest.h"

#include "EugeneModule.hpp"
#include "RackTestHelpers.hpp"

typedef RareBreeds_Orbits_Eugene Eugene;

// Sets the knobs for a rhythm of `length` steps with `hits` on beats.
static void setRhythm(Eugene &eugene, unsigned int length, unsigned int hits, unsigned int shift = 0)
{
        eugene.getParam(Eugene::LENGTH_KNOB_PARAM).setValue(static_cast<float>(length));
        eugene.getParam(Eugene::HITS_KNOB_PARAM).setValue(static_cast<float>(hits) / static_cast<float>(length));
        eugene.getParam(Eugene::SHIFT_KNOB_PARAM).setValue(static_cast<float>(shift));
}

static ClockedOutputs sendClocks(Eugene &eugene, unsigned int clocks)
{
        return sendClocks(eugene, Eugene::CLOCK_INPUT, Eugene::BEAT_OUTPUT, Eugene::EOC_OUTPUT, 1, clocks);
}

TEST_CASE("Eugene plays its rhythm on the beat output, starting from the first step")
{
        Eugene eugene;
        setRhythm(eugene, 8, 3);
        CHECK(sendClocks(eugene, 16).beat[0] == "x.x..x..x.x..x..");
}

TEST_CASE("Eugene's shift rotates the rhythm later")
{
        Eugene eugene;
        setRhythm(eugene, 8, 3, 1);
        CHECK(sendClocks(eugene, 8).beat[0] == ".x.x..x.");
}

TEST_CASE("Eugene's reverse plays the rhythm backwards from the last step")
{
        Eugene eugene;
        setRhythm(eugene, 8, 3);
        eugene.getParam(Eugene::REVERSE_KNOB_PARAM).setValue(1.f);
        CHECK(sendClocks(eugene, 16).beat[0] == "..x..x.x..x..x.x");
}

TEST_CASE("Eugene's invert swaps on and off beats")
{
        Eugene eugene;
        setRhythm(eugene, 8, 3);
        eugene.getParam(Eugene::INVERT_KNOB_PARAM).setValue(1.f);
        CHECK(sendClocks(eugene, 8).beat[0] == ".x.xx.xx");
}

TEST_CASE("Eugene's end of cycle output follows its EOC mode")
{
        Eugene eugene;
        setRhythm(eugene, 8, 3);

        SUBCASE("On Repeat, the default, fires as each repeat starts")
        {
                CHECK(sendClocks(eugene, 24).eoc[0] == "........x.......x.......");
        }
        SUBCASE("First fires on every first step")
        {
                eugene.m_eoc.setMode(1);
                CHECK(sendClocks(eugene, 16).eoc[0] == "x.......x.......");
        }
        SUBCASE("Last fires on every last step")
        {
                eugene.m_eoc.setMode(2);
                CHECK(sendClocks(eugene, 16).eoc[0] == ".......x.......x");
        }
}

TEST_CASE("Loading Eugene's widget config twice before the widget draws doesn't leak it")
{
        // dataFromJson() keeps the "widget" object for the widget to apply when it next draws.
        // Loading a second patch before then used to leak the first one's.
        json_t *first = json_object();
        json_t *second = json_object();
        json_t *first_patch = json_object();
        json_t *second_patch = json_object();
        json_object_set(first_patch, "widget", first);
        json_object_set(second_patch, "widget", second);

        {
                Eugene eugene;
                eugene.dataFromJson(first_patch);
                eugene.dataFromJson(second_patch);
        }
        json_decref(first_patch);
        json_decref(second_patch);

        CHECK(referenceCount(first) == 1);
        CHECK(referenceCount(second) == 1);
        json_decref(first);
        json_decref(second);
}

TEST_CASE("Every Eugene param is configured, and Sync keeps the id that patches saved it with")
{
        // Rack labels an unconfigured param "#<id + 1>", which shows up in tools that list every param
        Eugene eugene;
        for(int id = 0; id < Eugene::NUM_PARAMS; ++id)
        {
                CAPTURE(id);
                CHECK_FALSE(eugene.getParamQuantity(id)->name.empty());
        }
        CHECK(Eugene::SYNC_KNOB_PARAM == 9);
        CHECK_FALSE(eugene.getParamQuantity(Eugene::UNUSED_PARAM)->randomizeEnabled);
}

TEST_CASE("Eugene's Hits shows the number of hits played, and takes a typed number of hits")
{
        Eugene eugene;
        APP->engine->addModule(&eugene);
        setRhythm(eugene, 16, 5);
        ParamQuantity *hits = eugene.getParamQuantity(Eugene::HITS_KNOB_PARAM);
        CHECK(hits->getString() == "Hits: 5 / 16");

        SUBCASE("A number of hits")
        {
                hits->setDisplayValueString("7");
                CHECK(eugene.readHits(16) == 7);
        }
        SUBCASE("A percentage")
        {
                hits->setDisplayValueString("50%");
                CHECK(hits->getValue() == 0.5f);
        }
        SUBCASE("The text shown, which the text field starts with, keeps the number of hits")
        {
                eugene.getParam(Eugene::HITS_KNOB_PARAM).setValue(0.33f);
                hits->setDisplayValueString(hits->getDisplayValueString());
                CHECK(eugene.readHits(16) == 5);
        }
        SUBCASE("Including Hits CV")
        {
                connectInput(eugene.getInput(Eugene::HITS_CV_INPUT), 1);
                eugene.getInput(Eugene::HITS_CV_INPUT).setVoltage(5.f);
                eugene.getParam(Eugene::HITS_CV_KNOB_PARAM).setValue(0.25f);
                CHECK(hits->getString() == "Hits: 9 / 16");
                hits->setDisplayValueString("12");
                CHECK(eugene.readHits(16) == 12);
        }

        APP->engine->removeModule(&eugene);
}

TEST_CASE("Eugene's Length and Shift show what's played, and take a typed value")
{
        Eugene eugene;
        APP->engine->addModule(&eugene);
        ParamQuantity *length = eugene.getParamQuantity(Eugene::LENGTH_KNOB_PARAM);
        ParamQuantity *shift = eugene.getParamQuantity(Eugene::SHIFT_KNOB_PARAM);

        SUBCASE("A randomized length shows the whole number played")
        {
                eugene.getParam(Eugene::LENGTH_KNOB_PARAM).setValue(17.3f);
                CHECK(length->getString() == "Length: 17");
        }
        SUBCASE("Including Length CV")
        {
                eugene.getParam(Eugene::LENGTH_KNOB_PARAM).setValue(17.f);
                connectInput(eugene.getInput(Eugene::LENGTH_CV_INPUT), 1);
                eugene.getInput(Eugene::LENGTH_CV_INPUT).setVoltage(5.f);
                eugene.getParam(Eugene::LENGTH_CV_KNOB_PARAM).setValue(0.2f);
                CHECK(length->getString() == "Length: 23");
                length->setDisplayValueString("20");
                CHECK(eugene.readLength() == 20);
        }
        SUBCASE("Shift wraps at the length")
        {
                setRhythm(eugene, 16, 5, 20);
                CHECK(shift->getString() == "Shift: 4");
                shift->setDisplayValueString("3");
                CHECK(eugene.readShift(16) == 3);
        }

        APP->engine->removeModule(&eugene);
}

TEST_CASE("Eugene's CV knobs show percentages")
{
        Eugene eugene;
        eugene.getParam(Eugene::LENGTH_CV_KNOB_PARAM).setValue(0.5f);
        CHECK(eugene.getParamQuantity(Eugene::LENGTH_CV_KNOB_PARAM)->getString() == "Length CV: 50%");
        CHECK(eugene.getParamQuantity(Eugene::HITS_CV_KNOB_PARAM)->getString() == "Hits CV: 0%");
        CHECK(eugene.getParamQuantity(Eugene::SHIFT_CV_KNOB_PARAM)->getString() == "Shift CV: 0%");
}
