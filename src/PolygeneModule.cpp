#include "PolygeneModule.hpp"
#include "Euclidean.hpp"
#include "PolygeneWidget.hpp"

// TODO: Library function
static unsigned int clampRounded(float value, unsigned int min, unsigned int max)
{
        return clamp(int(std::round(value)), min, max);
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
        return euclidean::nearEvenRhythmBeat(length, hits, variation, shift, beat) != invert;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readLength()
{
        auto cv = m_module->inputs[LENGTH_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_length = m_length + cv * (euclidean::max_length - 1);
        return clampRounded(f_length, 1, euclidean::max_length);
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
        auto f_shift = m_shift + cv * (euclidean::max_length - 1);
        return clampRounded(f_shift, 0, euclidean::max_length - 1) % length;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readVariation(unsigned int length, unsigned int hits)
{
        auto cv = m_module->inputs[VARIATION_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
        auto f_variation = m_variation + cv;
        auto count = euclidean::numNearEvenRhythms(length, hits);
        return clampRounded(f_variation * (count - 1), 0, count - 1);
}

void RareBreeds_Orbits_Polygene::Channel::process(const ProcessArgs &args)
{
        auto length = readLength();

        // Avoid stepping out of bounds
        if(m_current_step >= length)
        {
                m_current_step = 0;
        }

        if(m_module->inputs[SYNC_INPUT].getChannels() > m_channel)
        {
                // A rising edge of the sync input tells the module to set the current step to 0
                if(m_sync_trigger.process(m_module->inputs[SYNC_INPUT].getPolyVoltage(m_channel)))
                {
                        m_current_step = 0;
                }
        }

        // A rising clock edge means first play the current beat
        // then advance to the next step
        if(m_module->inputs[CLOCK_INPUT].getChannels() > m_channel &&
           m_clock_trigger.process(m_module->inputs[CLOCK_INPUT].getPolyVoltage(m_channel)))
        {
                // Play the current beat
                // If the channel is reversing we want to play the beat prior to this
                // this ensures that a clock from the start of a rhythm plays the last
                // note in the rhythm when reversed rather than the first.
                auto hits = readHits(length);
                auto shift = readShift(length);
                auto invert = readInvert();
                auto variation = readVariation(length, hits);
                auto reverse = readReverse();

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

                if(isOnBeat(length, hits, shift, variation, m_current_step, invert))
                {
                        m_output_generator.trigger(1e-3f);
                }

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

        auto out = m_output_generator.process(args.sampleTime) ? 10.f : 0.f;
        m_module->outputs[BEAT_OUTPUT].setVoltage(out, m_channel);
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
        m_length = random::uniform() * euclidean::max_length;
        m_hits = random::uniform();
        m_shift = random::uniform() * (euclidean::max_length - 1);
        m_variation = random::uniform();
        m_reverse = (random::uniform() < 0.5f);
        m_invert = (random::uniform() < 0.5f);
}

RareBreeds_Orbits_Polygene::RareBreeds_Orbits_Polygene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CHANNEL_KNOB_PARAM, 0.f, 15.f, 0.f, "Channel", "", 0.f, 1.f, 1.f);
        configParam(LENGTH_KNOB_PARAM, 1.f, euclidean::max_length, euclidean::max_length, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, euclidean::max_length - 1, 0.f, "Shift");
        configParam(VARIATION_KNOB_PARAM, 0.f, 1.f, 0.0f, "Variation", "%", 0.f, 100.f);
        configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
        configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");
        configParam(RANDOM_KNOB_PARAM, 0.f, 1.f, 0.f, "Random");
        configParam(SYNC_KNOB_PARAM, 0.f, 1.f, 0.f, "Sync");

        reset();
}

void RareBreeds_Orbits_Polygene::reset()
{
        m_previous_channel_id = max_channels;
        m_active_channel_id = 0;
        m_active_channel = &m_channels[m_active_channel_id];

        m_length = params[LENGTH_KNOB_PARAM].getValue();
        m_hits = params[HITS_KNOB_PARAM].getValue();
        m_shift = params[SHIFT_KNOB_PARAM].getValue();
        m_variation = params[VARIATION_KNOB_PARAM].getValue();

        for(auto i = 0u; i < max_channels; ++i)
        {
                m_channels[i].init(this, i);
        }
}

void RareBreeds_Orbits_Polygene::process(const ProcessArgs &args)
{
        m_active_channels = inputs[CLOCK_INPUT].getChannels();
        outputs[BEAT_OUTPUT].setChannels(m_active_channels);

        m_active_channel_id = std::round(params[CHANNEL_KNOB_PARAM].getValue());
        m_active_channel = &m_channels[m_active_channel_id];

        // Update the SVGs on the reverse and invert buttons when the channel changes
        if(m_previous_channel_id != m_active_channel_id)
        {
                params[REVERSE_KNOB_PARAM].setValue(m_active_channel->m_reverse);
                params[INVERT_KNOB_PARAM].setValue(m_active_channel->m_invert);
                m_previous_channel_id = m_active_channel_id;
        }

        float length = params[LENGTH_KNOB_PARAM].getValue();
        if(length != m_length)
        {
                m_active_channel->m_length = length;
                m_length = length;
        }

        float hits = params[HITS_KNOB_PARAM].getValue();
        if(hits != m_hits)
        {
                m_active_channel->m_hits = hits;
                m_hits = hits;
        }

        float shift = params[SHIFT_KNOB_PARAM].getValue();
        if(shift != m_shift)
        {
                m_active_channel->m_shift = shift;
                m_shift = shift;
        }

        float variation = params[VARIATION_KNOB_PARAM].getValue();
        if(variation != m_variation)
        {
                m_active_channel->m_variation = variation;
                m_variation = variation;
        }

        reverse_trigger.process(std::round(params[REVERSE_KNOB_PARAM].getValue()));
        m_active_channel->m_reverse = reverse_trigger.state;

        invert_trigger.process(std::round(params[INVERT_KNOB_PARAM].getValue()));
        m_active_channel->m_invert = invert_trigger.state;

        if(random_trigger.process(std::round(params[RANDOM_KNOB_PARAM].getValue())))
        {
                m_active_channel->onRandomize();
                params[REVERSE_KNOB_PARAM].setValue(m_active_channel->m_reverse);
                params[INVERT_KNOB_PARAM].setValue(m_active_channel->m_invert);
        }

        if(sync_trigger.process(std::round(params[SYNC_KNOB_PARAM].getValue())))
        {
                for(auto &chan : m_channels)
                {
                        chan.m_current_step = 0;
                }
        }

        for(auto &chan : m_channels)
        {
                chan.process(args);
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

                if(widget)
                {
                        json_t *w = widget->dataToJson();
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
                json_load_integer(root, "active_channel_id", &m_active_channel_id);
                json_t *channels = json_object_get(root, "channels");
                if(channels)
                {
                        for(auto i = 0u; i < max_channels; ++i)
                        {
                                json_t *channel = json_array_get(channels, i);
                                if(channel)
                                {
                                        m_channels[i].dataFromJson(channel);
                                }
                        }
                }

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

void RareBreeds_Orbits_Polygene::onRandomize()
{
        for(auto &chan : m_channels)
        {
                chan.onRandomize();
        }

        // Parameters have already been randomised by VCV rack
        // But then the active channel controlled by those parameters has been randomised again
        // Update the parameters so they reflect the active channels randomised parameters
        params[LENGTH_KNOB_PARAM].setValue(m_active_channel->m_length);
        params[HITS_KNOB_PARAM].setValue(m_active_channel->m_hits);
        params[SHIFT_KNOB_PARAM].setValue(m_active_channel->m_shift);
        params[VARIATION_KNOB_PARAM].setValue(m_active_channel->m_variation);
}

void RareBreeds_Orbits_Polygene::onReset()
{
        reset();
}
