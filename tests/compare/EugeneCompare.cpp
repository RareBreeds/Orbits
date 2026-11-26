// Scenarios comparing Eugene's behaviour with other revisions.

#include "ComparePatch.hpp"

static const char *const kEugene = "RareBreeds_Orbits_Eugene";

static const int kPeriod = kComparePeriod;

static void startClock(ComparePatch &patch)
{
        patch.connect("Clock", 1);
        patch.setClock("Clock", kPeriod);
}

// Every factory preset.
static void presets(ComparePatch &patch)
{
        startClock(patch);
        const std::vector<std::string> names = patch.presetNames();
        for(const std::string &name : names)
        {
                patch.loadPreset(name);
                patch.runClocks(40);
        }
}

// Presets loaded while the module plays, as modules that switch presets do (e.g. 8Face, see issue
// #3). Some land in the middle of an output pulse.
static void presetSwitching(ComparePatch &patch)
{
        CompareRandom random;
        startClock(patch);
        const std::vector<std::string> names = patch.presetNames();
        for(int i = 0; i < 200; ++i)
        {
                patch.loadPreset(names[static_cast<size_t>(random.next(static_cast<int>(names.size())))]);
                patch.run(1 + random.next(2 * kPeriod));
        }
}

// Randomizing just after turning a knob, while the engine is still smoothing it, mustn't let the
// smoothing undo the randomization.
static void randomizeWhileSmoothing(ComparePatch &patch)
{
        startClock(patch);
        patch.connect("Random CV", 1);
        for(int i = 0; i < 10; ++i)
        {
                patch.turnKnob("Hits", 0.f);
                patch.run(5);
                patch.trigger("Random CV", 10);
                patch.runClocks(5);
                patch.turnKnob("Hits CV", 1.f);
                patch.run(5);
                patch.randomize();
                patch.runClocks(5);
        }
}

// Hits half way between two steps.
static void halfSteps(ComparePatch &patch)
{
        startClock(patch);
        for(int length = 1; length <= 32; ++length)
        {
                patch.setParam("Length", static_cast<float>(length));
                for(int step = 0; step < length; ++step)
                {
                        patch.setParam("Hits", (static_cast<float>(step) + 0.5f) / static_cast<float>(length));
                        patch.runClocks(length);
                }
        }
}

// Hits CV taking Hits to half a step, where the arithmetic can leave it a hair either side of half.
static void hitsCVHalfSteps(ComparePatch &patch)
{
        static const float voltages[] = {-3.f, -2.f, -1.f, 1.f, 2.f, 3.f};
        startClock(patch);
        patch.setParam("Length", 5.f);
        patch.setParam("Hits", 0.5f);
        patch.setParam("Hits CV", 1.f);
        patch.connect("Hits CV", 1);
        for(float voltage : voltages)
        {
                patch.setVoltage("Hits CV", voltage);
                patch.runClocks(10);
        }
}

static void defaults(ComparePatch &patch)
{
        startClock(patch);
        patch.runClocks(70);
}

static void knobs(ComparePatch &patch)
{
        static const int lengths[] = {1, 2, 3, 5, 8, 13, 16, 31, 32};
        static const float hits[] = {0.f, 0.1f, 0.3f, 0.5f, 0.77f, 1.f};
        startClock(patch);
        for(int length : lengths)
        {
                const int shifts[] = {0, 1, length / 2, length - 1, 31};
                for(float hit : hits)
                {
                        for(int shift : shifts)
                        {
                                patch.setParam("Length", static_cast<float>(length));
                                patch.setParam("Hits", hit);
                                patch.setParam("Shift", static_cast<float>(shift));
                                patch.runClocks(length + 3);
                        }
                }
        }
}

