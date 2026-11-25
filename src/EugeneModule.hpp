#pragma once

#include "OrbitsModule.hpp"
#include "OrbitsRhythm.hpp"
#include "OrbitsStepCounter.hpp"
#include "OrbitsWidgetData.hpp"

#include <atomic>

struct EugeneDisplayData
{
        unsigned int length, hits, shift, current_step;
        bool reverse, invert;

        bool operator==(const EugeneDisplayData &rhs) const
        {
                return length == rhs.length && hits == rhs.hits && shift == rhs.shift &&
                       current_step == rhs.current_step && reverse == rhs.reverse && invert == rhs.invert;
        }

        bool operator!=(const EugeneDisplayData &rhs) const
        {
                return !(*this == rhs);
        }
};

struct RareBreeds_Orbits_Eugene : Module
{
        enum ParamIds
        {
                LENGTH_KNOB_PARAM,
                HITS_KNOB_PARAM,
                SHIFT_KNOB_PARAM,
                LENGTH_CV_KNOB_PARAM,
                HITS_CV_KNOB_PARAM,
                SHIFT_CV_KNOB_PARAM,
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
                RANDOMIZE_LENGTH_CV,
                RANDOMIZE_HITS,
                RANDOMIZE_HITS_CV,
                RANDOMIZE_SHIFT,
                RANDOMIZE_SHIFT_CV,
                RANDOMIZE_REVERSE,
                RANDOMIZE_INVERT,
                RANDOMIZE_COUNT
        };
        const unsigned int RANDOMIZE_ALL = (1 << RANDOMIZE_COUNT) - 1;

        dsp::SchmittTrigger m_clock_trigger;
        dsp::SchmittTrigger m_sync_trigger;
        dsp::SchmittTrigger m_sync_button_trigger;
        dsp::SchmittTrigger m_reverse_trigger;
        dsp::SchmittTrigger m_invert_trigger;
        dsp::SchmittTrigger m_random_trigger;
        BeatMode m_beat;
        BeatGenerator m_beat_generator;

        EOCMode m_eoc;
        EOCGenerator m_eoc_generator;

        OrbitsStepCounter m_steps;
        unsigned int m_randomization_mask = RANDOMIZE_ALL;

        std::atomic<bool> m_reverse{false};
        std::atomic<bool> m_invert{false};

        OrbitsWidgetData m_widget_data;
        RareBreeds_Orbits_Eugene();

        unsigned int readLength();
        unsigned int readHits(unsigned int length);
        unsigned int readShift(unsigned int length);
        bool readReverse();
        bool readInvert();
        void process(const ProcessArgs &process_args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onReset() override;
        EugeneDisplayData getDisplayData();
        static EugeneDisplayData getDisplayData(RareBreeds_Orbits_Eugene *module);
        void onRandomize(const RandomizeEvent &e) override;
};
