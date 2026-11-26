// Scenarios comparing Polygene's behaviour with other revisions.
//
// Polygene's Length, Hits, Shift, Variation, Reverse and Invert knobs edit the channel the Channel
// knob selects, and turning the Channel knob moves them to that channel's settings. A change of
// channel takes effect on the next sample, so scenarios run a sample before setting the knobs.

#include "ComparePatch.hpp"

#include <string>

static const char *const kPolygene = "RareBreeds_Orbits_Polygene";

static const int kPeriod = kComparePeriod;

static void startClock(ComparePatch &patch, int channels)
{
        patch.connect("Clock", channels);
        patch.setClock("Clock", kPeriod);
}

static void selectChannel(ComparePatch &patch, int channel)
{
        patch.setParam("Channel", static_cast<float>(channel));
        patch.run(1);
}

static void setRhythm(ComparePatch &patch, int channel, float length, float hits, float shift, float variation)
{
        selectChannel(patch, channel);
        patch.setParam("Length", length);
        patch.setParam("Hits", hits);
        patch.setParam("Shift", shift);
        patch.setParam("Variation", variation);
        patch.run(1);
}

// A different rhythm on each channel, some reversed or inverted.
static void setChannelRhythms(ComparePatch &patch)
{
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                const float length = static_cast<float>(1 + (c * 7) % 32);
                setRhythm(patch, c, length, static_cast<float>((c * 5) % 11) / 10.f, static_cast<float>(c % 5),
                          static_cast<float>(c % 4) / 3.f);
                patch.setParam("Reverse", c % 3 == 1 ? 1.f : 0.f);
                patch.setParam("Invert", c % 5 == 2 ? 1.f : 0.f);
                patch.run(1);
        }
        selectChannel(patch, 0);
}

// Every factory preset.
static void presets(ComparePatch &patch)
{
        startClock(patch, 16);
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
        startClock(patch, 4);
        const std::vector<std::string> names = patch.presetNames();
        for(int i = 0; i < 200; ++i)
        {
                patch.loadPreset(names[static_cast<size_t>(random.next(static_cast<int>(names.size())))]);
                patch.run(1 + random.next(2 * kPeriod));
        }
}

// Randomizing or changing channel just after turning a knob, while the engine is still smoothing
// it, mustn't let the smoothing undo the randomization or move the knob's value to another channel.
static void randomizeWhileSmoothing(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 4);
        patch.connect("Random CV", 1);
        for(int i = 0; i < 10; ++i)
        {
                patch.turnKnob("Hits", 0.f);
                patch.run(5);
                patch.pressButton("Randomize channel", 10);
                patch.runClocks(5);
                patch.turnKnob("Variation", 1.f);
                patch.run(5);
                selectChannel(patch, (i + 1) % 4);
                patch.runClocks(5);
                patch.turnKnob("Hits", 0.9f);
                patch.run(5);
                patch.trigger("Random CV", 10);
                patch.runClocks(5);
                patch.turnKnob("Hits", 0.1f);
                patch.run(5);
                patch.randomize();
                patch.runClocks(5);
        }
}

// Hits half way between two steps.
static void halfSteps(ComparePatch &patch)
{
        startClock(patch, 1);
        for(int length = 1; length <= 32; ++length)
        {
                for(int step = 0; step < length; ++step)
                {
                        setRhythm(patch, 0, static_cast<float>(length),
                                  (static_cast<float>(step) + 0.5f) / static_cast<float>(length), 0.f, 0.f);
                        patch.runClocks(length);
                }
        }
}

// Hits CV taking Hits to half a step, where the arithmetic can leave it a hair either side of half.
static void hitsCVHalfSteps(ComparePatch &patch)
{
        static const float voltages[] = {-3.f, -2.f, -1.f, 1.f, 2.f, 3.f};
        startClock(patch, 1);
        setRhythm(patch, 0, 5.f, 0.5f, 0.f, 0.f);
        patch.connect("Hits CV", 1);
        for(float voltage : voltages)
        {
                patch.setVoltage("Hits CV", voltage);
                patch.runClocks(10);
        }
}

static void defaults(ComparePatch &patch)
{
        startClock(patch, 1);
        patch.runClocks(70);
}

