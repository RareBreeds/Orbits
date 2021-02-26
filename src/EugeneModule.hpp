#pragma once

#include "OrbitsModule.hpp"
#include "Rhythm.hpp"

struct RareBreeds_Orbits_EugeneWidget;

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
        dsp::PulseGenerator m_output_generator;

        EOCMode m_eoc;
        EOCGenerator m_eoc_generator;

        unsigned int m_current_step = 0;
        rhythm::Rhythm m_rhythm;
        unsigned int m_old_length = rhythm::max_length + 1;
        unsigned int m_old_hits = rhythm::max_length + 1;
        unsigned int m_old_shift = rhythm::max_length + 1;
        bool m_old_reverse = false;
        bool m_old_invert = false;
        RareBreeds_Orbits_EugeneWidget *m_widget = NULL;

        RareBreeds_Orbits_Eugene();

        unsigned int readLength();
        unsigned int readHits(unsigned int length);
        unsigned int readShift(unsigned int length);
        bool readReverse();
        bool readInvert();
        void advanceIndex();
        void updateOutput(const ProcessArgs &args);
        void updateEuclideanRhythm(uint32_t length, uint32_t hits, uint32_t shift, bool invert);
        void updateRhythm();
        void process(const ProcessArgs &args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onReset() override;
        int getEOCMode(void);
        void setEOCMode(int eoc_mode);
};
