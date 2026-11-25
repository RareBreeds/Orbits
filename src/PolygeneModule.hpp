#pragma once

#include "OrbitsModule.hpp"
#include "OrbitsRhythm.hpp"
#include "OrbitsStepCounter.hpp"
#include "OrbitsUndoQueue.hpp"
#include "OrbitsWidgetData.hpp"

#include <atomic>

enum InputMode
{
        // Default behavior, as specified by polyphonic voltage standards
        // https://vcvrack.com/manual/VoltageStandards#Polyphony
        // "If monophonic (M = 1), its voltage should be copied to all engines."
        INPUT_MODE_MONOPHONIC_COPIES_TO_ALL,

        // The behavior a lot of people expect, only the first channel is
        // modulated by a monophonic cable.
        INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST
};

struct PolygeneChannelDisplayData
{
        unsigned int length, hits, shift, variation, invert, current_step;
};

struct PolygeneDisplayData
{
        uint8_t active_channel_id, active_channels;
        PolygeneChannelDisplayData channels[PORT_MAX_CHANNELS];
};

struct PolygeneChannelState
{
        float length, hits, shift, variation;
        bool reverse, invert;
};

struct PolygeneRandomizeRecord
{
        uint8_t channel;
        PolygeneChannelState old_state, new_state;
};

struct RareBreeds_Orbits_Polygene : Module, OrbitsUndoSource
{
        enum ParamIds
        {
                CHANNEL_KNOB_PARAM,
                LENGTH_KNOB_PARAM,
                HITS_KNOB_PARAM,
                SHIFT_KNOB_PARAM,
                VARIATION_KNOB_PARAM,
                REVERSE_KNOB_PARAM,
                INVERT_KNOB_PARAM,
                RANDOM_KNOB_PARAM,
                SYNC_KNOB_PARAM,
                NUM_PARAMS
        };
        enum InputIds
        {
                CLOCK_INPUT,
                SYNC_INPUT,
                LENGTH_CV_INPUT,
                HITS_CV_INPUT,
                SHIFT_CV_INPUT,
                VARIATION_CV_INPUT,
                REVERSE_CV_INPUT,
                INVERT_CV_INPUT,
                RANDOM_CV_INPUT,
                NUM_INPUTS
        };
        enum OutputIds
        {
                BEAT_OUTPUT,
                EOC_OUTPUT,
                NUM_OUTPUTS
        };
        enum LightIds
        {
                NUM_LIGHTS
        };
        enum RandomizationMaskBits
        {
                RANDOMIZE_LENGTH,
                RANDOMIZE_HITS,
                RANDOMIZE_SHIFT,
                RANDOMIZE_VARIATION,
                RANDOMIZE_REVERSE,
                RANDOMIZE_INVERT,
                RANDOMIZE_COUNT
        };
        const unsigned int RANDOMIZE_ALL = (1 << RANDOMIZE_COUNT) - 1;

        // The channel currently being displayed and controlled by the knobs
        uint8_t m_active_channel_id = 0;
        uint8_t m_previous_channel_id = PORT_MAX_CHANNELS;

        struct Channel
        {
                OrbitsStepCounter m_steps;
                uint8_t m_channel = 0;
                dsp::SchmittTrigger m_clock_trigger;
                dsp::SchmittTrigger m_sync_trigger;
                dsp::SchmittTrigger m_reverse_trigger;
                dsp::SchmittTrigger m_invert_trigger;
                dsp::SchmittTrigger m_random_trigger;
                BeatGenerator m_beat_generator;
                EOCGenerator m_eoc_generator;
                PolygeneChannelState m_state;

                // Reverse/invert as last evaluated by the module's process() on the engine thread.
                // The display reads these instead of evaluating the triggers itself.
                std::atomic<bool> m_reverse{false};
                std::atomic<bool> m_invert{false};

                RareBreeds_Orbits_Polygene *m_module = NULL;

                void init(RareBreeds_Orbits_Polygene *module, uint8_t channel);
                bool readReverse(void);
                bool readInvert(void);
                static bool isOnBeat(unsigned int length, unsigned int hits, unsigned int shift, unsigned int variation,
                                     unsigned int beat, bool invert);
                unsigned int readLength();
                unsigned int readHits(unsigned int length);
                unsigned int readShift(unsigned int length);
                unsigned int readVariation(unsigned int length, unsigned int shift);
                void process(const ProcessArgs &args);
                json_t *dataToJson();
                void dataFromJson(json_t *root);
                void onRandomizeWithHistory(unsigned int randomization_mask);
                void onRandomize(unsigned int randomization_mask);
        };

        uint8_t m_active_channels = 1;
        Channel m_channels[PORT_MAX_CHANNELS];
        Channel *m_active_channel = NULL;
        dsp::BooleanTrigger m_reverse_trigger;
        dsp::BooleanTrigger m_invert_trigger;
        RepeatTrigger m_random_trigger;
        dsp::BooleanTrigger m_sync_trigger;
        OrbitsWidgetData m_widget_data;

        // Channels randomized by the Random button, waiting to be added to the undo history.
        OrbitsUndoQueue<PolygeneRandomizeRecord> m_randomize_undo;

        BeatMode m_beat;
        EOCMode m_eoc;
        InputMode m_input_mode[NUM_INPUTS];
        unsigned int m_randomization_mask = RANDOMIZE_ALL;

        RareBreeds_Orbits_Polygene();
        void reset();
        void syncParamsToActiveChannel();
        void process(const ProcessArgs &process_args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onRandomize(const RandomizeEvent &e) override;
        void onReset(const ResetEvent &e) override;
        void flushUndoHistory() override;
        PolygeneDisplayData getDisplayData(void);
        static PolygeneDisplayData getDisplayData(RareBreeds_Orbits_Polygene *module);
        float getParameterizedVoltage(int input_id, uint8_t channel);
        InputMode getInputMode(int input_id);
};
