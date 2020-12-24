#include <cstdint>
#include <string>

#if ORBITS_DEBUG
#include <cassert>
#endif // ORBITS_DEBUG

#include "plugin.hpp"

const auto max_rhythm_length = 32;

// When the number of hits is 0 or 1 the rhythm is always 0x0 or 0x1
// respectively, no need to store them in the look up table.
static const uint8_t euclidean_rhythm_u8[] = {
        // Y = Beat Length - 2
        // X = Number of on-beats - 2
        /* 2 */ 0x3u,
        /* 3 */ 0x5u,  0x7u,
        /* 4 */ 0x5u,  0xdu,  0xfu,
        /* 5 */ 0x9u,  0xdu,  0x1du, 0x1fu,
        /* 6 */ 0x9u,  0x15u, 0x2du, 0x3du, 0x3fu,
        /* 7 */ 0x11u, 0x29u, 0x2du, 0x5du, 0x7du, 0x7fu,
        /* 8 */ 0x11u, 0x29u, 0x55u, 0xadu, 0xddu, 0xfdu, 0xffu
};

static const uint16_t euclidean_rhythm_u16[] = {
        // Y = Beat Length - 9
        // X = Number of on-beats - 2
        /* 9  */ 0x21u,  0x49u,  0xa9u,   0xadu,   0x16du,  0x1bdu,  0x1fdu,  0x1ffu,
        /* 10 */ 0x21u,  0x91u,  0x129u,  0x155u,  0x1adu,  0x2ddu,  0x3bdu,  0x3fdu,  0x3ffu,
        /* 11 */ 0x41u,  0x91u,  0x129u,  0x2a9u,  0x2adu,  0x5adu,  0x6ddu,  0x77du,  0x7fdu,  0x7ffu,
        /* 12 */ 0x41u,  0x111u, 0x249u,  0x4a9u,  0x555u,  0x6adu,  0xb6du,  0xdddu,  0xf7du,  0xffdu,  0xfffu,
        /* 13 */ 0x81u,  0x221u, 0x491u,  0x929u,  0xaa9u,  0xaadu,  0xdadu,  0x16ddu, 0x1bbdu, 0x1efdu, 0x1ffdu, 0x1fffu,
        /* 14 */ 0x81u,  0x221u, 0x891u,  0x929u,  0x14a9u, 0x1555u, 0x16adu, 0x2dadu, 0x2eddu, 0x3bbdu, 0x3efdu, 0x3ffdu, 0x3fffu,
        /* 15 */ 0x101u, 0x421u, 0x891u,  0x1249u, 0x2529u, 0x2aa9u, 0x2aadu, 0x35adu, 0x5b6du, 0x6eddu, 0x77bdu, 0x7dfdu, 0x7ffdu, 0x7fffu,
        /* 16 */ 0x101u, 0x841u, 0x1111u, 0x2491u, 0x2929u, 0x52a9u, 0x5555u, 0x5aadu, 0xadadu, 0xb6ddu, 0xddddu, 0xef7du, 0xfdfdu, 0xfffdu, 0xffffu
};

