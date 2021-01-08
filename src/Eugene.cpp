#include <cstdint>
#include <string>

#if ORBITS_DEBUG
#include <cassert>
#endif // ORBITS_DEBUG

#include "plugin.hpp"

const auto max_rhythm_length = 32;

// When the number of hits is 0 or 1 the rhythm is always 0x0 or 0x1
// respectively, no need to store them in the look up table.
// When the number of hits is equal to the length we know all beats
// are on, so also no need to store them in the look up table.
static const uint8_t euclidean_rhythm_u8[] = {
        // Y = Beat Length - 3
        // X = Number of on-beats - 2
        /* 3 */ 0x5u,
        /* 4 */ 0x5u,  0xdu,
        /* 5 */ 0x9u,  0xdu,  0x1du,
        /* 6 */ 0x9u,  0x15u, 0x2du, 0x3du,
        /* 7 */ 0x11u, 0x29u, 0x2du, 0x5du, 0x7du,
        /* 8 */ 0x11u, 0x29u, 0x55u, 0xadu, 0xddu, 0xfdu
};

static const uint16_t euclidean_rhythm_u16[] = {
        // Y = Beat Length - 9
        // X = Number of on-beats - 2
        /* 9  */ 0x21u,  0x49u,  0xa9u,   0xadu,   0x16du,  0x1bdu,  0x1fdu,
        /* 10 */ 0x21u,  0x91u,  0x129u,  0x155u,  0x1adu,  0x2ddu,  0x3bdu,  0x3fdu,
        /* 11 */ 0x41u,  0x91u,  0x129u,  0x2a9u,  0x2adu,  0x5adu,  0x6ddu,  0x77du,  0x7fdu,
        /* 12 */ 0x41u,  0x111u, 0x249u,  0x4a9u,  0x555u,  0x6adu,  0xb6du,  0xdddu,  0xf7du,  0xffdu,
        /* 13 */ 0x81u,  0x221u, 0x491u,  0x929u,  0xaa9u,  0xaadu,  0xdadu,  0x16ddu, 0x1bbdu, 0x1efdu, 0x1ffdu,
        /* 14 */ 0x81u,  0x221u, 0x891u,  0x929u,  0x14a9u, 0x1555u, 0x16adu, 0x2dadu, 0x2eddu, 0x3bbdu, 0x3efdu, 0x3ffdu,
        /* 15 */ 0x101u, 0x421u, 0x891u,  0x1249u, 0x2529u, 0x2aa9u, 0x2aadu, 0x35adu, 0x5b6du, 0x6eddu, 0x77bdu, 0x7dfdu, 0x7ffdu,
        /* 16 */ 0x101u, 0x841u, 0x1111u, 0x2491u, 0x2929u, 0x52a9u, 0x5555u, 0x5aadu, 0xadadu, 0xb6ddu, 0xddddu, 0xef7du, 0xfdfdu, 0xfffdu
};

