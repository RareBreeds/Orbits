#include "EugeneModule.hpp"
#include "EugeneWidget.hpp"
#include "Rhythm.hpp"

static unsigned int clampRounded(float value, unsigned int min, unsigned int max)
{
        return std::max(std::min((unsigned int)(value + 0.5f), max), min);
}

static void json_load_integer(json_t *root, const char *param, int *result)
{
        json_t *obj = json_object_get(root, param);
        if(obj)
        {
                *result = json_integer_value(obj);
        }
}

RareBreeds_Orbits_Eugene::RareBreeds_Orbits_Eugene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(LENGTH_KNOB_PARAM, 1.f, rhythm::max_length, rhythm::max_length, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, rhythm::max_length - 1, 0.f, "Shift");
        configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
        configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
        configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
        configSwitch(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});
        configSwitch(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert", {"Off", "On"});
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

EugeneDisplayData RareBreeds_Orbits_Eugene::getDisplayData(void)
{
        EugeneDisplayData data;
        data.length = readLength();
        data.hits = readHits(data.length);
        data.shift = readShift(data.length);
        data.reverse = readReverse();
        data.invert = readInvert();
        data.current_step = m_current_step > data.length - 1 ? 0 : m_current_step;
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
                data.length = rhythm::max_length;
                data.hits = rhythm::max_length / 2;
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
        value += attenuation * normalized_input * (rhythm::max_length - 1);

        return clampRounded(value, 1, rhythm::max_length);
}

unsigned int RareBreeds_Orbits_Eugene::readHits(unsigned int length)
{
        float value = getParam(HITS_KNOB_PARAM).getValue();
        float input = getInput(HITS_CV_INPUT).getVoltage();
        float normalized_input = input / 5.f;
        float attenuation = getParam(HITS_CV_KNOB_PARAM).getValue();
        value += attenuation * normalized_input;
        float hits_float = value * length;
        return clampRounded(hits_float, 0, length);
}

unsigned int RareBreeds_Orbits_Eugene::readShift(unsigned int length)
{
        float value = getParam(SHIFT_KNOB_PARAM).getValue();
        float input = getInput(SHIFT_CV_INPUT).getVoltage();
        float normalized_input = input / 5.f;
        float attenuation = getParam(SHIFT_CV_KNOB_PARAM).getValue();
        value += attenuation * normalized_input * (rhythm::max_length - 1);
        return clampRounded(value, 0, rhythm::max_length - 1) % length;
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

void RareBreeds_Orbits_Eugene::process(const ProcessArgs &args)
{
        if(m_sync_trigger.process(getInput(SYNC_INPUT).getVoltage()))
        {
                m_current_step = 0;
        }

        if(m_sync_button_trigger.process(getParam(SYNC_KNOB_PARAM).getValue() > 0.5f))
        {
                m_current_step = 0;
        }

        if(m_random_trigger.process(getInput(RANDOM_CV_INPUT).getVoltage()))
        {
                RandomizeEvent e;
                onRandomize(e);
        }

        if(m_clock_trigger.process(getInput(CLOCK_INPUT).getVoltage()))
        {
                auto length = readLength();
                auto hits = readHits(length);
                auto shift = readShift(length);
                auto reverse = readReverse();
                auto invert = readInvert();

                if(m_current_step >= length)
                {
                        m_current_step = 0;
                }

                if(reverse)
                {
                        if(m_current_step == 0)
                        {
                                m_current_step = length - 1;
                        }
                        else
                        {
                                --m_current_step;
                        }
                }

                // If we're going forwards, first is 0, last is length - 1
                // If we're going backwards, first is length - 1, last is 0
                unsigned int first = reverse ? length - 1 : 0;
                unsigned int last = reverse ? 0 : length - 1;
                m_eoc_generator.update(m_eoc, m_current_step == first, m_current_step == last);

                m_beat_generator.update(rhythm::beat(length, hits, shift, m_current_step, invert));

                if(!reverse)
                {
                        if(m_current_step == length - 1)
                        {
                                m_current_step = 0;
                        }
                        else
                        {
                                ++m_current_step;
                        }
                }
        }

        getOutput(BEAT_OUTPUT).setVoltage(m_beat_generator.process(m_beat, args.sampleTime) ? 10.f : 0.f);
        getOutput(EOC_OUTPUT).setVoltage(m_eoc_generator.process(args.sampleTime) ? 10.f : 0.f);
}

json_t *RareBreeds_Orbits_Eugene::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "beat", m_beat.dataToJson());
                json_object_set_new(root, "eoc", m_eoc.dataToJson());
                json_object_set_new(root, "randomization_mask", json_integer(m_randomization_mask));

                if(m_widget)
                {
                        json_t *w = m_widget->dataToJson();
                        if(w)
                        {
                                json_object_set_new(root, "widget", w);
                        }
                }
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
                json_load_integer(root, "randomization_mask", &m_randomization_mask);

                json_t *obj = json_object_get(root, "widget");
                if(obj)
                {
                        // Always defer the widget config to the widget draw method for 2 reasons
                        // 1. The widget isn't created before dataFromJson is called in Rack-2
                        //    so store the config and initialize when it's first used.
                        // 2. EightFaceMk2 calls dataFromJson from a different thread than the
                        //    main thread which may cause GL operations to fail.
                        // Note the module must remember to decref this if the widget is never created
                        m_widget_config = json_incref(obj);
                }
        }
}

void RareBreeds_Orbits_Eugene::onReset()
{
        m_current_step = 0;
}

RareBreeds_Orbits_Eugene::~RareBreeds_Orbits_Eugene()
{
        json_decref(m_widget_config);
}

void RareBreeds_Orbits_Eugene::onRandomize(const RandomizeEvent& e)
{
        (void) e;

        if (m_randomization_mask & (1 << RANDOMIZE_LENGTH))
        {
                float length = random::uniform() * rhythm::max_length;
                getParam(LENGTH_KNOB_PARAM).setValue(length);
        }

        if (m_randomization_mask & (1 << RANDOMIZE_LENGTH_CV))
        {
                getParam(LENGTH_CV_KNOB_PARAM).setValue(random::uniform());
        }

        if (m_randomization_mask & (1 << RANDOMIZE_HITS))
        {
                float hits = random::uniform();
                getParam(HITS_KNOB_PARAM).setValue(hits);
        }

        if (m_randomization_mask & (1 << RANDOMIZE_HITS_CV))
        {
                getParam(HITS_CV_KNOB_PARAM).setValue(random::uniform());
        }

        if (m_randomization_mask & (1 << RANDOMIZE_SHIFT))
        {
                float shift = random::uniform() * (rhythm::max_length - 1);
                getParam(SHIFT_KNOB_PARAM).setValue(shift);
        }

        if (m_randomization_mask & (1 << RANDOMIZE_SHIFT_CV))
        {
                getParam(SHIFT_CV_KNOB_PARAM).setValue(random::uniform());
        }

        if (m_randomization_mask & (1 << RANDOMIZE_REVERSE))
        {
                bool reverse = (random::uniform() < 0.5f);
                getParam(REVERSE_KNOB_PARAM).setValue(reverse);
        }

        if (m_randomization_mask & (1 << RANDOMIZE_INVERT))
        {
                bool invert = (random::uniform() < 0.5f);
                getParam(INVERT_KNOB_PARAM).setValue(invert);
        }
}