static void channels(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 16);
        patch.runClocks(70);

        // Fewer channels, then more again: channels that weren't clocked carry on from where they were.
        static const int counts[] = {4, 1, 9, 16};
        for(int count : counts)
        {
                startClock(patch, count);
                patch.runClocks(20);
        }

        // Editing channels that aren't clocked, then clocking them.
        startClock(patch, 4);
        setRhythm(patch, 10, 5.f, 0.6f, 1.f, 0.f);
        setRhythm(patch, 2, 9.f, 0.3f, 0.f, 0.5f);
        patch.runClocks(10);
        startClock(patch, 12);
        patch.runClocks(20);

        // No clock, so no outputs.
        patch.disconnect("Clock");
        patch.runClocks(2);
        startClock(patch, 3);
        patch.runClocks(10);
}

// Each channel clocked at its own rate.
static void clockRates(ComparePatch &patch)
{
        setChannelRhythms(patch);
        patch.connect("Clock", 16);
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                patch.setClock("Clock", c, 100 + 53 * c);
        }
        patch.runClocks(100);
}

static void knobs(ComparePatch &patch)
{
        static const int lengths[] = {1, 2, 5, 8, 13, 32};
        static const float hits[] = {0.f, 0.3f, 0.5f, 1.f};
        static const float variations[] = {0.f, 0.4f, 1.f};
        startClock(patch, 1);
        for(int length : lengths)
        {
                const int shifts[] = {0, 1, length - 1};
                for(float hit : hits)
                {
                        for(int shift : shifts)
                        {
                                for(float variation : variations)
                                {
                                        setRhythm(patch, 0, static_cast<float>(length), hit, static_cast<float>(shift),
                                                  variation);
                                        patch.runClocks(length + 3);
                                }
                        }
                }
        }
}

// Knobs turned at any time to values between their steps, including the Channel knob. They're set
// immediately: changing channel while a knob smooths is in randomizeWhileSmoothing().
static void knobsAnyTime(ComparePatch &patch)
{
        CompareRandom random;
        setChannelRhythms(patch);
        startClock(patch, 4);
        for(int i = 0; i < 600; ++i)
        {
                switch(random.next(7))
                {
                case 0:
                        patch.setParam("Length", 1.f + static_cast<float>(random.next(3101)) / 100.f);
                        break;
                case 1:
                        patch.setParam("Hits", static_cast<float>(random.next(1001)) / 1000.f);
                        break;
                case 2:
                        patch.setParam("Shift", static_cast<float>(random.next(3101)) / 100.f);
                        break;
                case 3:
                        patch.setParam("Variation", static_cast<float>(random.next(1001)) / 1000.f);
                        break;
                case 4:
                        patch.setParam("Reverse", static_cast<float>(random.next(2)));
                        break;
                case 5:
                        patch.setParam("Invert", static_cast<float>(random.next(2)));
                        break;
                default:
                        patch.setParam("Channel", static_cast<float>(random.next(501)) / 100.f);
                        break;
                }
                patch.run(1 + random.next(3 * kPeriod));
        }
}