// Knobs turned at any time, not just between clocks, to values between their steps.
static void knobsAnyTime(ComparePatch &patch)
{
        CompareRandom random;
        startClock(patch);
        for(int i = 0; i < 400; ++i)
        {
                switch(random.next(3))
                {
                case 0:
                        patch.setParam("Length", 1.f + static_cast<float>(random.next(3101)) / 100.f);
                        break;
                case 1:
                        patch.turnKnob("Hits", static_cast<float>(random.next(1001)) / 1000.f);
                        break;
                default:
                        patch.setParam("Shift", static_cast<float>(random.next(3101)) / 100.f);
                        break;
                }
                patch.run(1 + random.next(3 * kPeriod));
        }
}

// Sweeps each CV input from `from` to `to` volts at several CV knob settings.
static void sweepCV(ComparePatch &patch, float from, float to)
{
        static const char *const names[] = {"Length", "Hits", "Shift"};
        static const float attenuations[] = {0.25f, 0.5f, 1.f};
        startClock(patch);
        patch.setParam("Length", 12.f);
        patch.setParam("Hits", 0.3f);
        patch.setParam("Shift", 2.f);
        for(const char *name : names)
        {
                const std::string cv = std::string(name) + " CV";
                patch.connect(cv.c_str(), 1);
                for(float attenuation : attenuations)
                {
                        patch.setParam(cv.c_str(), attenuation);
                        for(int step = 0; step <= 40; ++step)
                        {
                                patch.setVoltage(cv.c_str(), from + (to - from) * static_cast<float>(step) / 40.f);
                                patch.runClocks(3);
                        }
                }
                patch.setParam(cv.c_str(), 0.f);
                patch.disconnect(cv.c_str());
        }
}

static void positiveCV(ComparePatch &patch)
{
        sweepCV(patch, 0.f, 10.f);
}

static void negativeCV(ComparePatch &patch)
{
        sweepCV(patch, 0.f, -10.f);
}

// Only the first channel of a polyphonic CV input is used.
static void polyphonicCV(ComparePatch &patch)
{
        startClock(patch);
        patch.connect("Length CV", 3);
        patch.setParam("Length CV", 1.f);
        patch.setVoltage("Length CV", 0, 1.f);
        patch.setVoltage("Length CV", 1, 5.f);
        patch.setVoltage("Length CV", 2, -5.f);
        patch.runClocks(20);
}

static void reverseAndInvert(ComparePatch &patch)
{
        CompareRandom random;
        startClock(patch);
        patch.setParam("Length", 7.f);
        patch.setParam("Hits", 0.4f);
        patch.setParam("Shift", 1.f);
        for(int i = 0; i < 60; ++i)
        {
                patch.setParam(random.next(2) ? "Reverse" : "Invert", static_cast<float>(random.next(2)));
                patch.run(1 + random.next(4 * kPeriod));
        }

        // With their CV inputs connected, the switches are ignored and the inputs are gates. Gates
        // between the low and high thresholds are in reverseCVBetweenClocks().
        patch.setParam("Reverse", 1.f);
        patch.setParam("Invert", 1.f);
        patch.connect("Reverse CV", 1);
        patch.connect("Invert CV", 1);
        patch.runClocks(10);
        static const float gates[] = {10.f, 0.f, 1.f, 2.f, 0.f, 5.f};
        for(int i = 0; i < 60; ++i)
        {
                patch.setVoltage(random.next(2) ? "Reverse CV" : "Invert CV", gates[random.next(6)]);
                patch.run(1 + random.next(4 * kPeriod));
        }
        patch.disconnect("Reverse CV");
        patch.disconnect("Invert CV");
        patch.runClocks(10);
}

// A gate that goes low between clocks, then to a voltage between the low and high thresholds.
static void reverseCVBetweenClocks(ComparePatch &patch)
{
        startClock(patch);
        patch.setParam("Length", 7.f);
        patch.setParam("Hits", 0.4f);
        patch.connect("Reverse CV", 1);
        patch.runClocks(3);
        for(int i = 0; i < 4; ++i)
        {
                patch.run(kPeriod / 4);
                patch.setVoltage("Reverse CV", 2.f);
                patch.run(kPeriod / 4);
                patch.setVoltage("Reverse CV", 0.f);
                patch.run(kPeriod / 4);
                patch.setVoltage("Reverse CV", 0.5f);
                patch.run(kPeriod / 4);
                patch.runClocks(3);
                patch.setVoltage("Reverse CV", 2.f);
                patch.runClocks(1);
                patch.setVoltage("Reverse CV", 0.5f);
                patch.runClocks(3);
        }
}

