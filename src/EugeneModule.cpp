#include "EugeneModule.hpp"
#include "EugeneWidget.hpp"
#include "OrbitsRhythm.hpp"

RareBreeds_Orbits_Eugene::RareBreeds_Orbits_Eugene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(LENGTH_KNOB_PARAM, 1.f, kOrbitsRhythmMaxLength, kOrbitsRhythmMaxLength, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, kOrbitsRhythmMaxLength - 1, 0.f, "Shift");
        configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
        configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
        configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
        configSwitch(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});
        configSwitch(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert", {"Off", "On"});
        // Without a config Rack labels it "#9"
        configParam(UNUSED_PARAM, 0.f, 1.f, 0.f, "Unused")->randomizeEnabled = false;
        configButton(SYNC_KNOB_PARAM, "Sync");

        configInput(CLOCK_INPUT, "Clock");
        configInput(SYNC_INPUT, "Sync");
        configInput(LENGTH_CV_INPUT, "Length CV");
        configInput(HITS_CV_INPUT, "Hits CV");
        configInput(SHIFT_CV_INPUT, "Shift CV");
        configInput(REVERSE_CV_INPUT, "Reverse CV");
        configInput(INVERT_CV_INPUT, "Invert");
        configInput(RANDOM_CV_INPUT, "Random CV");

        configOutput(BEAT_OUTPUT, "Beat");
        configOutput(EOC_OUTPUT, "End of cycle");

        configBypass(CLOCK_INPUT, BEAT_OUTPUT);
}

EugeneDisplayData RareBreeds_Orbits_Eugene::getDisplayData()
{
        EugeneDisplayData data;
        data.length = readLength();
        data.hits = readHits(data.length);
        data.shift = readShift(data.length);
        data.reverse = m_reverse.load(std::memory_order_relaxed);
        data.invert = m_invert.load(std::memory_order_relaxed);
        data.current_step = m_steps.position(data.length);
        return data;
}

EugeneDisplayData RareBreeds_Orbits_Eugene::getDisplayData(RareBreeds_Orbits_Eugene *module)
{
        if(module)
        {
                return module->getDisplayData();
        }
        else
        {
                EugeneDisplayData data;
                data.length = kOrbitsRhythmMaxLength;
                data.hits = kOrbitsRhythmMaxLength / 2;
                data.shift = 0;
                data.current_step = 0;
                data.reverse = false;
                data.invert = false;
                return data;
        }
}

unsigned int RareBreeds_Orbits_Eugene::readLength()
{
        float value = getParam(LENGTH_KNOB_PARAM).getValue();
        float input = getInput(LENGTH_CV_INPUT).getVoltage();
        float normalized_input = input / 5.f;
        float attenuation = getParam(LENGTH_CV_KNOB_PARAM).getValue();
        value += attenuation * normalized_input * (kOrbitsRhythmMaxLength - 1);

        return clampRounded(value, 1, kOrbitsRhythmMaxLength);
}

unsigned int RareBreeds_Orbits_Eugene::readHits(unsigned int length)
{
        float value = getParam(HITS_KNOB_PARAM).getValue();
        float input = getInput(HITS_CV_INPUT).getVoltage();
        float normalized_input = input / 5.f;
        float attenuation = getParam(HITS_CV_KNOB_PARAM).getValue();
        value += attenuation * normalized_input;
        float hits_float = value * static_cast<float>(length);
        return clampRounded(hits_float, 0, length);
}

unsigned int RareBreeds_Orbits_Eugene::readShift(unsigned int length)
{
        float value = getParam(SHIFT_KNOB_PARAM).getValue();
        float input = getInput(SHIFT_CV_INPUT).getVoltage();
        float normalized_input = input / 5.f;
        float attenuation = getParam(SHIFT_CV_KNOB_PARAM).getValue();
        value += attenuation * normalized_input * (kOrbitsRhythmMaxLength - 1);
        return clampRounded(value, 0, kOrbitsRhythmMaxLength - 1) % length;
}

bool RareBreeds_Orbits_Eugene::readReverse()
{
        if(getInput(REVERSE_CV_INPUT).isConnected())
        {
                m_reverse_trigger.process(getInput(REVERSE_CV_INPUT).getVoltage());
                return m_reverse_trigger.isHigh();
        }
        else
        {
                return getParam(REVERSE_KNOB_PARAM).getValue() > 0.5f;
        }
}