static const uint32_t euclidean_rhythm_u32[] = {
        // Y = Beat Length - 17
        // X = Number of on-beats - 2
        /* 17 */ 0x201u,   0x841u,    0x2221u,    0x4491u,    0x4929u,    0x94a9u,     0xaaa9u,     0xaaadu,     0xd6adu,     0x16dadu,    0x176ddu,    0x1bbbdu,    0x1ef7du,    0x1fbfdu,    0x1fffdu,
        /* 18 */ 0x201u,   0x1041u,   0x4221u,    0x8891u,    0x9249u,    0x12929u,    0x152a9u,    0x15555u,    0x15aadu,    0x1adadu,    0x2db6du,    0x2eeddu,    0x37bbdu,    0x3df7du,    0x3fbfdu,    0x3fffdu,
        /* 19 */ 0x401u,   0x2081u,   0x4221u,    0x8891u,    0x12491u,   0x14929u,    0x294a9u,    0x2aaa9u,    0x2aaadu,    0x2d6adu,    0x56dadu,    0x5b6ddu,    0x6eeddu,    0x77bbdu,    0x7befdu,    0x7f7fdu,    0x7fffdu,
        /* 20 */ 0x401u,   0x2081u,   0x8421u,    0x11111u,   0x24491u,   0x24929u,    0x4a529u,    0x54aa9u,    0x55555u,    0x56aadu,    0x6b5adu,    0xb6dadu,    0xb76ddu,    0xdddddu,    0xef7bdu,    0xfbefdu,    0xff7fdu,    0xffffdu,
        /* 21 */ 0x801u,   0x4081u,   0x10841u,   0x22221u,   0x44891u,   0x49249u,    0x52929u,    0xa54a9u,    0xaaaa9u,    0xaaaadu,    0xb56adu,    0x15adadu,   0x16db6du,   0x176eddu,   0x1bbbbdu,   0x1def7du,   0x1f7efdu,   0x1feffdu,   0x1ffffdu,
        /* 22 */ 0x801u,   0x8101u,   0x20841u,   0x42221u,   0x48891u,   0x92491u,    0x94929u,    0x1294a9u,   0x154aa9u,   0x155555u,   0x156aadu,   0x1ad6adu,   0x2d6dadu,   0x2db6ddu,   0x36eeddu,   0x37bbbdu,   0x3bef7du,   0x3efdfdu,   0x3feffdu,   0x3ffffdu,
        /* 23 */ 0x1001u,  0x8101u,   0x20841u,   0x84221u,   0x88891u,   0x122491u,   0x124929u,   0x252929u,   0x2952a9u,   0x2aaaa9u,   0x2aaaadu,   0x2d5aadu,   0x35adadu,   0x5b6dadu,   0x5bb6ddu,   0x6eeeddu,   0x6f7bbdu,   0x7bef7du,   0x7efdfdu,   0x7fdffdu,   0x7ffffdu,
        /* 24 */ 0x1001u,  0x10101u,  0x41041u,   0x84221u,   0x111111u,  0x224491u,   0x249249u,   0x292929u,   0x4a94a9u,   0x552aa9u,   0x555555u,   0x55aaadu,   0x6ad6adu,   0xadadadu,   0xb6db6du,   0xbb76ddu,   0xddddddu,   0xef7bbdu,   0xf7df7du,   0xfdfdfdu,   0xffdffdu,   0xfffffdu,
        /* 25 */ 0x2001u,  0x20201u,  0x82081u,   0x108421u,  0x222221u,  0x448891u,   0x492491u,   0x4a4929u,   0x94a529u,   0xa952a9u,   0xaaaaa9u,   0xaaaaadu,   0xad5aadu,   0xd6b5adu,   0x16b6dadu,  0x16db6ddu,  0x176eeddu,  0x1bbbbbdu,  0x1def7bdu,  0x1efbefdu,  0x1fbfbfdu,  0x1ffbffdu,  0x1fffffdu,
        /* 26 */ 0x2001u,  0x20201u,  0x102081u,  0x210841u,  0x442221u,  0x488891u,   0x922491u,   0x924929u,   0x1252929u,  0x14a94a9u,  0x1552aa9u,  0x1555555u,  0x155aaadu,  0x16ad6adu,  0x1b5adadu,  0x2db6dadu,  0x2dbb6ddu,  0x36eeeddu,  0x377bbbdu,  0x3bdef7du,  0x3dfbefdu,  0x3fbfbfdu,  0x3ffbffdu,  0x3fffffdu,
        /* 27 */ 0x4001u,  0x40201u,  0x102081u,  0x410841u,  0x844221u,  0x888891u,   0x1224491u,  0x1249249u,  0x1494929u,  0x25294a9u,  0x2a552a9u,  0x2aaaaa9u,  0x2aaaaadu,  0x2b55aadu,  0x35ad6adu,  0x56d6dadu,  0x5b6db6du,  0x5bb76ddu,  0x6eeeeddu,  0x6f77bbdu,  0x77def7du,  0x7dfbefdu,  0x7f7fbfdu,  0x7ff7ffdu,  0x7fffffdu,
        /* 28 */ 0x4001u,  0x80401u,  0x204081u,  0x820841u,  0x884221u,  0x1111111u,  0x2244891u,  0x2492491u,  0x24a4929u,  0x4a52929u,  0x52a54a9u,  0x554aaa9u,  0x5555555u,  0x556aaadu,  0x5ab56adu,  0x6b5adadu,  0xb6b6dadu,  0xb6db6ddu,  0xbb76eddu,  0xdddddddu,  0xeef7bbdu,  0xefbef7du,  0xfbf7efdu,  0xfeff7fdu,  0xfff7ffdu,  0xffffffdu,
        /* 29 */ 0x8001u,  0x80401u,  0x408101u,  0x820841u,  0x1084221u, 0x2222221u,  0x2448891u,  0x4912491u,  0x4924929u,  0x5252929u,  0x95294a9u,  0xa954aa9u,  0xaaaaaa9u,  0xaaaaaadu,  0xad56aadu,  0xd5ad6adu,  0x15b5adadu, 0x16db6dadu, 0x16ddb6ddu, 0x1b76eeddu, 0x1bbbbbbdu, 0x1def7bbdu, 0x1efbef7du, 0x1f7efdfdu, 0x1feff7fdu, 0x1ffefffdu, 0x1ffffffdu,
        /* 30 */ 0x8001u,  0x100401u, 0x808101u,  0x1041041u, 0x2108421u, 0x4422221u,  0x4488891u,  0x9124491u,  0x9249249u,  0x9494929u,  0x1294a529u, 0x14a952a9u, 0x1554aaa9u, 0x15555555u, 0x1556aaadu, 0x16ad5aadu, 0x1ad6b5adu, 0x2d6d6dadu, 0x2db6db6du, 0x2ddb76ddu, 0x376eeeddu, 0x377bbbbdu, 0x3bdef7bdu, 0x3df7df7du, 0x3efefdfdu, 0x3fdff7fdu, 0x3ffefffdu, 0x3ffffffdu,
        /* 31 */ 0x10001u, 0x200801u, 0x808101u,  0x2082081u, 0x4210841u, 0x8442221u,  0x8888891u,  0x11224491u, 0x12492491u, 0x12524929u, 0x24a52929u, 0x254a94a9u, 0x2a954aa9u, 0x2aaaaaa9u, 0x2aaaaaadu, 0x2ad56aadu, 0x356ad6adu, 0x36b5adadu, 0x5b5b6dadu, 0x5b6db6ddu, 0x5dbb76ddu, 0x6eeeeeddu, 0x6f77bbbdu, 0x77bdef7du, 0x7befbefdu, 0x7efefdfdu, 0x7fbfeffdu, 0x7ffdfffdu, 0x7ffffffdu,
        /* 32 */ 0x10001u, 0x200801u, 0x1010101u, 0x4082081u, 0x8410841u, 0x10884221u, 0x11111111u, 0x22448891u, 0x24912491u, 0x24924929u, 0x29292929u, 0x4a5294a9u, 0x52a952a9u, 0x5552aaa9u, 0x55555555u, 0x555aaaadu, 0x5aad5aadu, 0x6b5ad6adu, 0xadadadadu, 0xb6db6dadu, 0xb6ddb6ddu, 0xbb76eeddu, 0xddddddddu, 0xdeef7bbdu, 0xef7def7du, 0xf7efbefdu, 0xfdfdfdfdu, 0xffbfeffdu, 0xfffdfffdu, 0xfffffffdu
};