// Sweeps each CV input from `from` to `to` volts, with each mono input behaviour, as a mono cable
// and as polyphonic cables with fewer and as many channels as the clock.
static void sweepCV(ComparePatch &patch, float from, float to)
{
        static const char *const names[] = {"Length", "Hits", "Shift", "Variation"};
        static const char *const keys[] = {"length_cv", "hits_cv", "shift_cv", "variation_cv"};
        static const int cable_channels[] = {1, 2, 4};
        setRhythm(patch, 0, 12.f, 0.3f, 2.f, 0.2f);
        setRhythm(patch, 1, 5.f, 0.5f, 0.f, 0.5f);
        setRhythm(patch, 2, 16.f, 0.2f, 1.f, 0.f);
        setRhythm(patch, 3, 1.f, 0.f, 0.f, 1.f);
        startClock(patch, 4);
        for(int i = 0; i < 4; ++i)
        {
                const std::string cv = std::string(names[i]) + " CV";
                for(int mode = 0; mode < 2; ++mode)
                {
                        patch.setData(keys[i], json_integer(mode));
                        for(int channels : cable_channels)
                        {
                                patch.connect(cv.c_str(), channels);
                                for(int step = 0; step <= 20; ++step)
                                {
                                        const float voltage = from + (to - from) * static_cast<float>(step) / 20.f;
                                        for(int c = 0; c < channels; ++c)
                                        {
                                                // Each channel's voltage a little different.
                                                patch.setVoltage(cv.c_str(), c,
                                                                 voltage * (1.f - 0.2f * static_cast<float>(c)));
                                        }
                                        patch.runClocks(3);
                                }
                        }
                }
                patch.setData(keys[i], json_integer(0));
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

static void reverseAndInvert(ComparePatch &patch)
{
        CompareRandom random;
        setChannelRhythms(patch);
        startClock(patch, 4);
        for(int i = 0; i < 80; ++i)
        {
                switch(random.next(3))
                {
                case 0:
                        patch.setParam("Reverse", static_cast<float>(random.next(2)));
                        break;
                case 1:
                        patch.setParam("Invert", static_cast<float>(random.next(2)));
                        break;
                default:
                        selectChannel(patch, random.next(5));
                        break;
                }
                patch.run(1 + random.next(4 * kPeriod));
        }
}

// With their CV inputs connected, the switches are ignored and the inputs are gates, polyphonic or
// mono. Gates between the low and high thresholds are in reverseCVBetweenClocks().
static void reverseAndInvertCV(ComparePatch &patch, int channels)
{
        static const float gates[] = {10.f, 0.f, 1.f, 2.f, 0.f, 5.f};
        CompareRandom random;
        setChannelRhythms(patch);
        startClock(patch, 4);
        patch.connect("Reverse CV", channels);
        patch.connect("Invert CV", channels);
        patch.runClocks(10);
        for(int i = 0; i < 150; ++i)
        {
                patch.setVoltage(random.next(2) ? "Reverse CV" : "Invert CV", random.next(channels),
                                 gates[random.next(6)]);
                patch.run(1 + random.next(4 * kPeriod));
        }
        patch.disconnect("Reverse CV");
        patch.disconnect("Invert CV");
        patch.runClocks(10);
}

// A gate that goes low between clocks, then to a voltage between the low and high thresholds.
static void reverseCVBetweenClocks(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 2);
        patch.connect("Reverse CV", 1);
        patch.runClocks(3);
        for(int i = 0; i < 4; ++i)
        {
                // High, then low, then between the thresholds, all between two clocks.
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

static void reverseAndInvertMonoCV(ComparePatch &patch)
{
        reverseAndInvertCV(patch, 1);
}

static void reverseAndInvertPolyCV(ComparePatch &patch)
{
        reverseAndInvertCV(patch, 4);
}

static void sync(ComparePatch &patch)
{
        static const int offsets[] = {kPeriod / 3, kPeriod - 1, kPeriod, kPeriod + 1};
        static const int cable_channels[] = {1, 3};
        setRhythm(patch, 0, 5.f, 0.6f, 0.f, 0.f);
        setRhythm(patch, 1, 7.f, 0.3f, 1.f, 0.f);
        setRhythm(patch, 2, 3.f, 0.7f, 0.f, 0.f);
        startClock(patch, 3);
        patch.runClocks(7);
        for(int mode = 0; mode < 2; ++mode)
        {
                patch.setData("sync_cv", json_integer(mode));
                for(int channels : cable_channels)
                {
                        patch.connect("Sync", channels);
                        for(int offset : offsets)
                        {
                                patch.runClocks(3);
                                patch.run(offset - 1);
                                patch.trigger("Sync", 1);
                                patch.runClocks(4);
                                // Only one channel of a polyphonic cable.
                                patch.run(kPeriod - offset);
                                patch.setVoltage("Sync", channels - 1, 10.f);
                                patch.run(1);
                                patch.setVoltage("Sync", channels - 1, 0.f);
                                patch.runClocks(4);
                        }
                }
        }

        // The button syncs every channel.
        patch.disconnect("Sync");
        for(int offset : offsets)
        {
                patch.runClocks(3);
                patch.run(offset);
                patch.pressButton("Sync", 10);
                patch.runClocks(4);
        }
}

// Patches from v2.0.0 have a "sync" setting instead of "sync_cv", with the opposite meaning.
static void oldSyncSetting(ComparePatch &patch)
{
        setRhythm(patch, 0, 5.f, 0.6f, 0.f, 0.f);
        setRhythm(patch, 1, 7.f, 0.3f, 1.f, 0.f);
        startClock(patch, 2);
        patch.connect("Sync", 1);
        for(int old_sync = 0; old_sync < 2; ++old_sync)
        {
                patch.setData("sync", json_integer(old_sync));
                for(int i = 0; i < 3; ++i)
                {
                        patch.runClocks(3);
                        patch.run(kPeriod / 2);
                        patch.trigger("Sync", 1);
                        patch.runClocks(4);
                }
        }
}

static void eocModes(ComparePatch &patch)
{
        setRhythm(patch, 0, 1.f, 0.5f, 0.f, 0.f);
        setRhythm(patch, 1, 4.f, 0.5f, 0.f, 0.f);
        setRhythm(patch, 2, 9.f, 0.5f, 0.f, 0.f);
        startClock(patch, 3);
        for(int mode = 0; mode < 3; ++mode)
        {
                patch.setData("eoc", json_integer(mode));
                for(int reverse = 0; reverse < 2; ++reverse)
                {
                        for(int c = 0; c < 3; ++c)
                        {
                                selectChannel(patch, c);
                                patch.setParam("Reverse", static_cast<float>(reverse));
                        }
                        patch.runClocks(19);
                }
        }
}

static void beatModes(ComparePatch &patch)
{
        setRhythm(patch, 0, 9.f, 0.5f, 0.f, 0.f);
        setRhythm(patch, 1, 7.f, 0.3f, 2.f, 0.5f);
        patch.connect("Clock", 2);
        for(int mode = 0; mode < 3; ++mode)
        {
                patch.setData("beat", json_integer(mode));
                for(int invert = 0; invert < 2; ++invert)
                {
                        patch.setParam("Invert", static_cast<float>(invert));
                        patch.setClock("Clock", kPeriod);
                        patch.runClocks(20);
                        // A clock faster than the 1 ms pulses.
                        patch.setClock("Clock", 30);
                        patch.run(20 * 30);
                }
        }
}

static void fastClocks(ComparePatch &patch)
{
        static const int periods[] = {2, 3, 4, 7, 47, 48, 49};
        setRhythm(patch, 0, 5.f, 0.6f, 0.f, 0.f);
        setRhythm(patch, 1, 8.f, 0.4f, 1.f, 0.5f);
        patch.connect("Clock", 2);
        for(int period : periods)
        {
                patch.setClock("Clock", period);
                patch.run(20 * period);
        }
}

static const int kRandomizationMasks[] = {63, 0, 1, 2, 4, 8, 16, 32, 5, 0x15, 0x2a};

// The Random button randomizes the selected channel, and repeats while it's held.
static void randomButton(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 4);
        for(int mask : kRandomizationMasks)
        {
                patch.setData("randomization_mask", json_integer(mask));
                for(int c = 0; c < 5; ++c)
                {
                        selectChannel(patch, c);
                        patch.pressButton("Randomize channel", 10);
                        patch.runClocks(3);
                }
        }
        patch.setData("randomization_mask", json_integer(63));
        selectChannel(patch, 1);
        // Held for 1.2 seconds.
        patch.pressButton("Randomize channel", 57600);
        patch.runClocks(20);
}

// A mono Random CV cable randomizes every channel by default, a polyphonic one each channel.
static void randomCV(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 4);
        patch.connect("Random CV", 1);
        for(int mask : kRandomizationMasks)
        {
                patch.setData("randomization_mask", json_integer(mask));
                for(int i = 0; i < 5; ++i)
                {
                        patch.trigger("Random CV", 10);
                        patch.runClocks(3);
                }
        }
        patch.setData("randomization_mask", json_integer(63));
        patch.connect("Random CV", 4);
        for(int i = 0; i < 12; ++i)
        {
                const int channel = (i * 3) % 4;
                patch.setVoltage("Random CV", channel, 10.f);
                patch.run(10);
                patch.setVoltage("Random CV", channel, 0.f);
                patch.runClocks(3);
        }
}

// The mono input behaviour option for Random CV, which v2.0.4 didn't load from the patch.
static void randomCVMonoMode(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 4);
        patch.connect("Random CV", 1);
        patch.setData("random_cv", json_integer(1));
        for(int i = 0; i < 5; ++i)
        {
                patch.trigger("Random CV", 10);
                patch.runClocks(3);
        }
}

