#include "doctest.h"

#include "OrbitsRhythm.hpp"
#include "PolygeneModule.hpp"
#include "RackTestHelpers.hpp"

#include <string>

typedef RareBreeds_Orbits_Polygene Polygene;

// The inputs with a "Mono Input Behavior" menu option, and the keys they're saved with.
static const struct
{
        int input;
        const char *key;
} kMonoInputSettings[] = {
        {Polygene::SYNC_INPUT, "sync_cv"},
        {Polygene::LENGTH_CV_INPUT, "length_cv"},
        {Polygene::HITS_CV_INPUT, "hits_cv"},
        {Polygene::SHIFT_CV_INPUT, "shift_cv"},
        {Polygene::VARIATION_CV_INPUT, "variation_cv"},
        {Polygene::RANDOM_CV_INPUT, "random_cv"},
};

static void loadJson(Polygene &polygene, const char *text)
{
        json_t *root = json_loads(text, 0, NULL);
        REQUIRE(root);
        polygene.dataFromJson(root);
        json_decref(root);
}

// Selects a channel with the Channel knob, as a user would.
static void selectChannel(Polygene &polygene, int channel)
{
        Module::ProcessArgs args = testProcessArgs();
        polygene.getParam(Polygene::CHANNEL_KNOB_PARAM).setValue(static_cast<float>(channel));
        polygene.process(args);
}

// Sets the active channel's rhythm with the knobs, as a user would.
static void setRhythm(Polygene &polygene, unsigned int length, unsigned int hits)
{
        Module::ProcessArgs args = testProcessArgs();
        polygene.getParam(Polygene::LENGTH_KNOB_PARAM).setValue(static_cast<float>(length));
        polygene.getParam(Polygene::HITS_KNOB_PARAM).setValue(static_cast<float>(hits) / static_cast<float>(length));
        polygene.process(args);
}

static std::string euclideanPattern(unsigned int length, unsigned int hits, unsigned int clocks)
{
        std::string pattern;
        for(unsigned int step = 0; step < clocks; ++step)
        {
                pattern += orbitsEuclideanBeat(length, hits, 0, step % length) ? 'x' : '.';
        }
        return pattern;
}

static bool sameState(const PolygeneChannelState &a, const PolygeneChannelState &b)
{
        return a.length == b.length && a.hits == b.hits && a.shift == b.shift && a.variation == b.variation &&
               a.reverse == b.reverse && a.invert == b.invert;
}

TEST_CASE("Polygene plays each channel's own rhythm")
{
        Polygene polygene;
        selectChannel(polygene, 1);
        setRhythm(polygene, 5, 2);
        selectChannel(polygene, 0);
        setRhythm(polygene, 8, 3);

        ClockedOutputs outputs =
                sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 2, 16);
        CHECK(outputs.beat[0] == euclideanPattern(8, 3, 16));
        CHECK(outputs.beat[1] == euclideanPattern(5, 2, 16));
        CHECK(outputs.eoc[0] == "........x.......");
        CHECK(outputs.eoc[1] == ".....x....x....x");
}

TEST_CASE("Bypassed, Polygene's outputs are silent, and each rhythm carries on where it was afterwards")
{
        Polygene polygene;
        selectChannel(polygene, 1);
        setRhythm(polygene, 5, 2);
        selectChannel(polygene, 0);
        setRhythm(polygene, 8, 3);

        ClockedOutputs before =
                sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 2, 3);
        CHECK_FALSE(
                sendBypassedClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 2, 4));
        ClockedOutputs after =
                sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 2, 13);
        CHECK(before.beat[0] + after.beat[0] == euclideanPattern(8, 3, 16));
        CHECK(before.beat[1] + after.beat[1] == euclideanPattern(5, 2, 16));
}

TEST_CASE("Polygene's shift, reverse and invert change the active channel's rhythm")
{
        Polygene polygene;
        selectChannel(polygene, 0);
        setRhythm(polygene, 8, 3);

        SUBCASE("Shift rotates the rhythm later")
        {
                polygene.getParam(Polygene::SHIFT_KNOB_PARAM).setValue(1.f);
                CHECK(sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 1, 8)
                              .beat[0] == ".x.x..x.");
        }
        SUBCASE("Reverse plays the rhythm backwards from the last step")
        {
                polygene.getParam(Polygene::REVERSE_KNOB_PARAM).setValue(1.f);
                CHECK(sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 1, 8)
                              .beat[0] == "..x..x.x");
        }
        SUBCASE("Invert swaps on and off beats")
        {
                polygene.getParam(Polygene::INVERT_KNOB_PARAM).setValue(1.f);
                CHECK(sendClocks(polygene, Polygene::CLOCK_INPUT, Polygene::BEAT_OUTPUT, Polygene::EOC_OUTPUT, 1, 8)
                              .beat[0] == ".x.xx.xx");
        }
}