static void sync(ComparePatch &patch)
{
        startClock(patch);
        patch.setParam("Length", 5.f);
        patch.setParam("Hits", 0.6f);
        patch.connect("Sync", 1);
        patch.runClocks(7);
        // Syncs between clocks, then on the sample before, of and after a clock.
        static const int offsets[] = {kPeriod / 3, kPeriod - 1, kPeriod, kPeriod + 1, kPeriod + 2};
        for(int reverse = 0; reverse < 2; ++reverse)
        {
                patch.setParam("Reverse", static_cast<float>(reverse));
                for(int offset : offsets)
                {
                        patch.runClocks(3);
                        patch.run(offset - 1);
                        patch.trigger("Sync", 1);
                        patch.runClocks(4);
                        patch.run(kPeriod - offset);
                        patch.pressButton("Sync", offset);
                        patch.runClocks(4);
                }
        }
}

static void eocModes(ComparePatch &patch)
{
        static const int lengths[] = {1, 4, 9};
        startClock(patch);
        for(int mode = 0; mode < 3; ++mode)
        {
                patch.setData("eoc", json_integer(mode));
                for(int reverse = 0; reverse < 2; ++reverse)
                {
                        patch.setParam("Reverse", static_cast<float>(reverse));
                        for(int length : lengths)
                        {
                                patch.setParam("Length", static_cast<float>(length));
                                patch.runClocks(2 * length + 1);
                        }
                }
        }
}

static void beatModes(ComparePatch &patch)
{
        patch.connect("Clock", 1);
        patch.setParam("Length", 9.f);
        patch.setParam("Hits", 0.5f);
        for(int mode = 0; mode < 3; ++mode)
        {
                patch.setData("beat", json_integer(mode));
                for(int invert = 0; invert < 2; ++invert)
                {
                        patch.setParam("Invert", static_cast<float>(invert));
                        patch.setClock("Clock", kPeriod);
                        patch.run(20 * kPeriod);
                        // A clock faster than the 1 ms pulses.
                        patch.setClock("Clock", 30);
                        patch.run(20 * 30);
                }
        }
}

static void fastClocks(ComparePatch &patch)
{
        static const int periods[] = {2, 3, 4, 7, 47, 48, 49};
        patch.connect("Clock", 1);
        patch.setParam("Length", 5.f);
        patch.setParam("Hits", 0.6f);
        for(int period : periods)
        {
                patch.setClock("Clock", period);
                patch.run(20 * period);
        }
}

// Only the first channel of a polyphonic clock is used.
static void polyphonicClock(ComparePatch &patch)
{
        patch.connect("Clock", 4);
        patch.setClock("Clock", 0, kPeriod);
        patch.setClock("Clock", 1, kPeriod / 3);
        patch.setClock("Clock", 3, 77);
        patch.runClocks(20);
}

static void randomCV(ComparePatch &patch)
{
        static const int masks[] = {255, 0, 1, 2, 4, 8, 16, 32, 64, 128, 5, 0x55, 0xaa};
        startClock(patch);
        patch.connect("Random CV", 1);
        for(int mask : masks)
        {
                patch.setData("randomization_mask", json_integer(mask));
                for(int i = 0; i < 10; ++i)
                {
                        patch.trigger("Random CV", 10);
                        patch.runClocks(3);
                }
        }
}

// Rack's Initialize and Randomize, from the context menu or its keyboard shortcuts.
static void initializeAndRandomize(ComparePatch &patch)
{
        startClock(patch);
        for(int i = 0; i < 20; ++i)
        {
                patch.randomize();
                patch.runClocks(5);
        }
        patch.setData("randomization_mask", json_integer(0x55));
        for(int i = 0; i < 5; ++i)
        {
                patch.randomize();
                patch.runClocks(5);
        }
        patch.setData("eoc", json_integer(2));
        patch.setData("beat", json_integer(1));
        patch.reset();
        patch.runClocks(20);
}

