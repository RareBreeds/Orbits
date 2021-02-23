#pragma once

#include "Euclidean.hpp"
#include "plugin.hpp"

struct RareBreeds_Orbits_PolygeneWidget;

struct RareBreeds_Orbits_Polygene : Module
{
        static const auto max_channels = 16u;
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
        int m_previous_channel_id = max_channels;

        // Old knob values
        float m_length;
        float m_hits;
        float m_shift;
        float m_variation;

        struct Channel
        {
                unsigned int m_current_step = 0;
                int m_channel;
                dsp::SchmittTrigger m_clock_trigger;
                dsp::SchmittTrigger m_sync_trigger;
                dsp::SchmittTrigger m_reverse_trigger;
                dsp::SchmittTrigger m_invert_trigger;
                dsp::PulseGenerator m_output_generator;
                dsp::PulseGenerator m_eoc_generator;
                float m_length;
                float m_hits;
                float m_shift;
                float m_variation;
                bool m_reverse, m_invert;
                RareBreeds_Orbits_Polygene *m_module;

                void init(RareBreeds_Orbits_Polygene *module, int channel);
                bool readReverse(void);
                bool readInvert(void);
                bool isOnBeat(unsigned int length, unsigned int hits, unsigned int shift, unsigned int variation,
                              unsigned int beat, bool invert);
                unsigned int readLength();
                unsigned int readHits(unsigned int length);
                unsigned int readShift(unsigned int length);
                unsigned int readVariation(unsigned int length, unsigned int shift);
                void process(const ProcessArgs &args);
                json_t *dataToJson();
                void dataFromJson(json_t *root);
                void onRandomize();
        };

        int m_active_channels = 1;
        Channel m_channels[max_channels];
        Channel *m_active_channel;
        dsp::BooleanTrigger reverse_trigger;
        dsp::BooleanTrigger invert_trigger;
        dsp::BooleanTrigger random_trigger;
        dsp::PulseGenerator random_timer;
        dsp::BooleanTrigger sync_trigger;
        RareBreeds_Orbits_PolygeneWidget *widget = NULL;

        RareBreeds_Orbits_Polygene();

        void reset();
        void process(const ProcessArgs &args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onRandomize() override;
        void onReset() override;
};