// Rack's Initialize and Randomize, from the context menu or its keyboard shortcuts.
static void initializeAndRandomize(ComparePatch &patch)
{
        startClock(patch, 4);
        for(int i = 0; i < 20; ++i)
        {
                patch.randomize();
                patch.runClocks(5);
        }
        patch.setData("randomization_mask", json_integer(0x15));
        for(int i = 0; i < 5; ++i)
        {
                patch.randomize();
                patch.runClocks(5);
        }
        selectChannel(patch, 2);
        patch.setData("eoc", json_integer(2));
        patch.setData("beat", json_integer(1));
        patch.setData("length_cv", json_integer(1));
        patch.reset();
        patch.runClocks(20);
}

static void reload(ComparePatch &patch)
{
        setChannelRhythms(patch);
        selectChannel(patch, 5);
        startClock(patch, 8);
        patch.connect("Hits CV", 1);
        patch.setVoltage("Hits CV", 1.f);
        patch.setData("eoc", json_integer(1));
        patch.setData("beat", json_integer(2));
        patch.setData("hits_cv", json_integer(1));
        patch.setData("sync_cv", json_integer(1));
        patch.setData("randomization_mask", json_integer(0x0f));
        patch.runClocks(5);
        patch.reloadPatch();
        patch.runClocks(15);
        // In the middle of an output pulse, and between clocks.
        patch.run(kPeriod / 2 + 10);
        patch.reloadPatch();
        patch.runClocks(15);
        setRhythm(patch, 3, 6.f, 0.5f, 1.f, 0.f);
        patch.runClocks(4);
        patch.reloadPreset();
        patch.runClocks(15);
        patch.pressButton("Randomize channel", 10);
        patch.reloadPatch();
        patch.runClocks(15);
}