static const uint32_t euclidean_rhythm_u32[] = {
        // Y = Beat Length - 17
        // X = Number of on-beats - 2
        /* 17 */ 0x201u,   0x841u,    0x2221u,    0x4491u,    0x4929u,    0x94a9u,     0xaaa9u,     0xaaadu,     0xd6adu,     0x16dadu,    0x176ddu,    0x1bbbdu,    0x1ef7du,    0x1fbfdu,    0x1fffdu,    0x1ffffu,
        /* 18 */ 0x201u,   0x1041u,   0x4221u,    0x8891u,    0x9249u,    0x12929u,    0x152a9u,    0x15555u,    0x15aadu,    0x1adadu,    0x2db6du,    0x2eeddu,    0x37bbdu,    0x3df7du,    0x3fbfdu,    0x3fffdu,    0x3ffffu,
        /* 19 */ 0x401u,   0x2081u,   0x4221u,    0x8891u,    0x12491u,   0x14929u,    0x294a9u,    0x2aaa9u,    0x2aaadu,    0x2d6adu,    0x56dadu,    0x5b6ddu,    0x6eeddu,    0x77bbdu,    0x7befdu,    0x7f7fdu,    0x7fffdu,    0x7ffffu,
        /* 20 */ 0x401u,   0x2081u,   0x8421u,    0x11111u,   0x24491u,   0x24929u,    0x4a529u,    0x54aa9u,    0x55555u,    0x56aadu,    0x6b5adu,    0xb6dadu,    0xb76ddu,    0xdddddu,    0xef7bdu,    0xfbefdu,    0xff7fdu,    0xffffdu,    0xfffffu,
        /* 21 */ 0x801u,   0x4081u,   0x10841u,   0x22221u,   0x44891u,   0x49249u,    0x52929u,    0xa54a9u,    0xaaaa9u,    0xaaaadu,    0xb56adu,    0x15adadu,   0x16db6du,   0x176eddu,   0x1bbbbdu,   0x1def7du,   0x1f7efdu,   0x1feffdu,   0x1ffffdu,   0x1fffffu,
        /* 22 */ 0x801u,   0x8101u,   0x20841u,   0x42221u,   0x48891u,   0x92491u,    0x94929u,    0x1294a9u,   0x154aa9u,   0x155555u,   0x156aadu,   0x1ad6adu,   0x2d6dadu,   0x2db6ddu,   0x36eeddu,   0x37bbbdu,   0x3bef7du,   0x3efdfdu,   0x3feffdu,   0x3ffffdu,   0x3fffffu,
        /* 23 */ 0x1001u,  0x8101u,   0x20841u,   0x84221u,   0x88891u,   0x122491u,   0x124929u,   0x252929u,   0x2952a9u,   0x2aaaa9u,   0x2aaaadu,   0x2d5aadu,   0x35adadu,   0x5b6dadu,   0x5bb6ddu,   0x6eeeddu,   0x6f7bbdu,   0x7bef7du,   0x7efdfdu,   0x7fdffdu,   0x7ffffdu,   0x7fffffu,
        /* 24 */ 0x1001u,  0x10101u,  0x41041u,   0x84221u,   0x111111u,  0x224491u,   0x249249u,   0x292929u,   0x4a94a9u,   0x552aa9u,   0x555555u,   0x55aaadu,   0x6ad6adu,   0xadadadu,   0xb6db6du,   0xbb76ddu,   0xddddddu,   0xef7bbdu,   0xf7df7du,   0xfdfdfdu,   0xffdffdu,   0xfffffdu,   0xffffffu,
        /* 25 */ 0x2001u,  0x20201u,  0x82081u,   0x108421u,  0x222221u,  0x448891u,   0x492491u,   0x4a4929u,   0x94a529u,   0xa952a9u,   0xaaaaa9u,   0xaaaaadu,   0xad5aadu,   0xd6b5adu,   0x16b6dadu,  0x16db6ddu,  0x176eeddu,  0x1bbbbbdu,  0x1def7bdu,  0x1efbefdu,  0x1fbfbfdu,  0x1ffbffdu,  0x1fffffdu,  0x1ffffffu,
        /* 26 */ 0x2001u,  0x20201u,  0x102081u,  0x210841u,  0x442221u,  0x488891u,   0x922491u,   0x924929u,   0x1252929u,  0x14a94a9u,  0x1552aa9u,  0x1555555u,  0x155aaadu,  0x16ad6adu,  0x1b5adadu,  0x2db6dadu,  0x2dbb6ddu,  0x36eeeddu,  0x377bbbdu,  0x3bdef7du,  0x3dfbefdu,  0x3fbfbfdu,  0x3ffbffdu,  0x3fffffdu,  0x3ffffffu,
        /* 27 */ 0x4001u,  0x40201u,  0x102081u,  0x410841u,  0x844221u,  0x888891u,   0x1224491u,  0x1249249u,  0x1494929u,  0x25294a9u,  0x2a552a9u,  0x2aaaaa9u,  0x2aaaaadu,  0x2b55aadu,  0x35ad6adu,  0x56d6dadu,  0x5b6db6du,  0x5bb76ddu,  0x6eeeeddu,  0x6f77bbdu,  0x77def7du,  0x7dfbefdu,  0x7f7fbfdu,  0x7ff7ffdu,  0x7fffffdu,  0x7ffffffu,
        /* 28 */ 0x4001u,  0x80401u,  0x204081u,  0x820841u,  0x884221u,  0x1111111u,  0x2244891u,  0x2492491u,  0x24a4929u,  0x4a52929u,  0x52a54a9u,  0x554aaa9u,  0x5555555u,  0x556aaadu,  0x5ab56adu,  0x6b5adadu,  0xb6b6dadu,  0xb6db6ddu,  0xbb76eddu,  0xdddddddu,  0xeef7bbdu,  0xefbef7du,  0xfbf7efdu,  0xfeff7fdu,  0xfff7ffdu,  0xffffffdu,  0xfffffffu,
        /* 29 */ 0x8001u,  0x80401u,  0x408101u,  0x820841u,  0x1084221u, 0x2222221u,  0x2448891u,  0x4912491u,  0x4924929u,  0x5252929u,  0x95294a9u,  0xa954aa9u,  0xaaaaaa9u,  0xaaaaaadu,  0xad56aadu,  0xd5ad6adu,  0x15b5adadu, 0x16db6dadu, 0x16ddb6ddu, 0x1b76eeddu, 0x1bbbbbbdu, 0x1def7bbdu, 0x1efbef7du, 0x1f7efdfdu, 0x1feff7fdu, 0x1ffefffdu, 0x1ffffffdu, 0x1fffffffu,
        /* 30 */ 0x8001u,  0x100401u, 0x808101u,  0x1041041u, 0x2108421u, 0x4422221u,  0x4488891u,  0x9124491u,  0x9249249u,  0x9494929u,  0x1294a529u, 0x14a952a9u, 0x1554aaa9u, 0x15555555u, 0x1556aaadu, 0x16ad5aadu, 0x1ad6b5adu, 0x2d6d6dadu, 0x2db6db6du, 0x2ddb76ddu, 0x376eeeddu, 0x377bbbbdu, 0x3bdef7bdu, 0x3df7df7du, 0x3efefdfdu, 0x3fdff7fdu, 0x3ffefffdu, 0x3ffffffdu, 0x3fffffffu,
        /* 31 */ 0x10001u, 0x200801u, 0x808101u,  0x2082081u, 0x4210841u, 0x8442221u,  0x8888891u,  0x11224491u, 0x12492491u, 0x12524929u, 0x24a52929u, 0x254a94a9u, 0x2a954aa9u, 0x2aaaaaa9u, 0x2aaaaaadu, 0x2ad56aadu, 0x356ad6adu, 0x36b5adadu, 0x5b5b6dadu, 0x5b6db6ddu, 0x5dbb76ddu, 0x6eeeeeddu, 0x6f77bbbdu, 0x77bdef7du, 0x7befbefdu, 0x7efefdfdu, 0x7fbfeffdu, 0x7ffdfffdu, 0x7ffffffdu, 0x7fffffffu,
        /* 32 */ 0x10001u, 0x200801u, 0x1010101u, 0x4082081u, 0x8410841u, 0x10884221u, 0x11111111u, 0x22448891u, 0x24912491u, 0x24924929u, 0x29292929u, 0x4a5294a9u, 0x52a952a9u, 0x5552aaa9u, 0x55555555u, 0x555aaaadu, 0x5aad5aadu, 0x6b5ad6adu, 0xadadadadu, 0xb6db6dadu, 0xb6ddb6ddu, 0xbb76eeddu, 0xddddddddu, 0xdeef7bbdu, 0xef7def7du, 0xf7efbefdu, 0xfdfdfdfdu, 0xffbfeffdu, 0xfffdfffdu, 0xfffffffdu, 0xffffffffu
};

