#include "PolygeneModule.hpp"
#include "PolygeneWidget.hpp"
#include "Rhythm.hpp"

static unsigned int clampRounded(float value, unsigned int min, unsigned int max)
{
        return clamp(int(value + 0.5f), min, max);
}

static void json_load_real(json_t *root, const char *param, float *result)
{
        json_t *obj = json_object_get(root, param);
        if(obj)
        {
                *result = json_real_value(obj);
        }
}

static void json_load_bool(json_t *root, const char *param, bool *result)
{
        json_t *obj = json_object_get(root, param);
        if(obj)
        {
                *result = json_boolean_value(obj);
        }
}

static void json_load_integer(json_t *root, const char *param, int *result)
{
        json_t *obj = json_object_get(root, param);
        if(obj)
        {
                *result = json_integer_value(obj);
        }
}

void RareBreeds_Orbits_Polygene::Channel::init(RareBreeds_Orbits_Polygene *module, int channel)
{
        m_current_step = 0;
        m_module = module;
        m_channel = channel;
        m_length = m_module->params[LENGTH_KNOB_PARAM].getValue();
        m_hits = m_module->params[HITS_KNOB_PARAM].getValue();
        m_shift = m_module->params[SHIFT_KNOB_PARAM].getValue();
        m_variation = m_module->params[VARIATION_KNOB_PARAM].getValue();
        m_reverse = false;
        m_invert = false;
}

bool RareBreeds_Orbits_Polygene::Channel::readReverse(void)
{
        if(m_module->inputs[REVERSE_CV_INPUT].isConnected())
        {
                m_reverse_trigger.process(m_module->inputs[REVERSE_CV_INPUT].getPolyVoltage(m_channel));
                return m_reverse_trigger.isHigh();
        }
        else
        {
                return m_reverse;
        }
}

bool RareBreeds_Orbits_Polygene::Channel::readInvert(void)
{
        if(m_module->inputs[INVERT_CV_INPUT].isConnected())
        {
                m_invert_trigger.process(m_module->inputs[INVERT_CV_INPUT].getPolyVoltage(m_channel));
                return m_invert_trigger.isHigh();
        }
        else
        {
                return m_invert;
        }
}

