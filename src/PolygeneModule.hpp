#pragma once

#include "OrbitsModule.hpp"
#include "Rhythm.hpp"

#include <atomic>

struct RareBreeds_Orbits_PolygeneWidget;

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

struct PolygeneDisplayData
{
        unsigned int active_channel_id, active_channels;
        struct
        {
                unsigned int length, hits, shift, variation, invert, current_step;
        } channels[PORT_MAX_CHANNELS];
};

struct PolygeneChannelState
{
        float length, hits, shift, variation;
        bool reverse, invert;
};

struct RareBreeds_Orbits_Polygene : Module
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

        // The channel currently being displayed and controlled by the knobs
        int m_active_channel_id = 0;
        int m_previous_channel_id = PORT_MAX_CHANNELS;

        struct Channel
        {
                unsigned int m_current_step = 0;
                int m_channel;
                dsp::SchmittTrigger m_clock_trigger;
                dsp::SchmittTrigger m_sync_trigger;
                dsp::SchmittTrigger m_reverse_trigger;
                dsp::SchmittTrigger m_invert_trigger;
                BeatGenerator m_beat_generator;
                EOCGenerator m_eoc_generator;
                PolygeneChannelState m_state;
                RareBreeds_Orbits_Polygene *m_module;

                void init(RareBreeds_Orbits_Polygene *module, int channel);
                bool readReverse(void);
                bool readInvert(void);
                static bool isOnBeat(unsigned int length, unsigned int hits, unsigned int shift, unsigned int variation,
                                     unsigned int beat, bool invert);
                unsigned int readLength();
                unsigned int readStep(unsigned int length)
                {
                        return m_current_step >= length ? 0 : m_current_step;
                }
                unsigned int readHits(unsigned int length);
                unsigned int readShift(unsigned int length);
                unsigned int readVariation(unsigned int length, unsigned int shift);
                void process(const ProcessArgs &args);
                json_t *dataToJson();
                void dataFromJson(json_t *root);
                void onRandomizeWithHistory();
                void onRandomize();
        };

        int m_active_channels = 1;
        Channel m_channels[PORT_MAX_CHANNELS];
        Channel *m_active_channel;
        dsp::BooleanTrigger m_reverse_trigger;
        dsp::BooleanTrigger m_invert_trigger;
        RepeatTrigger m_random_trigger;
        dsp::BooleanTrigger m_sync_trigger;
        RareBreeds_Orbits_PolygeneWidget *m_widget = NULL;
        std::atomic<json_t *> m_widget_config{nullptr};
        BeatMode m_beat;
        EOCMode m_eoc;
        InputMode m_input_mode[NUM_INPUTS];

        RareBreeds_Orbits_Polygene();
        virtual ~RareBreeds_Orbits_Polygene();
        void reset();
        void syncParamsToActiveChannel();
        void process(const ProcessArgs &args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onRandomize(const RandomizeEvent& e) override;
        void onReset(const ResetEvent& e) override;
        PolygeneDisplayData getDisplayData(void);
        static PolygeneDisplayData getDisplayData(RareBreeds_Orbits_Polygene *module);
        float getParameterizedVoltage(int input_id, int channel);
        InputMode getInputMode(int input_id);
};