TEST_CASE("Polygene saves and loads every mono input setting")
{
        Polygene saved;
        for(const auto &setting : kMonoInputSettings)
        {
                saved.m_input_mode[setting.input] = INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST;
        }
        json_t *root = saved.dataToJson();

        Polygene loaded;
        loaded.dataFromJson(root);
        for(const auto &setting : kMonoInputSettings)
        {
                CAPTURE(setting.key);
                const bool saved_key = json_object_get(root, setting.key) != NULL;
                CHECK(saved_key);
                CHECK(loaded.m_input_mode[setting.input] == INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST);
        }
        json_decref(root);
}

TEST_CASE("Invalid or missing mono input settings load as the default")
{
        // random_cv is missing, as in patches from v2.0.4 and earlier, and the rest are invalid.
        Polygene polygene;
        for(const auto &setting : kMonoInputSettings)
        {
                polygene.m_input_mode[setting.input] = INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST;
        }
        loadJson(polygene,
                 "{\"sync_cv\": 7, \"length_cv\": -1, \"hits_cv\": \"1\", \"shift_cv\": 1.0, \"variation_cv\": null}");
        for(const auto &setting : kMonoInputSettings)
        {
                CAPTURE(setting.key);
                CHECK(polygene.m_input_mode[setting.input] == INPUT_MODE_MONOPHONIC_COPIES_TO_ALL);
        }
}

TEST_CASE("Patches from v2.0.0 keep their sync behaviour")
{
        Polygene individual;
        loadJson(individual, "{\"sync\": 0}");
        CHECK(individual.m_input_mode[Polygene::SYNC_INPUT] == INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST);

        Polygene all;
        loadJson(all, "{\"sync\": 1}");
        CHECK(all.m_input_mode[Polygene::SYNC_INPUT] == INPUT_MODE_MONOPHONIC_COPIES_TO_ALL);
}

TEST_CASE("Polygene only loads an active channel that exists")
{
        Polygene valid;
        const std::string last = "{\"active_channel_id\": " + std::to_string(PORT_MAX_CHANNELS - 1) + "}";
        loadJson(valid, last.c_str());
        CHECK(valid.m_active_channel_id == PORT_MAX_CHANNELS - 1);

        // The display indexes its channel data with this, so out of range values used to read
        // past the end of it.
        const std::string too_big = "{\"active_channel_id\": " + std::to_string(PORT_MAX_CHANNELS) + "}";
        for(const char *text : {too_big.c_str(), "{\"active_channel_id\": 200}", "{\"active_channel_id\": -1}",
                                "{\"active_channel_id\": \"3\"}"})
        {
                Polygene polygene;
                loadJson(polygene, text);
                CAPTURE(text);
                CHECK(polygene.m_active_channel_id == 0);
                CHECK(Polygene::getDisplayData(&polygene).active_channel_id < PORT_MAX_CHANNELS);
        }
}

TEST_CASE("Polygene keeps the active channel in range whatever the Channel knob's value")
{
        // Rack limits the knob to its range, but a value from elsewhere, e.g. a VCV Rack patch with
        // 16 channels loaded on MetaModule with 4, mustn't index past the channels.
        Polygene polygene;
        selectChannel(polygene, 40);
        CHECK(polygene.m_active_channel_id == PORT_MAX_CHANNELS - 1);
        selectChannel(polygene, -5);
        CHECK(polygene.m_active_channel_id == 0);
}

TEST_CASE("The module browser preview only uses valid channels and rhythms")
{
        const PolygeneDisplayData data = Polygene::getDisplayData(NULL);
        CHECK(data.active_channel_id < PORT_MAX_CHANNELS);
        CHECK(data.active_channels == PORT_MAX_CHANNELS);
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                CAPTURE(c);
                CHECK(data.channels[c].length >= 1);
                CHECK(data.channels[c].length <= kOrbitsRhythmMaxLength);
                CHECK(data.channels[c].hits <= data.channels[c].length);
                CHECK(data.channels[c].current_step < data.channels[c].length);
        }
}

TEST_CASE("Undoing a Random button press restores the channel that was randomized")
{
        Polygene polygene;
        polygene.id = 1;
        APP->engine->addModule(&polygene);
        clearTestUndoHistory();
        Module::ProcessArgs args = testProcessArgs();

        selectChannel(polygene, 3);
        const PolygeneChannelState before = polygene.m_channels[3].m_state;

        polygene.getParam(Polygene::RANDOM_KNOB_PARAM).setValue(1.f);
        polygene.process(args);
        const PolygeneChannelState randomized = polygene.m_channels[3].m_state;
        polygene.getParam(Polygene::RANDOM_KNOB_PARAM).setValue(0.f);
        polygene.process(args);
        CHECK_FALSE(sameState(randomized, before));

        // The press is queued on the engine thread and added to the history on the UI thread.
        CHECK_FALSE(APP->history->canUndo());
        polygene.flushUndoHistory();
        REQUIRE(APP->history->canUndo());

        // Undo used to restore whichever channel was active, rather than the randomized one.
        selectChannel(polygene, 5);
        const PolygeneChannelState other = polygene.m_channels[5].m_state;
        APP->history->undo();
        CHECK(sameState(polygene.m_channels[3].m_state, before));
        CHECK(sameState(polygene.m_channels[5].m_state, other));

        APP->history->redo();
        CHECK(sameState(polygene.m_channels[3].m_state, randomized));

        clearTestUndoHistory();
        APP->engine->removeModule(&polygene);
}

