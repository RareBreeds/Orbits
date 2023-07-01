#pragma once

#include "OrbitsModule.hpp"
#include "Rhythm.hpp"

#include <atomic>

struct RareBreeds_Orbits_EugeneWidget;

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

        dsp::SchmittTrigger m_clock_trigger;
        dsp::SchmittTrigger m_sync_trigger;
        dsp::SchmittTrigger m_reverse_trigger;
        dsp::SchmittTrigger m_invert_trigger;
        BeatMode m_beat;
        BeatGenerator m_beat_generator;

        EOCMode m_eoc;
        EOCGenerator m_eoc_generator;

        unsigned int m_current_step = 0;
        RareBreeds_Orbits_EugeneWidget *m_widget = NULL;
        std::atomic<json_t *> m_widget_config{nullptr};
        RareBreeds_Orbits_Eugene();
        virtual ~RareBreeds_Orbits_Eugene();

        unsigned int readLength();
        unsigned int readHits(unsigned int length);
        unsigned int readShift(unsigned int length);
        bool readReverse();
        bool readInvert();
        void process(const ProcessArgs &args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onReset() override;
        EugeneDisplayData getDisplayData(void);
        static EugeneDisplayData getDisplayData(RareBreeds_Orbits_Eugene *module);
};
