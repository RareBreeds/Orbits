#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "OrbitsConfig.hpp"
#include "OrbitsSkinned.hpp"

static OrbitsConfig config("res/eugene-layout.json");

struct EugeneRhythmDisplay : TransparentWidget
{
        RareBreeds_Orbits_Eugene *module = NULL;
        std::shared_ptr<Font> font;

        EugeneRhythmDisplay();
        void draw(const DrawArgs &args) override;
};

EugeneRhythmDisplay::EugeneRhythmDisplay()
{
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
}

void EugeneRhythmDisplay::draw(const DrawArgs &args)
{
        if(!module)
        {
                return;
        }

        const auto length = module->readLength();
        const auto hits = module->readHits(length);
        const auto shift = module->readShift(length);
        const auto foreground_color = color::WHITE;

        nvgSave(args.vg);

        const Rect b = Rect(Vec(0, 0), box.size);
        nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

        // Everything drawn after here is in the foreground colour
        nvgStrokeColor(args.vg, foreground_color);
        nvgFillColor(args.vg, foreground_color);

        // Translate so (0, 0) is the center of the screen
        nvgTranslate(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Draw length text center bottom and hits text center top
        nvgBeginPath(args.vg);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontSize(args.vg, 20);
        nvgFontFaceId(args.vg, font->handle);
        nvgText(args.vg, 0.f, -7.f, std::to_string(hits).c_str(), NULL);
        nvgText(args.vg, 0.f, 7.f, std::to_string(length).c_str(), NULL);
        // nvgFill(args.vg);

        // Scale to [-1, 1]
        nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Flip x and y so we start at the top and positive angle increments go clockwise
        nvgScale(args.vg, -1.f, -1.f);

        // Triangle showing direction
        const bool reverse = module->readReverse();
        // nvgBeginPath(args.vg);
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

        // set the on beat radius so 8 can fix on the screen
        const auto on_radius = 1.f / 8.f;

        // set outline radius so it's bigger than the on_radius
        const auto outline_radius = on_radius * 1.3f;

        // set the off beat radius so it's almost a dot
        const auto off_radius = on_radius / 10.f;

        // Placement of the inner ring of circles when there's too many of them for one ring
        const auto inner_ring_scale = 0.75f;

        // Width of the line when drawing circles
        const auto circle_stroke_width = 0.03f;
        nvgStrokeWidth(args.vg, circle_stroke_width);

        // Add a border of half a circle so we don't draw over the edge
        nvgScale(args.vg, 1.f - outline_radius, 1.f - outline_radius);

        // TODO: probably faster to draw all filled objects first then all outlined as a separate path
        const bool invert = module->readInvert();
        for(unsigned int k = 0; k < length; ++k)
        {
                float y_pos = 1.f;
                if(length > 16 and k % 2)
                {
                        y_pos = inner_ring_scale;
                }

                float radius = off_radius;
                if(module->rhythm[k])
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

                // Current beat marker
                if(module->index == k)
                {
                        nvgBeginPath(args.vg);
                        nvgCircle(args.vg, 0.f, y_pos, outline_radius);
                        nvgStroke(args.vg);
                }

                // shift pointer
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

RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module) : OrbitsWidget(&config)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->widget = this;
        }

        m_theme = m_config->getDefaultThemeId();

        // clang-format off
        setPanel(APP->window->loadSvg(m_config->getSvg("panel")));

        addChild(createOrbitsSkinnedScrew(m_config, "screw_top_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_top_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_bottom_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_bottom_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "length_knob", module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "hits_knob", module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "shift_knob", module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "length_cv_knob", module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "hits_cv_knob", module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "shift_cv_knob", module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(m_config, "reverse_switch", module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(m_config, "invert_switch", module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

        addInput(createOrbitsSkinnedInput(m_config, "clock_port", module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "sync_port", module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "length_cv_port", module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "hits_cv_port", module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "shift_cv_port", module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "reverse_cv_port", module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "invert_cv_port", module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

        addOutput(createOrbitsSkinnedOutput(m_config, "beat_port", module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));
        // clang-format on

        EugeneRhythmDisplay *r = createWidget<EugeneRhythmDisplay>(m_config->getPos("display"));
        r->module = module;
        r->box.size = m_config->getSize("display");
        addChild(r);
}