bool RareBreeds_Orbits_Polygene::Channel::isOnBeat(unsigned int length, unsigned int hits, unsigned int shift,
                                                   unsigned int variation, unsigned int beat, bool invert)
{
        return rhythm::nearEvenRhythmBeat(length, hits, variation, shift, beat) != invert;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readLength()
{
        auto cv = m_module->inputs[LENGTH_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_length = m_length + cv * (rhythm::max_length - 1);
        return clampRounded(f_length, 1, rhythm::max_length);
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readHits(unsigned int length)
{
        auto cv = m_module->inputs[HITS_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_hits = m_hits + cv;
        return clampRounded(f_hits * length, 0, length);
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readShift(unsigned int length)
{
        auto cv = m_module->inputs[SHIFT_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_shift = m_shift + cv * (rhythm::max_length - 1);
        return clampRounded(f_shift, 0, rhythm::max_length - 1) % length;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readVariation(unsigned int length, unsigned int hits)
{
        auto cv = m_module->inputs[VARIATION_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_variation = m_variation + cv;
        auto count = rhythm::numNearEvenRhythms(length, hits);
        return clampRounded(f_variation * (count - 1), 0, count - 1);
}

void RareBreeds_Orbits_Polygene::Channel::process(const ProcessArgs &args)
{
        // A rising clock edge means first play the current beat
        // then advance to the next step
        if(m_clock_trigger.process(m_module->inputs[CLOCK_INPUT].getPolyVoltage(m_channel)))
        {
                // Play the current beat
                // If the channel is reversing we want to play the beat prior to this
                // this ensures that a clock from the start of a rhythm plays the last
                // note in the rhythm when reversed rather than the first.
                auto length = readLength();
                auto hits = readHits(length);
                auto shift = readShift(length);
                auto invert = readInvert();
                auto variation = readVariation(length, hits);
                auto reverse = readReverse();

                // Avoid stepping out of bounds
                m_current_step = readStep(length);

                m_eoc_generator.update(m_module->m_eoc, m_current_step == 0,
                                        m_current_step == (reverse ? 1 : length - 1));

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

                m_beat_generator.update(isOnBeat(length, hits, shift, variation, m_current_step, invert));

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

        auto out = m_beat_generator.process(m_module->m_beat, args.sampleTime) ? 10.f : 0.f;
        m_module->outputs[BEAT_OUTPUT].setVoltage(out, m_channel);

        auto eoc_out = m_eoc_generator.process(args.sampleTime) ? 10.f : 0.f;
        m_module->outputs[EOC_OUTPUT].setVoltage(eoc_out, m_channel);
}

json_t *RareBreeds_Orbits_Polygene::Channel::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "length", json_real(m_length));
                json_object_set_new(root, "hits", json_real(m_hits));
                json_object_set_new(root, "shift", json_real(m_shift));
                json_object_set_new(root, "variation", json_real(m_variation));
                json_object_set_new(root, "reverse", json_boolean(m_reverse));
                json_object_set_new(root, "invert", json_boolean(m_invert));
        }
        return root;
}

void RareBreeds_Orbits_Polygene::Channel::dataFromJson(json_t *root)
{
        if(root)
        {
                json_load_real(root, "length", &m_length);
                json_load_real(root, "hits", &m_hits);
                json_load_real(root, "shift", &m_shift);
                json_load_real(root, "variation", &m_variation);
                json_load_bool(root, "reverse", &m_reverse);
                json_load_bool(root, "invert", &m_invert);
        }
}

void RareBreeds_Orbits_Polygene::Channel::onRandomize()
{
        m_length = random::uniform() * rhythm::max_length;
        m_hits = random::uniform();
        m_shift = random::uniform() * (rhythm::max_length - 1);
        m_variation = random::uniform();
        m_reverse = (random::uniform() < 0.5f);
        m_invert = (random::uniform() < 0.5f);
}

PolygeneDisplayData RareBreeds_Orbits_Polygene::getDisplayData(void)
{
        PolygeneDisplayData data;
        data.active_channel_id = m_active_channel_id;
        data.active_channels = m_active_channels;
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                auto ch = &data.channels[c];
                ch->length = m_channels[c].readLength();
                ch->hits = m_channels[c].readHits(ch->length);
                ch->shift = m_channels[c].readShift(ch->length);
                ch->variation = m_channels[c].readVariation(ch->length, ch->hits);
                ch->invert = m_channels[c].readInvert();
                ch->current_step = m_channels[c].readStep(ch->length);
        }
        return data;
}

PolygeneDisplayData RareBreeds_Orbits_Polygene::getDisplayData(RareBreeds_Orbits_Polygene *module)
{
        if(module)
        {
                return module->getDisplayData();
        }
        else
        {
                PolygeneDisplayData data = {3, PORT_MAX_CHANNELS,
                        {{32, 6, 0, 0, 0, 4},
                        {16, 9, 3, 0, 0, 0},
                        {7, 3, 6, 0, 0, 5},
                        {9, 7, 2, 0, 0, 3},
                        {17, 4, 12, 0, 0, 14},
                        {30, 20, 15, 0, 0, 20},
                        {19, 4, 14, 0, 0, 10},
                        {4, 2, 3, 0, 0, 2},
                        {7, 1, 0, 0, 0, 6},
                        {8, 5, 3, 0, 0, 1},
                        {16, 10, 4, 0, 0, 4},
                        {19, 16, 2, 0, 0, 18},
                        {24, 20, 8, 0, 0, 14},
                        {28, 14, 6, 0, 0, 20},
                        {21, 12, 4, 0, 0, 4},
                        {9, 3, 3, 0, 0, 7}}};
                return data;
        }
}

RareBreeds_Orbits_Polygene::RareBreeds_Orbits_Polygene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(CHANNEL_KNOB_PARAM, 0.f, 15.f, 0.f, "Channel", "", 0.f, 1.f, 1.f);
        configParam(LENGTH_KNOB_PARAM, 1.f, rhythm::max_length, rhythm::max_length, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, rhythm::max_length - 1, 0.f, "Shift");
        configParam(VARIATION_KNOB_PARAM, 0.f, 1.f, 0.0f, "Variation", "%", 0.f, 100.f);
        configSwitch(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});
        configSwitch(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert", {"Off", "On"});
        configButton(RANDOM_KNOB_PARAM, "Randomize channel");
        configButton(SYNC_KNOB_PARAM, "Sync");

        configInput(CLOCK_INPUT, "Clock");
        configInput(SYNC_INPUT, "Sync");
        configInput(LENGTH_CV_INPUT, "Length CV");
        configInput(HITS_CV_INPUT, "Hits CV");
        configInput(SHIFT_CV_INPUT, "Shift CV");
        configInput(VARIATION_CV_INPUT, "Variation CV");
        configInput(REVERSE_CV_INPUT, "Reverse CV");
        configInput(INVERT_CV_INPUT, "Invert CV");

        configOutput(BEAT_OUTPUT, "Beat");
        configOutput(EOC_OUTPUT, "End of cycle");

        configBypass(CLOCK_INPUT, BEAT_OUTPUT);

        reset();
}

void RareBreeds_Orbits_Polygene::reset()
{
        m_previous_channel_id = PORT_MAX_CHANNELS;
        m_active_channel_id = 0;
        m_active_channel = &m_channels[m_active_channel_id];

        m_length = params[LENGTH_KNOB_PARAM].getValue();
        m_hits = params[HITS_KNOB_PARAM].getValue();
        m_shift = params[SHIFT_KNOB_PARAM].getValue();
        m_variation = params[VARIATION_KNOB_PARAM].getValue();

        for(auto i = 0u; i < PORT_MAX_CHANNELS; ++i)
        {
                m_channels[i].init(this, i);
        }
}

void RareBreeds_Orbits_Polygene::syncParamsToActiveChannel()
{
        params[LENGTH_KNOB_PARAM].setValue(m_active_channel->m_length);
        params[HITS_KNOB_PARAM].setValue(m_active_channel->m_hits);
        params[SHIFT_KNOB_PARAM].setValue(m_active_channel->m_shift);
        params[VARIATION_KNOB_PARAM].setValue(m_active_channel->m_variation);
        params[REVERSE_KNOB_PARAM].setValue(m_active_channel->m_reverse);
        params[INVERT_KNOB_PARAM].setValue(m_active_channel->m_invert);
}

void RareBreeds_Orbits_Polygene::process(const ProcessArgs &args)
{
        m_active_channels = inputs[CLOCK_INPUT].getChannels();
        outputs[BEAT_OUTPUT].setChannels(m_active_channels);
        outputs[EOC_OUTPUT].setChannels(m_active_channels);

        m_active_channel_id = std::round(params[CHANNEL_KNOB_PARAM].getValue());
        m_active_channel = &m_channels[m_active_channel_id];

        // Update the knob positions when the channel changes
        if(m_previous_channel_id != m_active_channel_id)
        {
                syncParamsToActiveChannel();
                m_previous_channel_id = m_active_channel_id;
        }

        m_active_channel->m_length = params[LENGTH_KNOB_PARAM].getValue();
        m_active_channel->m_hits = params[HITS_KNOB_PARAM].getValue();
        m_active_channel->m_shift = params[SHIFT_KNOB_PARAM].getValue();
        m_active_channel->m_variation = params[VARIATION_KNOB_PARAM].getValue();

        m_reverse_trigger.process(params[REVERSE_KNOB_PARAM].getValue() > 0.5f);
        m_active_channel->m_reverse = m_reverse_trigger.state;

        m_invert_trigger.process(params[INVERT_KNOB_PARAM].getValue() > 0.5f);
        m_active_channel->m_invert = m_invert_trigger.state;

        bool rnd = params[RANDOM_KNOB_PARAM].getValue() > 0.5f;
        if(m_random_trigger.process(rnd, args.sampleTime))
        {
                m_active_channel->onRandomize();
                syncParamsToActiveChannel();
        }

        bool any_sync = false;
        int sync_channels = inputs[SYNC_INPUT].getChannels();
        for(int i = 0; i < sync_channels; ++i)
        {
                if(m_channels[i].m_sync_trigger.process(inputs[SYNC_INPUT].getPolyVoltage(i)))
                {
                        m_channels[i].m_current_step = 0;
                        any_sync = true;
                }
        }

        if(m_sync_trigger.process(params[SYNC_KNOB_PARAM].getValue() > 0.5f) ||
          (SYNC_MODE_ALL_CHANNELS == m_sync_mode && any_sync))
        {
                for(auto &chan : m_channels)
                {
                        chan.m_current_step = 0;
                }
        }

        int clock_channels = inputs[CLOCK_INPUT].getChannels();
        for(int i = 0; i < clock_channels; ++i)
        {
                m_channels[i].process(args);
        }
}

json_t *RareBreeds_Orbits_Polygene::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "length", json_real(m_length));
                json_object_set_new(root, "hits", json_real(m_hits));
                json_object_set_new(root, "shift", json_real(m_shift));
                json_object_set_new(root, "variation", json_real(m_variation));
                json_object_set_new(root, "beat", m_beat.dataToJson());
                json_object_set_new(root, "eoc", m_eoc.dataToJson());
                json_object_set_new(root, "sync", json_integer(m_sync_mode));
                json_object_set_new(root, "active_channel_id", json_integer(m_active_channel_id));

                json_t *channels = json_array();
                if(channels)
                {
                        for(auto &chan : m_channels)
                        {
                                json_t *channel_json = chan.dataToJson();
                                if(channel_json)
                                {
                                        json_array_append_new(channels, channel_json);
                                }
                        }

                        json_object_set_new(root, "channels", channels);
                }

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

void RareBreeds_Orbits_Polygene::dataFromJson(json_t *root)
{
        if(root)
        {
                json_load_real(root, "length", &m_length);
                json_load_real(root, "hits", &m_hits);
                json_load_real(root, "shift", &m_shift);
                json_load_real(root, "variation", &m_variation);
                m_beat.dataFromJson(json_object_get(root, "beat"));
                m_eoc.dataFromJson(json_object_get(root, "eoc"));
                int sync_mode = SYNC_MODE_INDIVIDUAL_CHANNELS;
                json_load_integer(root, "sync", &sync_mode);
                m_sync_mode = (SyncMode) sync_mode;
                json_load_integer(root, "active_channel_id", &m_active_channel_id);
                json_t *channels = json_object_get(root, "channels");
                if(channels)
                {
                        for(auto i = 0u; i < PORT_MAX_CHANNELS; ++i)
                        {
                                json_t *channel = json_array_get(channels, i);
                                if(channel)
                                {
                                        m_channels[i].dataFromJson(channel);
                                }
                        }
                }

                if(m_widget)
                {
                        json_t *obj = json_object_get(root, "widget");
                        if(obj)
                        {
                                m_widget->dataFromJson(obj);
                        }
                }
        }
}

void RareBreeds_Orbits_Polygene::onRandomize(const RandomizeEvent& e)
{
        for(auto &chan : m_channels)
        {
                chan.onRandomize();
        }

        // Update the parameters so they reflect the active channels randomized parameters
        syncParamsToActiveChannel();
}

void RareBreeds_Orbits_Polygene::onReset(const ResetEvent& e)
{
        Module::onReset(e);
        reset();
}
