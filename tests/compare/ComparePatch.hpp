#pragma once

#include "plugin.hpp"

#include <cstdio>
#include <string>
#include <vector>

// The scenarios' usual clock period in samples: 10 ms at 48 kHz, so the modules' 1 ms output pulses
// end well before the next clock.
static const int kComparePeriod = 480;

// A patch for comparing the plugin's behaviour between revisions, set up in Rack's engine as a user
// would set it up in Rack. A source module's cables go to the inputs of the module under test, and
// cables from all of its outputs go to a recorder module. Each change the recorder sees is written
// to a log, with the sample it happened on.
//
// The scenarios are built against the plugin's sources from other revisions too, so they only use
// Rack's API: modules, params, inputs and outputs are found by their slugs and names, and options
// are set through the module's saved data, as loading a patch would set them.
struct ComparePatch
{
        // Adds a module of the plugin's `model_slug` to the engine, with the source and recorder.
        ComparePatch(const char *model_slug, std::FILE *log);
        // Removes and deletes every module and cable in the engine.
        ~ComparePatch();

        // Connects a cable with `channels` channels from the source to `input`. Its voltages start at 0V.
        void connect(const char *input, int channels);
        void disconnect(const char *input);
        // Sets a connected input's voltage, on every channel or on one.
        void setVoltage(const char *input, float voltage);
        void setVoltage(const char *input, int channel, float voltage);
        // Sends a square wave clock between 0V and 10V, high for the first half of each `period`
        // samples and starting with a rising edge on the next sample, on every channel or on one.
        void setClock(const char *input, int period);
        void setClock(const char *input, int channel, int period);

        // Sets a param immediately, as turning a knob, a switch or a button, typing a value or MIDI
        // mapping does.
        void setParam(const char *param, float value);
        // Sets a param as turning a knob that smooths its changes does in Rack: the engine moves the
        // param towards the value over the next few hundred milliseconds. In v2.0.4, Hits,
        // Variation and Eugene's CV knobs smoothed; since then, none of the knobs do. Only one param
        // is smoothed at a time.
        void turnKnob(const char *param, float value);
        // Presses and releases a button, holding it for `samples` samples.
        void pressButton(const char *param, int samples);
        // Sets a value in the module's saved data and loads it, as loading a patch with it would.
        void setData(const char *key, json_t *value);
        // The module's saved data, and loading all of it. setModuleData() takes the reference.
        json_t *moduleData();
        void setModuleData(json_t *data);

        // Sends a trigger of `samples` samples at 10V to a connected input, on every channel.
        void trigger(const char *input, int samples);

        // Runs the engine for `samples` samples, or for `clocks` periods of kComparePeriod. Throws if
        // an output was anything other than 0V or 10V, as the modules' outputs are all gates.
        void run(int samples);
        void runClocks(int clocks);

        // Saves the whole patch and loads it again, as saving and reopening it in Rack would. With
        // m_save_prefix, also writes the patch to <m_save_prefix>-<n>.json, the nth reload. With
        // m_load_prefix, loads <m_load_prefix>-<n>.json instead, a patch another build saved.
        void reloadPatch();
        // Saves the module's settings and loads them into the same module, as a preset would.
        void reloadPreset();
        // Loads a preset file, as Rack's preset menu does. `name` is a file in the model's directory
        // in m_presets_directory.
        void loadPreset(const std::string &name);
        // The names of the model's preset files, sorted.
        std::vector<std::string> presetNames() const;
        // Rack's context menu Initialize and Randomize, and bypassing the module.
        void reset();
        void randomize();
        void bypass(bool bypassed);
        // Writes a note to the log. Every change to the patch writes one, so the log shows what caused
        // each change of the outputs.
        void note(const char *format, ...) __attribute__((format(printf, 2, 3)));

        // The state the source and recorder modules use, kept here as reloading the patch recreates
        // them. The signal the source sends to each of the module's inputs:
        struct Signal
        {
                int channels = 0;
                float voltage[PORT_MAX_CHANNELS] = {};
                int period[PORT_MAX_CHANNELS] = {};
                int64_t start[PORT_MAX_CHANNELS] = {};
        };
        Module *m_module = NULL;
        Module *m_source = NULL;
        Module *m_recorder = NULL;
        std::FILE *m_log;
        int64_t m_frame = 0;
        std::vector<Signal> m_signals;
        // The ID of the cable to each input, or -1 if it isn't connected.
        std::vector<int64_t> m_input_cable_ids;
        std::vector<int> m_recorded_channels;
        std::vector<std::vector<float>> m_recorded_voltages;
        // The plugin's presets directory, with a directory per model.
        std::string m_presets_directory;
        std::string m_save_prefix;
        std::string m_load_prefix;
        int m_reloads = 0;
        // Set by the recorder when an output isn't 0V or 10V.
        std::string m_invalid_output;

      private:
        int findParam(const char *name) const;
        int findInput(const char *name) const;
        // Finds the modules and the cables to the module's inputs, after creating or loading the patch.
        void findModules();
};

// Registers the plugin and the source and recorder modules with Rack, so patches that use them load.
void registerComparePlugins();

// A small deterministic random number generator for scenarios, separate from Rack's, which the
// modules use when they randomize.
struct CompareRandom
{
        uint32_t m_state = 1;

        // A number from 0 to `count` - 1.
        int next(int count)
        {
                m_state = m_state * 1664525u + 1013904223u;
                return static_cast<int>((m_state >> 8) % static_cast<uint32_t>(count));
        }
};

// Since 88aaa40, Reverse and Invert CV are read on every sample. Before, they were only read on
// clocks (and, in Rack, as the display drew), so a gate going low between clocks and then to a
// voltage between the low and high thresholds wasn't seen as low.
static const char *const kReverseCVBetweenClocks =
        "Reverse CV going low between clocks is seen, even if it then stays between 0V and 1V";

// Since 59bbc4c, randomizing or changing Polygene's channel stops the smoothing of a knob that was
// just turned. Before, the smoothing carried on, overwriting the new value.
static const char *const kKnobSmoothing =
        "Hits could sometimes stay at 0 after randomizing (a knob still smoothing after being turned "
        "overwrote randomized values, and in Polygene the next channel's after changing channel)";

// Since 59bbc4c, values are rounded with std::round(). v2.0.4 added 0.5 and truncated, and the
// float addition rounded values a hair below half a step up, e.g. Length 5, Hits 50% and Hits CV
// -2V gives 0.49999997 hits: 1 in v2.0.4, 0 now.
static const char *const kHalfStepRounding = "Values a hair below half a step, e.g. from CV, now round down";

static const char *const kNegativeCV =
        "Length, Hits and Shift CV well below 0V could jump to the maximum value instead of the minimum";

typedef void (*CompareScenarioFunction)(ComparePatch &patch);

struct CompareScenario
{
        // Also the name of the file its log is written to.
        const char *name;
        const char *model_slug;
        // Why the scenario behaves differently from the last release, or NULL if it shouldn't.
        const char *expected_difference;
        CompareScenarioFunction run;
};

// Defined in EugeneCompare.cpp and PolygeneCompare.cpp.
extern const CompareScenario kEugeneScenarios[];
extern const size_t kEugeneScenarioCount;
extern const CompareScenario kPolygeneScenarios[];
extern const size_t kPolygeneScenarioCount;
// Defined in InvalidPatches.cpp.
extern const CompareScenario kInvalidPatchScenarios[];
extern const size_t kInvalidPatchScenarioCount;

// Defined in CompareMain.cpp, which can use Rack's internal functions. Scenarios start at 48 kHz.
void setCompareSampleRate(float sample_rate);