static uint32_t sumTo(unsigned int n)
{
        return n * (n + 1) / 2;
}

static uint32_t rotL(uint32_t val, uint32_t size, uint32_t amount)
{
        return (val << amount | val >> (size - amount));
}

static unsigned int euclideanRhythmIndex(unsigned int length, unsigned int on_beats_m2, unsigned int first_entry_length)
{
        unsigned int first_entry_elements = first_entry_length - 2;
        unsigned int length_off = length - first_entry_length;
        return length_off * first_entry_elements + sumTo(length_off - 1) + on_beats_m2;
}

static uint32_t euclideanRhythm(unsigned int length, unsigned int on_beats)
{
#if ORBITS_DEBUG
        assert(length >= 1);
        assert(length <= 32);
        assert(on_beats <= length);
#endif // ORBITS_DEBUG

        // All euclidean rhythms with on beats of 0 or 1 are just 0 or 1 respectively.
        if(on_beats <= 1)
        {
                return on_beats;
        }

        // All beats are on beats when on_beats == length.
        if(on_beats == length)
        {
                // Technically the top bits should be 0's as they are not on beats.
                // i.e. return (~0U) >> (32 - length);
                // But we don't access bits outside the length of the rhythm so we can set them to whatever we want.
                return ~0;
        }

        // On beats 0 and 1 aren't stored in the look up table.
        unsigned int on_beats_m2 = on_beats - 2;
        // Rhythms are stored in the smallest native type available.
        if(length <= 8)
        {
                // Rhythm lengths 0, 1 and 2 aren't stored in the look up table.
                unsigned int length_m3 = length - 3;

                // Index for this length starts at the sum of lengths up to this
                // number, then add the number of on beats.
                unsigned int index = sumTo(length_m3) + on_beats_m2;

                return (uint32_t)euclidean_rhythm_u8[index];
        }
        else if(length <= 16)
        {
                // Rhythm lengths less than 9 aren't stored in the look up table.
                unsigned int index = euclideanRhythmIndex(length, on_beats_m2, 9);
                return (uint32_t)euclidean_rhythm_u16[index];
        }
        else // length <= 32
        {
                // Rhythm lengths less than 17 aren't stored in the look up table.
                unsigned int index = euclideanRhythmIndex(length, on_beats_m2, 17);
                return euclidean_rhythm_u32[index];
        }
}