static uint32_t sumTo(unsigned int n)
{
        return n * (n + 1) / 2;
}

static uint32_t rotL(uint32_t val, uint32_t size, uint32_t amount)
{
        return (val << amount | val >> (size - amount));
}

static uint32_t euclideanRhythm(unsigned int length, unsigned int on_beats)
{
#if ORBITS_DEBUG
        assert(length >= 1);
        assert(length <= 32);
        assert(on_beats <= length);
#endif // ORBITS_DEBUG

        // All euclidean rhythms with on beats of 0 or 1 are just 0 or 1
        // respectively.
        if(on_beats <= 1)
        {
                return on_beats;
        }

        // On beats 0 and 1 aren't stored in the look up table.
        unsigned int on_beats_m2 = on_beats - 2;
        // Rhythms are stored in the smallest native type available.
        if(length <= 8)
        {
                // Rhythm lengths 0 and 1 aren't stored in the look up table.
                unsigned int length_m2 = length - 2;

                // Index for this length starts at the sum of lengths up to this
                // number, then add the number of on beats.
                unsigned int index = sumTo(length_m2) + on_beats_m2;

                return (uint32_t)euclidean_rhythm_u8[index];
        }
        else if(length <= 16)
        {
                // Rhythm lengths less than 9 aren't stored in the look up
                // table.
                unsigned int length_m9 = length - 9;
                unsigned int index = length_m9 * 8 + sumTo(length_m9 - 1) + on_beats_m2;
                return (uint32_t)euclidean_rhythm_u16[index];
        }
        else // length <= 32
        {
                // Rhythm lengths less than 17 aren't stored in the look up
                // table.
                unsigned int length_m17 = length - 17;
                unsigned int index = length_m17 * 16 + sumTo(length_m17 - 1) + on_beats_m2;
                return euclidean_rhythm_u32[index];
        }
}

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
        unsigned int index = 0;
        uint32_t rhythm;

        RareBreeds_Orbits_Eugene()
        {
                config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
                configParam(LENGTH_KNOB_PARAM, 1.f, max_rhythm_length, max_rhythm_length, "Length");
                configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
                configParam(SHIFT_KNOB_PARAM, 0.f, max_rhythm_length - 1, 0.f, "Shift");
                configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
                configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
                configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
                configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
                configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");

                // Ensure the default rhythm exists before drawing
                unsigned int length = readLength();
                rhythm = euclideanRhythm(length, readHits(length));
        }

        bool isOnBeat(unsigned int index, unsigned int length, unsigned int shift, bool invert)
        {
                uint32_t rotated = rotL(rhythm, length, shift);
                return ((rotated >> index) & 1) != invert;
        }

        unsigned int readLength(void)
        {
                float value = params[LENGTH_KNOB_PARAM].getValue();
                if(inputs[LENGTH_CV_INPUT].isConnected())
                {
                        // bipolar +-5V input
                        float input = inputs[LENGTH_CV_INPUT].getVoltage();
                        float normalized_input = input / 5.f;
                        float attenuation = params[LENGTH_CV_KNOB_PARAM].getValue();
                        value += attenuation * normalized_input * (max_rhythm_length - 1);
                }

                return clamp(int(std::round(value)), 1, max_rhythm_length);
        }

        unsigned int readHits(unsigned int length)
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

        unsigned int readShift(unsigned int length)
        {
                float value = params[SHIFT_KNOB_PARAM].getValue();
                if(inputs[SHIFT_CV_INPUT].isConnected())
                {
                        // bipolar +-5V input
                        float input = inputs[SHIFT_CV_INPUT].getVoltage();
                        float normalized_input = input / 5.f;
                        float attenuation = params[SHIFT_CV_KNOB_PARAM].getValue();
                        value += attenuation * normalized_input * (max_rhythm_length - 1);
                }

                return clamp(int(std::round(value)), 0, max_rhythm_length - 1) % length;
        }

        bool readReverse(void)
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

        bool readInvert(void)
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

        void advanceIndex(unsigned int length, bool reverse)
        {
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
                else
                {
                        ++index;
                        if(index >= length)
                        {
                                index = 0;
                        }
                }
        }

        bool resetActive()
        {
                return inputs[SYNC_INPUT].isConnected() && syncTrigger.process(inputs[SYNC_INPUT].getVoltage());
        }

        void process(const ProcessArgs &args) override
        {
                unsigned int length = readLength();

                // wrap the index to the new length
                // to avoid accessing the rhythm out of bounds
                if(index >= length)
                {
                        index = 0;
                }

                rhythm = euclideanRhythm(length, readHits(length));

                if(inputs[CLOCK_INPUT].isConnected() && clockTrigger.process(inputs[CLOCK_INPUT].getVoltage()))
                {
                        advanceIndex(length, readReverse());

                        if(resetActive())
                        {
                                index = 0;
                        }

                        if(isOnBeat(index, length, readShift(length), readInvert()))
                        {
                                outputGenerator.trigger(1e-3f);
                        }
                }

                outputs[BEAT_OUTPUT].setVoltage(outputGenerator.process(args.sampleTime) ? 10.f : 0.f);
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
                        return;

                const auto length = module->readLength();
                const auto hits = module->readHits(length);
                const auto shift = module->readShift(length);

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

                const bool invert = module->readInvert();
                for(unsigned int k = 0; k < length; ++k)
                {
                        float y_pos = 1.f;
                        if(length > 16 and k % 2)
                        {
                                y_pos = inner_ring_scale;
                        }

                        float radius = off_radius;
                        if(module->isOnBeat(k, length, shift, invert))
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

                        if(module->index == k)
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

                addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                addChild(createWidget<ScrewSilver>(
                        Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10.48, 67.0)), module,
                                                                  RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30.48, 67.0)), module,
                                                                  RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50.48, 67.0)), module,
                                                                  RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.48, 86.0)), module,
                                                             RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 86.0)), module,
                                                             RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.48, 86.0)), module,
                                                             RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
                addParam(createParamCentered<CKSS>(mm2px(Vec(10.48, 112.0)), module,
                                                   RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
                addParam(createParamCentered<CKSS>(mm2px(Vec(50.48, 112.0)), module,
                                                   RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 29.5)), module,
                                                         RareBreeds_Orbits_Eugene::CLOCK_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 44.5)), module,
                                                         RareBreeds_Orbits_Eugene::SYNC_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.48, 100.0)), module,
                                                         RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 100.0)), module,
                                                         RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.48, 100.0)), module,
                                                         RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.813, 112.0)), module,
                                                         RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.147, 112.0)), module,
                                                         RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

                addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.72, 29.5)), module,
                                                           RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

                RhythmDisplay *r = createWidget<RhythmDisplay>(mm2px(Vec(14.48, 16.5)));
                r->module = module;
                r->box.size = mm2px(Vec(32.0, 32.0));
                addChild(r);
        }
};

Model *modelRareBreeds_Orbits_Eugene =
        createModel<RareBreeds_Orbits_Eugene, RareBreeds_Orbits_EugeneWidget>("RareBreeds_Orbits_Eugene");