static void reload(ComparePatch &patch)
{
        startClock(patch);
        patch.setParam("Length", 11.f);
        patch.setParam("Hits", 0.35f);
        patch.setParam("Shift", 3.f);
        patch.setParam("Hits CV", 0.5f);
        patch.connect("Hits CV", 1);
        patch.setVoltage("Hits CV", 2.f);
        patch.setData("eoc", json_integer(1));
        patch.setData("beat", json_integer(2));
        patch.setData("randomization_mask", json_integer(0x0f));
        patch.runClocks(5);
        patch.reloadPatch();
        patch.runClocks(15);
        // In the middle of an output pulse, and between clocks.
        patch.run(kPeriod / 2 + 10);
        patch.reloadPatch();
        patch.runClocks(15);
        patch.setParam("Reverse", 1.f);
        patch.runClocks(4);
        patch.reloadPreset();
        patch.runClocks(15);
        patch.connect("Random CV", 1);
        patch.trigger("Random CV", 10);
        patch.reloadPatch();
        patch.runClocks(15);
}

static void bypass(ComparePatch &patch)
{
        startClock(patch);
        patch.runClocks(10);
        patch.bypass(true);
        patch.runClocks(10);
        patch.bypass(false);
        patch.runClocks(10);
}

// The output pulses last 1 ms whatever the sample rate.
static void sampleRates(ComparePatch &patch)
{
        static const float rates[] = {44100.f, 96000.f, 192000.f, 11025.f};
        startClock(patch);
        for(float rate : rates)
        {
                patch.note("sample rate %g", static_cast<double>(rate));
                setCompareSampleRate(rate);
                patch.runClocks(10);
        }
}

const CompareScenario kEugeneScenarios[] = {
        {"eugene-defaults", kEugene, NULL, defaults},
        {"eugene-knobs", kEugene, NULL, knobs},
        {"eugene-knobs-any-time", kEugene, NULL, knobsAnyTime},
        {"eugene-positive-cv", kEugene, NULL, positiveCV},
        {"eugene-negative-cv", kEugene, kNegativeCV, negativeCV},
        {"eugene-polyphonic-cv", kEugene, NULL, polyphonicCV},
        {"eugene-reverse-invert", kEugene, NULL, reverseAndInvert},
        {"eugene-reverse-cv-between-clocks", kEugene, kReverseCVBetweenClocks, reverseCVBetweenClocks},
        {"eugene-sync", kEugene, NULL, sync},
        {"eugene-eoc-modes", kEugene, NULL, eocModes},
        {"eugene-beat-modes", kEugene, NULL, beatModes},
        {"eugene-fast-clocks", kEugene, NULL, fastClocks},
        {"eugene-polyphonic-clock", kEugene, NULL, polyphonicClock},
        {"eugene-random-cv", kEugene, NULL, randomCV},
        {"eugene-initialize-randomize", kEugene, NULL, initializeAndRandomize},
        {"eugene-reload", kEugene, NULL, reload},
        {"eugene-bypass", kEugene, kBypassSilences, bypass},
        {"eugene-sample-rates", kEugene, NULL, sampleRates},
        {"eugene-presets", kEugene, NULL, presets},
        {"eugene-preset-switching", kEugene, NULL, presetSwitching},
        {"eugene-randomize-while-smoothing", kEugene, kKnobSmoothing, randomizeWhileSmoothing},
        {"eugene-half-steps", kEugene, NULL, halfSteps},
        {"eugene-hits-cv-half-steps", kEugene, kHalfStepRounding, hitsCVHalfSteps},
};
const size_t kEugeneScenarioCount = sizeof(kEugeneScenarios) / sizeof(kEugeneScenarios[0]);
