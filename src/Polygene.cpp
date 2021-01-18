#include <cstdint>
#include <string>
#include <cassert>

#include "Euclidean.hpp"
#include "plugin.hpp"

static const auto max_channels = 16;

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

struct RareBreeds_Orbits_Polygene : Module
{
        enum ParamIds
        {
                CHANNEL_KNOB_PARAM,
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
                RareBreeds_Orbits_Polygene *m_module;

                void init(RareBreeds_Orbits_Polygene *module, int channel)
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

                bool isOnBeat(unsigned int beat, unsigned int length, unsigned int hits, unsigned int shift, bool invert)
                {
                        euclidean::Rhythm rhythm = euclidean::rhythm(length, hits);
                        return euclidean::rotate(rhythm, length, shift)[beat] != invert;
                }

                unsigned int readLength()
                {
                        auto cv = m_module->inputs[LENGTH_CV_INPUT].getNormalPolyVoltage(0.0f, m_channel) / 5.f;
                        auto f_length = m_length + m_length_cv * cv  * (euclidean::max_length - 1);
                        return clampRounded(f_length, 1, euclidean::max_length);
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
                        auto f_shift = m_shift + m_shift_cv * cv  * (euclidean::max_length - 1);
                        return clampRounded(f_shift, 0, euclidean::max_length - 1) % length;
                }

                void process(const ProcessArgs &args)
                {
                        auto length = readLength();

                        // Avoid stepping out of bounds
                        if(m_current_step >= length)
                        {
                                m_current_step = 0;
                        }

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

                                auto hits = readHits(length);
                                auto shift = readShift(length);
                                auto invert = readInvert();
                                if(isOnBeat(m_current_step, length, hits, shift, invert))
                                {
                                        m_output_generator.trigger(1e-3f);
                                }
                        }

                        auto out = m_output_generator.process(args.sampleTime) ? 10.f : 0.f;
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

                void dataFromJson(json_t *root)
                {
                        json_load_real(root, "length", &m_length);
                        json_load_real(root, "length_cv", &m_length_cv);
                        json_load_real(root, "hits", &m_hits);
                        json_load_real(root, "hits_cv", &m_hits_cv);
                        json_load_real(root, "shift", &m_shift);
                        json_load_real(root, "shift_cv", &m_shift_cv);
                        json_load_bool(root, "reverse", &m_reverse);
                        json_load_bool(root, "invert", &m_invert);
                }

                void onRandomize()
                {
                        m_length = random::uniform() * (euclidean::max_length - 1) + 1;
                        m_length_cv = random::uniform();
                        m_hits = random::uniform();
                        m_hits_cv = random::uniform();
                        m_shift = random::uniform();
                        m_shift_cv = random::uniform();
                        m_reverse = (random::uniform() < 0.5f);
                        m_invert = (random::uniform() < 0.5f);
                }
        };

        Channel m_channels[max_channels];
        Channel *m_active_channel;
        dsp::BooleanTrigger reverse_trigger;
        dsp::BooleanTrigger invert_trigger;