static unsigned int clampRounded(float value, unsigned int min, unsigned int max)
{
        return clamp(int(std::round(value)), min, max);
}

static const int max_channels = 16;

struct RareBreeds_Orbits_Eugene : Module
{
        enum ParamIds
        {
		CHANNEL_NEXT_PARAM,
		CHANNEL_PREV_PARAM,
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
                NUM_OUTPUTS
        };
        enum LightIds
        {
                NUM_LIGHTS
        };

        // The channel currently being displayed and controlled by the knobs
        int m_active_channel_id = 0;

        // Old knob values
        float m_length, m_length_cv;
        float m_hits, m_hits_cv;
        float m_shift, m_shift_cv;

        struct Channel
        {
                unsigned int m_current_step = 0;
                int m_channel;
                uint32_t m_rhythm;
                dsp::SchmittTrigger m_clock_trigger;
                dsp::SchmittTrigger m_sync_trigger;
                dsp::SchmittTrigger m_reverse_trigger;
                dsp::SchmittTrigger m_invert_trigger;
                dsp::PulseGenerator m_output_generator;
                bool m_apply_sync = false;
                float m_length, m_length_cv;
                float m_hits, m_hits_cv;
                float m_shift, m_shift_cv;
                bool m_reverse, m_invert;
                RareBreeds_Orbits_Eugene *m_module;

                void init(RareBreeds_Orbits_Eugene *module, int channel)
                {
                        m_module = module;
                        m_channel = channel;
                        m_length = m_module->params[LENGTH_KNOB_PARAM].getValue();
                        m_length_cv = m_module->params[LENGTH_CV_KNOB_PARAM].getValue();
                        m_hits = m_module->params[HITS_KNOB_PARAM].getValue();
                        m_hits_cv = m_module->params[HITS_CV_KNOB_PARAM].getValue();
                        m_shift = m_module->params[SHIFT_KNOB_PARAM].getValue();
                        m_shift_cv = m_module->params[SHIFT_CV_KNOB_PARAM].getValue();
                        m_reverse = false;
                        m_invert = false;

                        auto length = readLength();
                        auto hits = readHits(length);
                        m_rhythm = euclideanRhythm(length, hits);
                }

                void toggleReverse(void)
                {
                        m_reverse = !m_reverse;
                }

                bool readReverse(void)
                {
                        if(m_module->inputs[REVERSE_CV_INPUT].isConnected())
                        {
                                m_reverse_trigger.process(m_module->inputs[REVERSE_CV_INPUT].getPolyVoltage(m_channel));
                                return m_reverse_trigger.isHigh() != m_reverse;
                        }
                        else
                        {
                                return m_reverse;
                        }
                }

                void toggleInvert(void)
                {
                        m_invert = !m_invert;
                }

                bool readInvert(void)
                {
                        if(m_module->inputs[INVERT_CV_INPUT].isConnected())
                        {
                                m_invert_trigger.process(m_module->inputs[INVERT_CV_INPUT].getPolyVoltage(m_channel));
                                return m_invert_trigger.isHigh() != m_invert;
                        }
                        else
                        {
                                return m_invert;
                        }
                }

                bool isOnBeat(unsigned int beat, unsigned int length, unsigned int shift, bool invert)
                {
                        return (((rotL(m_rhythm, length, shift) >> beat) & 1) != invert);
                }