bool RareBreeds_Orbits_Eugene::readInvert()
{
        if(getInput(INVERT_CV_INPUT).isConnected())
        {
                m_invert_trigger.process(getInput(INVERT_CV_INPUT).getVoltage());
                return m_invert_trigger.isHigh();
        }
        else
        {
                return getParam(INVERT_KNOB_PARAM).getValue() > 0.5f;
        }
}

void RareBreeds_Orbits_Eugene::process(const ProcessArgs &process_args)
{
        if(m_sync_trigger.process(getInput(SYNC_INPUT).getVoltage()))
        {
                m_steps.reset();
        }

        if(m_sync_button_trigger.process(getParam(SYNC_KNOB_PARAM).getValue() > 0.5f))
        {
                m_steps.reset();
        }

        if(m_random_trigger.process(getInput(RANDOM_CV_INPUT).getVoltage()))
        {
                RandomizeEvent e;
                onRandomize(e);
        }

        const bool reverse = readReverse();
        const bool invert = readInvert();
        m_reverse.store(reverse, std::memory_order_relaxed);
        m_invert.store(invert, std::memory_order_relaxed);

        if(m_clock_trigger.process(getInput(CLOCK_INPUT).getVoltage()))
        {
                auto length = readLength();
                auto hits = readHits(length);
                auto shift = readShift(length);

                const OrbitsStepCounter::Step step = m_steps.advance(length, reverse);
                m_eoc_generator.update(m_eoc, step.is_first, step.is_last);
                m_beat_generator.update(orbitsEuclideanBeat(length, hits, shift, step.index, invert));
        }

        getOutput(BEAT_OUTPUT).setVoltage(m_beat_generator.process(m_beat, process_args.sampleTime) ? 10.f : 0.f);
        getOutput(EOC_OUTPUT).setVoltage(m_eoc_generator.process(process_args.sampleTime) ? 10.f : 0.f);
}

json_t *RareBreeds_Orbits_Eugene::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "beat", m_beat.dataToJson());
                json_object_set_new(root, "eoc", m_eoc.dataToJson());
                json_object_set_new(root, "randomization_mask", json_integer(m_randomization_mask));

                m_widget_data.save(root);
        }
        return root;
}

void RareBreeds_Orbits_Eugene::dataFromJson(json_t *root)
{
        if(root)
        {
                m_beat.dataFromJson(json_object_get(root, "beat"));
                m_eoc.dataFromJson(json_object_get(root, "eoc"));
                m_randomization_mask = RANDOMIZE_ALL;
                json_load_unsigned_int(root, "randomization_mask", &m_randomization_mask);

                m_widget_data.load(root);
        }
}

void RareBreeds_Orbits_Eugene::onReset()
{
        m_steps.reset();
}

void RareBreeds_Orbits_Eugene::onRandomize(const RandomizeEvent & /*e*/)
{
        if(m_randomization_mask & (1 << RANDOMIZE_LENGTH))
        {
                float length = random::uniform() * kOrbitsRhythmMaxLength;
                getParamQuantity(LENGTH_KNOB_PARAM)->setImmediateValue(length);
        }

        if(m_randomization_mask & (1 << RANDOMIZE_LENGTH_CV))
        {
                getParamQuantity(LENGTH_CV_KNOB_PARAM)->setImmediateValue(random::uniform());
        }

        if(m_randomization_mask & (1 << RANDOMIZE_HITS))
        {
                getParamQuantity(HITS_KNOB_PARAM)->setImmediateValue(random::uniform());
        }

        if(m_randomization_mask & (1 << RANDOMIZE_HITS_CV))
        {
                getParamQuantity(HITS_CV_KNOB_PARAM)->setImmediateValue(random::uniform());
        }

        if(m_randomization_mask & (1 << RANDOMIZE_SHIFT))
        {
                float shift = random::uniform() * (kOrbitsRhythmMaxLength - 1);
                getParamQuantity(SHIFT_KNOB_PARAM)->setImmediateValue(shift);
        }

        if(m_randomization_mask & (1 << RANDOMIZE_SHIFT_CV))
        {
                getParamQuantity(SHIFT_CV_KNOB_PARAM)->setImmediateValue(random::uniform());
        }

        if(m_randomization_mask & (1 << RANDOMIZE_REVERSE))
        {
                bool reverse = (random::uniform() < 0.5f);
                getParamQuantity(REVERSE_KNOB_PARAM)->setImmediateValue(reverse);
        }

        if(m_randomization_mask & (1 << RANDOMIZE_INVERT))
        {
                bool invert = (random::uniform() < 0.5f);
                getParamQuantity(INVERT_KNOB_PARAM)->setImmediateValue(invert);
        }
}