        RareBreeds_Orbits_Polygene()
        {
                config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
                configParam(CHANNEL_KNOB_PARAM, 0.f, 15.f, 0.f, "Channel", "", 0.f, 1.f, 1.f);
                configParam(LENGTH_KNOB_PARAM, 1.f, euclidean::max_length, euclidean::max_length, "Length");
                configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
                configParam(SHIFT_KNOB_PARAM, 0.f, euclidean::max_length - 1, 0.f, "Shift");
                configParam(LENGTH_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Length CV");
                configParam(HITS_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Hits CV");
                configParam(SHIFT_CV_KNOB_PARAM, 0.f, 1.f, 0.f, "Shift CV");
                configParam(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse");
                configParam(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert");

                reset();
        }

        void reset()
        {
                m_active_channel_id = 0;
                m_active_channel = &m_channels[m_active_channel_id];

                m_length = params[LENGTH_KNOB_PARAM].getValue();
                m_length_cv = params[LENGTH_CV_KNOB_PARAM].getValue();
                m_hits = params[HITS_KNOB_PARAM].getValue();
                m_hits_cv = params[HITS_CV_KNOB_PARAM].getValue();
                m_shift = params[SHIFT_KNOB_PARAM].getValue();
                m_shift_cv = params[SHIFT_CV_KNOB_PARAM].getValue();

                for(auto i = 0; i < max_channels; ++i)
                {
                        m_channels[i].init(this, i);
                }
        }

        void process(const ProcessArgs &args) override
        {
                auto active_channels = inputs[CLOCK_INPUT].getChannels();
                outputs[BEAT_OUTPUT].setChannels(active_channels);

                m_active_channel_id = std::round(params[CHANNEL_KNOB_PARAM].getValue());
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

                for(auto i = 0; i < max_channels; ++i)
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
                json_load_real(root, "length", &m_length);
                json_load_real(root, "length_cv", &m_length_cv);
                json_load_real(root, "hits", &m_hits);
                json_load_real(root, "hits_cv", &m_hits_cv);
                json_load_real(root, "shift", &m_shift);
                json_load_real(root, "shift_cv", &m_shift_cv);
                json_load_integer(root, "active_channel_id", &m_active_channel_id);
                json_t *channels = json_object_get(root, "channels");
                if(channels)
                {
                        for(auto i = 0; i < max_channels; ++i)
                        {
                                json_t *channel = json_array_get(channels, i);
                                if(channel)
                                {
                                        m_channels[i].dataFromJson(channel);
                                }
                        }
                }
        }

        void onRandomize() override
        {
                for(auto i = 0; i < max_channels; ++i)
                {
                        m_channels[i].onRandomize();
                }

                // Parameters have already been randomised by VCV rack
                // But then the active channel controlled by those parameters has been randomised again
                // Update the parameters so they reflect the active channels randomised parameters
                params[LENGTH_KNOB_PARAM].setValue(m_active_channel->m_length);
                params[LENGTH_CV_KNOB_PARAM].setValue(m_active_channel->m_length_cv);
                params[HITS_KNOB_PARAM].setValue(m_active_channel->m_hits);
                params[HITS_CV_KNOB_PARAM].setValue(m_active_channel->m_hits_cv);
                params[SHIFT_KNOB_PARAM].setValue(m_active_channel->m_shift);
                params[SHIFT_CV_KNOB_PARAM].setValue(m_active_channel->m_shift_cv);
        }

        void onReset() override
        {
                reset();
        }
};

struct PolygeneRhythmDisplay : TransparentWidget
{
        RareBreeds_Orbits_Polygene *module = NULL;
        std::shared_ptr<Font> font;

        PolygeneRhythmDisplay()
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
                const auto reverse = module->m_active_channel->readReverse();

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

                // Draw current channel in the bottom right
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

                // Triangle showing direction
                nvgBeginPath(args.vg);
                if(reverse)
                {
                        nvgMoveTo(args.vg, 0.23f, -0.2f);
                        nvgLineTo(args.vg, 0.3f, 0.f);
                        nvgLineTo(args.vg, 0.23f, 0.2f);
                }
                else
                {
                        nvgMoveTo(args.vg, -0.23f, 0.2f);
                        nvgLineTo(args.vg, -0.3f, -0.f);
                        nvgLineTo(args.vg, -0.23f, -0.2f);
                }
                nvgFill(args.vg);
                nvgClosePath(args.vg);

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
                const auto circle_stroke_width = 0.02f;
                nvgStrokeWidth(args.vg, circle_stroke_width);

                // Add a border of half a circle so we don't draw over the edge
                nvgScale(args.vg, 1.f - outline_radius, 1.f - outline_radius);

                for(auto k = 0u; k < length; ++k)
                {
                        auto y_pos = 1.f;
                        if(length > 16 && k % 2)
                        {
                                y_pos = inner_ring_scale;
                        }

                        auto on_beat = module->m_active_channel->isOnBeat(k, length, hits, shift, invert);
                        auto radius = on_beat ? on_radius : off_radius;

                        nvgSave(args.vg);
                        nvgRotate(args.vg, 2.f * k * M_PI / length);
                        nvgBeginPath(args.vg);
                        nvgCircle(args.vg, 0.f, y_pos, radius);
                        if(on_beat && invert)
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

struct RareBreeds_Orbits_PolygeneWidget : ModuleWidget
{
        RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module)
        {
                setModule(module);
                setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RareBreeds_Orbits_Polygene.svg")));

                addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
                addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.72, 38.15)), module, RareBreeds_Orbits_Polygene::CHANNEL_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(10.48, 67.0)), module, RareBreeds_Orbits_Polygene::LENGTH_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(30.48, 67.0)), module, RareBreeds_Orbits_Polygene::HITS_KNOB_PARAM));
                addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(50.48, 67.0)), module, RareBreeds_Orbits_Polygene::SHIFT_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.48, 85.471)), module, RareBreeds_Orbits_Polygene::LENGTH_CV_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 85.471)), module, RareBreeds_Orbits_Polygene::HITS_CV_KNOB_PARAM));
                addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.48, 85.471)), module, RareBreeds_Orbits_Polygene::SHIFT_CV_KNOB_PARAM));
                addParam(createParamCentered<CKD6>(mm2px(Vec(10.48, 112.0)), module, RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM));
                addParam(createParamCentered<CKD6>(mm2px(Vec(50.48, 112.0)), module, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM));

                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 23.15)), module, RareBreeds_Orbits_Polygene::CLOCK_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.24, 38.15)), module, RareBreeds_Orbits_Polygene::SYNC_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.48, 100.088)), module, RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 100.088)), module, RareBreeds_Orbits_Polygene::HITS_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(45.48, 100.088)), module, RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.813, 112.0)), module, RareBreeds_Orbits_Polygene::REVERSE_CV_INPUT));
                addInput(createInputCentered<PJ301MPort>(mm2px(Vec(37.147, 112.0)), module, RareBreeds_Orbits_Polygene::INVERT_CV_INPUT));

                addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(53.72, 23.15)), module, RareBreeds_Orbits_Polygene::BEAT_OUTPUT));

                PolygeneRhythmDisplay *r = createWidget<PolygeneRhythmDisplay>(mm2px(Vec(14.48, 14.913)));
                r->module = module;
                r->box.size = mm2px(Vec(32.0, 32.0));
                addChild(r);
        }
};

Model *modelRareBreeds_Orbits_Polygene = createModel<RareBreeds_Orbits_Polygene, RareBreeds_Orbits_PolygeneWidget>("RareBreeds_Orbits_Polygene");