                unsigned int readLength()
                {
                        auto cv = m_module->inputs[LENGTH_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
                        auto f_length = m_length + m_length_cv * cv  * (max_rhythm_length - 1);
                        return clampRounded(f_length, 1, max_rhythm_length);
                }

                unsigned int readHits(unsigned int length)
                {
                        auto cv = m_module->inputs[HITS_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
                        auto f_hits = m_hits + m_hits_cv * cv;
                        return clampRounded(f_hits * length, 0, length);
                }

                unsigned int readShift(unsigned int length)
                {
                        auto cv = m_module->inputs[SHIFT_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
                        auto f_shift = m_shift + m_shift_cv * cv  * (max_rhythm_length - 1);
                        return clampRounded(f_shift, 0, max_rhythm_length - 1) % length;
                }

                void process(const ProcessArgs &args)
                {
                        auto length = readLength();

                        // Avoid stepping out of bounds
                        if(m_current_step >= length)
                        {
                                m_current_step = 0;
                        }

                        auto hits = readHits(length);

                        m_rhythm = euclideanRhythm(length, hits);

                        if(m_module->inputs[SYNC_INPUT].getChannels() > m_channel)
                        {
                                m_sync_trigger.process(m_module->inputs[SYNC_INPUT].getPolyVoltage(m_channel));
                                if(m_sync_trigger.isHigh())
                                {
                                        m_apply_sync = true;
                                }
                        }

                        if(m_module->inputs[CLOCK_INPUT].getChannels() > m_channel &&
                           m_clock_trigger.process(m_module->inputs[CLOCK_INPUT].getPolyVoltage(m_channel)))
                        {
                                if(readReverse())
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
                                else
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

                                if(m_apply_sync)
                                {
                                        m_apply_sync = false;
                                        m_current_step = 0;
                                }

                                auto shift = readShift(length);
                                auto invert = readInvert();
                                if(isOnBeat(m_current_step, length, shift, invert))
                                {
                                        m_output_generator.trigger(1e-3f);
                                }
                        }

                        float out = m_output_generator.process(args.sampleTime) ? 10.f : 0.f;
                        m_module->outputs[BEAT_OUTPUT].setVoltage(out, m_channel);
                }

                json_t *dataToJson()
                {
                        json_t *root = json_object();
                        json_object_set_new(root, "length", json_real(m_length));
                        json_object_set_new(root, "length_cv", json_real(m_length_cv));
                        json_object_set_new(root, "hits", json_real(m_hits));
                        json_object_set_new(root, "hits_cv", json_real(m_hits_cv));
                        json_object_set_new(root, "shift", json_real(m_shift));
                        json_object_set_new(root, "shift_cv", json_real(m_shift_cv));
                        json_object_set_new(root, "reverse", json_boolean(m_reverse));
                        json_object_set_new(root, "invert", json_boolean(m_invert));
                        return root;
                }
        };

        Channel m_channels[max_channels];
        Channel *m_active_channel;
        dsp::BooleanTrigger channel_next_trigger;
        dsp::BooleanTrigger channel_prev_trigger;
        dsp::BooleanTrigger reverse_trigger;
        dsp::BooleanTrigger invert_trigger;

        RareBreeds_Orbits_Eugene()
        {
                config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CHANNEL_NEXT_PARAM, 0.f, 1.f, 0.f, "Next Channel");
		configParam(CHANNEL_PREV_PARAM, 0.f, 1.f, 0.f, "Previous Channel");
                configParam(LENGTH_KNOB_PARAM, 1.f, max_rhythm_length, max_rhythm_length, "Length");
                configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
                configParam(SHIFT_KNOB_PARAM, 0.f, max_rhythm_length - 1, 0.f, "Shift");
                configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
                configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
                configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
                configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
                configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");

                for(auto i = 0; i < max_channels; ++i)
                {
                        m_channels[i].init(this, i);
                }

                m_active_channel_id = 0;
                m_active_channel = &m_channels[m_active_channel_id];

                m_length = params[LENGTH_KNOB_PARAM].getValue();
                m_length_cv = params[LENGTH_CV_KNOB_PARAM].getValue();
                m_hits = params[HITS_KNOB_PARAM].getValue();
                m_hits_cv = params[HITS_CV_KNOB_PARAM].getValue();
                m_shift = params[SHIFT_KNOB_PARAM].getValue();
                m_shift_cv = params[SHIFT_CV_KNOB_PARAM].getValue();
        }

        void process(const ProcessArgs &args) override
        {
                auto active_channels = inputs[CLOCK_INPUT].getChannels();
                outputs[BEAT_OUTPUT].setChannels(active_channels);

                // Update the active channel if its out of range of the active channels
                // Relies on clamp returning 'a' if 'b' < 'a'
                m_active_channel_id = clamp(m_active_channel_id, 0, active_channels - 1);

                if(channel_next_trigger.process(std::round(params[CHANNEL_NEXT_PARAM].getValue())))
                {
                        if(m_active_channel_id == active_channels - 1)
                        {
                                m_active_channel_id = 0;
                        }
                        else
                        {
                                ++m_active_channel_id;
                        }
                }

                if(channel_prev_trigger.process(std::round(params[CHANNEL_PREV_PARAM].getValue())))
                {
                        if(m_active_channel_id ==  0)
                        {
                                m_active_channel_id = active_channels - 1;
                        }
                        else
                        {
                                --m_active_channel_id;
                        }
                }    

                m_active_channel = &m_channels[m_active_channel_id];

                float length = params[LENGTH_KNOB_PARAM].getValue();
                if(length != m_length)
                {
                        m_active_channel->m_length = length;
                        m_length = length;
                }

                float length_cv = params[LENGTH_CV_KNOB_PARAM].getValue();
                if(length_cv != m_length_cv)
                {
                        m_active_channel->m_length_cv = length_cv;
                        m_length_cv = length_cv;
                }

                float hits = params[HITS_KNOB_PARAM].getValue();
                if(hits != m_hits)
                {
                        m_active_channel->m_hits = hits;
                        m_hits = hits;
                }

                float hits_cv = params[HITS_CV_KNOB_PARAM].getValue();
                if(hits_cv != m_hits_cv)
                {
                        m_active_channel->m_hits_cv = hits_cv;
                        m_hits_cv = hits_cv;
                }

                float shift = params[SHIFT_KNOB_PARAM].getValue();
                if(shift != m_shift)
                {
                        m_active_channel->m_shift = shift;
                        m_shift = shift;
                }

                float shift_cv = params[SHIFT_CV_KNOB_PARAM].getValue();
                if(shift_cv != m_shift_cv)
                {
                        m_active_channel->m_shift_cv = shift_cv;
                        m_shift_cv = shift_cv;
                }

                if(reverse_trigger.process(std::round(params[REVERSE_KNOB_PARAM].getValue())))
                {
                        m_active_channel->toggleReverse();
                }

                if(invert_trigger.process(std::round(params[INVERT_KNOB_PARAM].getValue())))
                {
                        m_active_channel->toggleInvert();
                }

                for(auto i = 0; i < active_channels; ++i)
                {
                        m_channels[i].process(args);
                }
        }

        json_t *dataToJson() override
        {
                json_t *root = json_object();

                json_object_set_new(root, "length", json_real(m_length));
                json_object_set_new(root, "length_cv", json_real(m_length_cv));
                json_object_set_new(root, "hits", json_real(m_hits));
                json_object_set_new(root, "hits_cv", json_real(m_hits_cv));
                json_object_set_new(root, "shift", json_real(m_shift));
                json_object_set_new(root, "shift_cv", json_real(m_shift_cv));

                json_object_set_new(root, "active_channel_id", json_integer(m_active_channel_id));

                json_t *channels = json_array();
                for(auto i = 0; i < max_channels; ++i)
                {
                        json_array_append_new(channels, m_channels[i].dataToJson());
                }

		json_object_set_new(root, "channels", channels);

		return root;
	}

        void dataFromJson(json_t *root) override
        {
        }
};

struct RhythmDisplay : TransparentWidget
{
        RareBreeds_Orbits_Eugene *module = NULL;
        std::shared_ptr<Font> font;