static void bypass(ComparePatch &patch)
{
        setChannelRhythms(patch);
        startClock(patch, 4);
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
        setChannelRhythms(patch);
        startClock(patch, 4);
        for(float rate : rates)
        {
                patch.note("sample rate %g", static_cast<double>(rate));
                setCompareSampleRate(rate);
                patch.runClocks(10);
        }
}

const CompareScenario kPolygeneScenarios[] = {
        {"polygene-defaults", kPolygene, NULL, defaults},
        {"polygene-channels", kPolygene, NULL, channels},
        {"polygene-clock-rates", kPolygene, NULL, clockRates},
        {"polygene-knobs", kPolygene, NULL, knobs},
        {"polygene-knobs-any-time", kPolygene, NULL, knobsAnyTime},
        {"polygene-positive-cv", kPolygene, NULL, positiveCV},
        // On arm64 this only shows the half step rounding, at Hits CV -2V.
        {"polygene-negative-cv", kPolygene, kNegativeCV, negativeCV},
        {"polygene-reverse-invert", kPolygene, NULL, reverseAndInvert},
        {"polygene-reverse-invert-mono-cv", kPolygene, NULL, reverseAndInvertMonoCV},
        {"polygene-reverse-invert-poly-cv", kPolygene, NULL, reverseAndInvertPolyCV},
        {"polygene-reverse-cv-between-clocks", kPolygene, kReverseCVBetweenClocks, reverseCVBetweenClocks},
        {"polygene-sync", kPolygene, NULL, sync},
        {"polygene-old-sync-setting", kPolygene, NULL, oldSyncSetting},
        {"polygene-eoc-modes", kPolygene, NULL, eocModes},
        {"polygene-beat-modes", kPolygene, NULL, beatModes},
        {"polygene-fast-clocks", kPolygene, NULL, fastClocks},
        {"polygene-random-button", kPolygene, NULL, randomButton},
        {"polygene-random-cv", kPolygene, NULL, randomCV},
        {"polygene-random-cv-mono-mode", kPolygene,
         "Polygene's mono input behaviour option for the RND input wasn't saved with the patch", randomCVMonoMode},
        {"polygene-initialize-randomize", kPolygene, NULL, initializeAndRandomize},
        {"polygene-reload", kPolygene, NULL, reload},
        {"polygene-bypass", kPolygene, kBypassSilences, bypass},
        {"polygene-sample-rates", kPolygene, NULL, sampleRates},
        {"polygene-presets", kPolygene, NULL, presets},
        {"polygene-preset-switching", kPolygene, NULL, presetSwitching},
        {"polygene-randomize-while-smoothing", kPolygene, kKnobSmoothing, randomizeWhileSmoothing},
        {"polygene-half-steps", kPolygene, NULL, halfSteps},
        {"polygene-hits-cv-half-steps", kPolygene, kHalfStepRounding, hitsCVHalfSteps},
};
const size_t kPolygeneScenarioCount = sizeof(kPolygeneScenarios) / sizeof(kPolygeneScenarios[0]);
