#pragma once

#include "Euclidean.hpp"
#include "OrbitsModule.hpp"

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

        dsp::SchmittTrigger clockTrigger;
        dsp::SchmittTrigger syncTrigger;
        dsp::SchmittTrigger reverseTrigger;
        dsp::SchmittTrigger invertTrigger;
        dsp::PulseGenerator outputGenerator;

        Orbits::EOCMode eoc;
        Orbits::EOCGenerator eocGenerator;

        unsigned int index = 0;
        euclidean::Rhythm rhythm;
        unsigned int oldLength = euclidean::max_length + 1;
        unsigned int oldHits = euclidean::max_length + 1;
        unsigned int oldShift = euclidean::max_length + 1;
        bool oldReverse = false;
        bool oldInvert = false;
        RareBreeds_Orbits_EugeneWidget *widget = NULL;

        RareBreeds_Orbits_Eugene();

        unsigned int readLength();
        unsigned int readHits(unsigned int length);
        unsigned int readShift(unsigned int length);
        bool readReverse();
        bool readInvert();
        void advanceIndex();
        void updateOutput(const ProcessArgs &args);
        void updateEuclideanRhythm(unsigned int hits, unsigned int length, unsigned int shift, bool invert);
        void updateRhythm();
        void process(const ProcessArgs &args) override;
        json_t *dataToJson() override;
        void dataFromJson(json_t *root) override;
        void onReset() override;
        int getEOCMode(void);
        void setEOCMode(int eoc_mode);
};