        RhythmDisplay()
        {
                font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
        }

        void draw(const DrawArgs &args) override
        {
                if(!module)
                {
                        return;
                }

                const auto length = module->m_active_channel->readLength();
                const auto hits = module->m_active_channel->readHits(length);
                const auto shift = module->m_active_channel->readShift(length);
                const auto invert = module->m_active_channel->readInvert();

                nvgStrokeColor(args.vg, color::WHITE);
                nvgSave(args.vg);

                const Rect b = Rect(Vec(0, 0), box.size);
                nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

                // Translate so (0, 0) is the center of the screen
                nvgTranslate(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Draw length text center bottom and hits text center top
                nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                nvgFontSize(args.vg, 20);
                nvgFontFaceId(args.vg, font->handle);
                nvgFillColor(args.vg, color::WHITE);
                nvgText(args.vg, 0.f, -7.f, std::to_string(hits).c_str(), NULL);
                nvgText(args.vg, 0.f, 7.f, std::to_string(length).c_str(), NULL);
                nvgFontSize(args.vg, 12);
                nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
                // Fundamental numbers channels 1 - 16 on the display, copy that
                nvgText(args.vg, b.size.x / 2.f - .5f, b.size.y / 2.f - .5f,
                        std::to_string(module->m_active_channel_id + 1).c_str(), NULL);

                // Scale to [-1, 1]
                nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Flip x and y so we start at the top and positive angle
                // increments go clockwise
                nvgScale(args.vg, -1.f, -1.f);

                // set the on beat radius so 8 can fix on the screen
                const auto on_radius = 1.f / 8.f;

                // set outline radius so it's bigger than the on_radius
                const auto outline_radius = on_radius * 1.3f;

                // set the off beat radius so it's almost a dot
                const auto off_radius = on_radius / 10.f;

                // Placement of the inner ring of circles when there's too many
                // of them for one ring
                const auto inner_ring_scale = 0.75f;

                // Width of the line when drawing circles
                const auto circle_stroke_width = 0.03f;
                nvgStrokeWidth(args.vg, circle_stroke_width);

                // Add a border of half a circle so we don't draw over the edge
                nvgScale(args.vg, 1.f - outline_radius, 1.f - outline_radius);

                for(unsigned int k = 0; k < length; ++k)
                {
                        float y_pos = 1.f;
                        if(length > 16 and k % 2)
                        {
                                y_pos = inner_ring_scale;
                        }

                        float radius = off_radius;
                        if(module->m_active_channel->isOnBeat(k, length, shift, invert))
                        {
                                radius = on_radius;
                        }

                        nvgSave(args.vg);
                        nvgRotate(args.vg, 2.f * k * M_PI / length);
                        nvgBeginPath(args.vg);
                        nvgCircle(args.vg, 0.f, y_pos, radius);
                        if(invert)
                        {
                                nvgStroke(args.vg);
                        }
                        else
                        {
                                nvgFill(args.vg);
                        }

                        if(module->m_active_channel->m_current_step == k)
                        {
                                nvgBeginPath(args.vg);
                                nvgCircle(args.vg, 0.f, y_pos, outline_radius);
                                nvgStroke(args.vg);
                        }

                        if(shift == k)
                        {
                                nvgBeginPath(args.vg);
                                nvgMoveTo(args.vg, 0.f, 0.5f);
                                nvgLineTo(args.vg, -0.05f, 0.4f);
                                nvgLineTo(args.vg, 0.05f, 0.4f);
                                nvgClosePath(args.vg);
                                nvgFill(args.vg);
                        }

                        nvgRestore(args.vg);
                }

                nvgResetScissor(args.vg);
                nvgRestore(args.vg);
        }
};

struct RareBreeds_Orbits_EugeneWidget : ModuleWidget
{
        RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module)
        {
                setModule(module);
                setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RareBreeds_Orbits_Eugene.svg")));

                addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<CKD6>(mm2px(Vec(24.845, 51.49)), module, RareBreeds_Orbits_Eugene::CHANNEL_PREV_PARAM));
		addParam(createParamCentered<CKD6>(mm2px(Vec(36.866, 51.737)), module, RareBreeds_Orbits_Eugene::CHANNEL_NEXT_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10.48, 67.0)), module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30.48, 67.0)), module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50.48, 67.0)), module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.48, 85.471)), module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 85.471)), module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.48, 85.471)), module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
                addParam(createParamCentered<CKD6>(mm2px(Vec(10.48, 112.0)), module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
                addParam(createParamCentered<CKD6>(mm2px(Vec(50.48, 112.0)), module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));


		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 23.15)), module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 38.15)), module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.48, 100.088)), module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 100.088)), module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.48, 100.088)), module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.813, 112.0)), module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.147, 112.0)), module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.72, 23.15)), module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

                RhythmDisplay *r = createWidget<RhythmDisplay>(mm2px(Vec(14.48, 14.913)));
                r->module = module;
                r->box.size = mm2px(Vec(32.0, 32.0));
                addChild(r);
        }
};

Model *modelRareBreeds_Orbits_Eugene =
        createModel<RareBreeds_Orbits_Eugene, RareBreeds_Orbits_EugeneWidget>("RareBreeds_Orbits_Eugene");