TEST_CASE("Randomizing from the Random CV input doesn't add undo history")
{
        Polygene polygene;
        clearTestUndoHistory();
        Module::ProcessArgs args = testProcessArgs();

        // Let the trigger see a low voltage first, as Rack's triggers start high.
        Input &random_cv = polygene.getInput(Polygene::RANDOM_CV_INPUT);
        connectInput(random_cv, 1);
        polygene.process(args);
        const PolygeneChannelState before = polygene.m_channels[0].m_state;

        random_cv.setVoltage(10.f);
        polygene.process(args);
        CHECK_FALSE(sameState(polygene.m_channels[0].m_state, before));

        polygene.flushUndoHistory();

        CHECK_FALSE(APP->history->canUndo());
}

TEST_CASE("Loading Polygene's widget config twice before the widget draws doesn't leak it")
{
        json_t *first = json_object();
        json_t *second = json_object();
        json_t *first_patch = json_object();
        json_t *second_patch = json_object();
        json_object_set(first_patch, "widget", first);
        json_object_set(second_patch, "widget", second);

        {
                Polygene polygene;
                polygene.dataFromJson(first_patch);
                polygene.dataFromJson(second_patch);
        }
        json_decref(first_patch);
        json_decref(second_patch);

        CHECK(referenceCount(first) == 1);
        CHECK(referenceCount(second) == 1);
        json_decref(first);
        json_decref(second);
}

TEST_CASE("Every Polygene param is configured")
{
        // Rack labels an unconfigured param "#<id + 1>", which shows up in tools that list every param
        Polygene polygene;
        for(int id = 0; id < Polygene::NUM_PARAMS; ++id)
        {
                CAPTURE(id);
                CHECK_FALSE(polygene.getParamQuantity(id)->name.empty());
        }
}

TEST_CASE("Polygene's Hits shows the active channel's number of hits, and takes a typed number of hits")
{
        Polygene polygene;
        APP->engine->addModule(&polygene);
        selectChannel(polygene, 2);
        polygene.getParam(Polygene::LENGTH_KNOB_PARAM).setValue(8.f);
        polygene.getParam(Polygene::HITS_KNOB_PARAM).setValue(0.375f);
        polygene.process(testProcessArgs());
        ParamQuantity *hits = polygene.getParamQuantity(Polygene::HITS_KNOB_PARAM);
        CHECK(hits->getString() == "Hits: 3 / 8");

        hits->setDisplayValueString("6");
        polygene.process(testProcessArgs());
        CHECK(polygene.m_channels[2].readHits(8) == 6);

        APP->engine->removeModule(&polygene);
}

TEST_CASE("Polygene's Variation shows the active channel's variation, and takes a typed variation")
{
        Polygene polygene;
        APP->engine->addModule(&polygene);
        selectChannel(polygene, 1);
        polygene.getParam(Polygene::LENGTH_KNOB_PARAM).setValue(8.f);
        polygene.getParam(Polygene::HITS_KNOB_PARAM).setValue(0.375f);
        polygene.process(testProcessArgs());
        const unsigned int count = orbitsNearEvenRhythmCount(8, 3);
        REQUIRE(count > 2);
        ParamQuantity *variation = polygene.getParamQuantity(Polygene::VARIATION_KNOB_PARAM);
        CHECK(variation->getString() == "Variation: 1 / " + std::to_string(count));

        variation->setDisplayValueString("3");
        polygene.process(testProcessArgs());
        CHECK(polygene.m_channels[1].readVariation(8, 3) == 2);
        CHECK(variation->getDisplayValueString() == "3 / " + std::to_string(count));

        APP->engine->removeModule(&polygene);
}

TEST_CASE("Polygene's Length and Shift show the active channel's whole numbers played")
{
        Polygene polygene;
        APP->engine->addModule(&polygene);
        selectChannel(polygene, 0);
        polygene.getParam(Polygene::LENGTH_KNOB_PARAM).setValue(12.4f);
        polygene.getParam(Polygene::SHIFT_KNOB_PARAM).setValue(14.f);
        polygene.process(testProcessArgs());
        CHECK(polygene.getParamQuantity(Polygene::LENGTH_KNOB_PARAM)->getString() == "Length: 12");
        CHECK(polygene.getParamQuantity(Polygene::SHIFT_KNOB_PARAM)->getString() == "Shift: 2");

        APP->engine->removeModule(&polygene);
}
