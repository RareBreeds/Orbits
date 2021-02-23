#include "EugeneModule.hpp"
#include "Euclidean.hpp"
#include "EugeneWidget.hpp"


// TODO: Maximum length shouldn't be defined by what the Euclidean module can support
// should be separate and Euclidean needs to support the max of all modules, or templated.
RareBreeds_Orbits_Eugene::RareBreeds_Orbits_Eugene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(LENGTH_KNOB_PARAM, 1.f, euclidean::max_length, euclidean::max_length, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, euclidean::max_length - 1, 0.f, "Shift");
        configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
        configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
        configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
        configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
        configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");

        // Ensure the default rhythm exists before drawing
        updateRhythm();
}

unsigned int RareBreeds_Orbits_Eugene::readLength()
{
        float value = params[LENGTH_KNOB_PARAM].getValue();
        if(inputs[LENGTH_CV_INPUT].isConnected())
        {
                // bipolar +-5V input
                float input = inputs[LENGTH_CV_INPUT].getVoltage();
                float normalized_input = input / 5.f;
                float attenuation = params[LENGTH_CV_KNOB_PARAM].getValue();
                value += attenuation * normalized_input * (euclidean::max_length - 1);
        }

        return clamp(int(std::round(value)), 1, euclidean::max_length);
}

unsigned int RareBreeds_Orbits_Eugene::readHits(unsigned int length)
{
        float value = params[HITS_KNOB_PARAM].getValue();
        if(inputs[HITS_CV_INPUT].isConnected())
        {
                // bipolar +-5V input
                float input = inputs[HITS_CV_INPUT].getVoltage();
                float normalized_input = input / 5.f;
                float attenuation = params[HITS_CV_KNOB_PARAM].getValue();
                value += attenuation * normalized_input;
        }

        float hits_float = value * length;
        return clamp(int(std::round(hits_float)), 0, length);
}

unsigned int RareBreeds_Orbits_Eugene::readShift(unsigned int length)
{
        float value = params[SHIFT_KNOB_PARAM].getValue();
        if(inputs[SHIFT_CV_INPUT].isConnected())
        {
                // bipolar +-5V input
                float input = inputs[SHIFT_CV_INPUT].getVoltage();
                float normalized_input = input / 5.f;
                float attenuation = params[SHIFT_CV_KNOB_PARAM].getValue();
                value += attenuation * normalized_input * (euclidean::max_length - 1);
        }

        return clamp(int(std::round(value)), 0, euclidean::max_length - 1) % length;
}

bool RareBreeds_Orbits_Eugene::readReverse()
{
        if(inputs[REVERSE_CV_INPUT].isConnected())
        {
                reverseTrigger.process(inputs[REVERSE_CV_INPUT].getVoltage());
                return reverseTrigger.isHigh();
        }
        else
        {
                return std::round(params[REVERSE_KNOB_PARAM].getValue());
        }
}

bool RareBreeds_Orbits_Eugene::readInvert()
{
        if(inputs[INVERT_CV_INPUT].isConnected())
        {
                invertTrigger.process(inputs[INVERT_CV_INPUT].getVoltage());
                return invertTrigger.isHigh();
        }
        else
        {
                return std::round(params[INVERT_KNOB_PARAM].getValue());
        }
}

void RareBreeds_Orbits_Eugene::advanceIndex()
{
        unsigned int length = readLength();
        if(readReverse())
        {
                if(index == 0)
                {
                        index = length - 1;
                }
                else
                {
                        --index;
                }
        }
        else
        {
                ++index;
                if(index >= length)
                {
                        index = 0;
                }
        }
}

void RareBreeds_Orbits_Eugene::updateOutput(const ProcessArgs &args)
{
        if(inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage()))
        {
                index = 0;
        }

        if(inputs[CLOCK_INPUT].isConnected() && clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
        {
                auto length = readLength();
                auto reverse = readReverse();

                if(reverse)
                {
                        if(index == 0)
                        {
                                index = length - 1;
                        }
                        else
                        {
                                --index;
                        }
                }

                if(rhythm[index])
                {
                        outputGenerator.trigger(1e-3f);
                }

                if(!reverse)
                {
                        if(index == length - 1)
                        {
                                index = 0;
                        }
                        else
                        {
                                ++index;
                        }
                }
        }

        outputs[BEAT_OUTPUT].setVoltage(outputGenerator.process(args.sampleTime) ? 10.f : 0.f);
}

void RareBreeds_Orbits_Eugene::updateEuclideanRhythm(unsigned int hits, unsigned int length, unsigned int shift,
                                                     bool invert)
{
        rhythm = euclidean::rhythm(length, hits);

        auto tmp = rhythm;
        for(unsigned int i = 0; i < length; ++i)
        {
                rhythm[(i + shift) % length] = tmp[i];
        }

        if(invert)
        {
                rhythm.flip();
        }
}

void RareBreeds_Orbits_Eugene::updateRhythm()
{
        bool update = false;

        unsigned int length = readLength();
        if(length != oldLength)
        {
                oldLength = length;
                update = true;

                // wrap the index to the new length
                // to avoid accessing the rhythm out of bounds
                if(index >= length)
                {
                        index = 0;
                }
        }

        unsigned int shift = readShift(length);
        if(shift != oldShift)
        {
                oldShift = shift;
                update = true;
        }

        unsigned int hits = readHits(length);
        if(hits != oldHits)
        {
                oldHits = hits;
                update = true;
        }

        bool reverse = readReverse();
        if(reverse != oldReverse)
        {
                oldReverse = reverse;
                update = true;
        }

        bool invert = readInvert();
        if(invert != oldInvert)
        {
                oldInvert = invert;
                update = true;
        }

        if(update)
        {
                // This function can take a bit of time to run
                // Calling it for every sample is way too expensive
                updateEuclideanRhythm(hits, length, shift, invert);
        }
}

void RareBreeds_Orbits_Eugene::process(const ProcessArgs &args)
{
        updateRhythm();
        updateOutput(args);
}

json_t *RareBreeds_Orbits_Eugene::dataToJson()
{
        json_t *root = json_object();
        if(widget)
        {
                json_t *w = widget->dataToJson();
                if(w)
                {
                        json_object_set_new(root, "widget", w);
                }
        }
        return root;
}

void RareBreeds_Orbits_Eugene::dataFromJson(json_t *root)
{
        if(root)
        {
                if(widget)
                {
                        json_t *obj = json_object_get(root, "widget");
                        if(obj)
                        {
                                widget->dataFromJson(obj);
                        }
                }
        }
}

void RareBreeds_Orbits_Eugene::onReset()
{
        index = 0;
}