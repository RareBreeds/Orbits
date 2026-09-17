#include "EugeneComponentPositions.h"
#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "OrbitsConfig.hpp"
#include "OrbitsSkinned.hpp"

static OrbitsConfig g_config("res/eugene-layout.json");

struct EugeneRhythmDisplayUnbuffered : Widget
{
        EugeneDisplayData m_data;
        EugeneRhythmDisplayUnbuffered(Vec size);
        void drawLayer(const DrawArgs &args, int layer) override;
};

EugeneRhythmDisplayUnbuffered::EugeneRhythmDisplayUnbuffered(Vec size)
{
        box.pos = Vec(0.0, 0.0);
        box.size = size;
}

void EugeneRhythmDisplayUnbuffered::drawLayer(const DrawArgs &args, int layer)
{
        // Drawings to layer 1 don't dim when the room lights are dimmed
        if(layer == 1)
        {
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
                std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
                nvgFontFaceId(args.vg, font->handle);
                nvgText(args.vg, 0.f, -7.f, std::to_string(m_data.hits).c_str(), NULL);
                nvgText(args.vg, 0.f, 7.f, std::to_string(m_data.length).c_str(), NULL);
                // nvgFill(args.vg);

                // Scale to [-1, 1]
                nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Flip x and y so we start at the top and positive angle increments go clockwise
                nvgScale(args.vg, -1.f, -1.f);

                // nvgBeginPath(args.vg);
                if(m_data.reverse)
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

                // set the on beat radius so 8 can fit on the screen
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

                for(unsigned int k = 0; k < m_data.length; ++k)
                {
                        float y_pos = 1.f;
                        if(m_data.length > 16 and k % 2)
                        {
                                y_pos = inner_ring_scale;
                        }

                        float radius = off_radius;

                        if(rhythm::beat(m_data.length, m_data.hits, m_data.shift, k, m_data.invert))
                        {
                                radius = on_radius;
                        }

                        nvgSave(args.vg);
                        const float kf = static_cast<float>(k);
                        const float mpif = static_cast<float>(M_PI);
                        const float lenf = static_cast<float>(m_data.length);
                        nvgRotate(args.vg, 2.f * kf * mpif / lenf);
                        nvgBeginPath(args.vg);
                        nvgCircle(args.vg, 0.f, y_pos, radius);
                        if(m_data.invert)
                        {
                                nvgStroke(args.vg);
                        }
                        else
                        {
                                nvgFill(args.vg);
                        }

                        // Current beat marker
                        // Highlight the beat that has just played
                        // For forward moving rhythms this is the previous index
                        // For reversed rhythms this is index
                        if((m_data.reverse && m_data.current_step == k) ||
                        (!m_data.reverse &&
                        ((k == m_data.length - 1) ? (0 == m_data.current_step) : (k + 1 == m_data.current_step))))
                        {
                                nvgBeginPath(args.vg);
                                nvgCircle(args.vg, 0.f, y_pos, outline_radius);
                                nvgStroke(args.vg);
                        }

                        // shift pointer
                        if(m_data.shift == k)
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
}

struct EugeneRhythmDisplay : Widget
{
        FramebufferWidget *m_fb;
        EugeneRhythmDisplayUnbuffered *m_ub;
        RareBreeds_Orbits_Eugene *m_module;

        EugeneRhythmDisplay(RareBreeds_Orbits_Eugene *module, Vec pos, Vec size);
        void drawLayer(const DrawArgs& args, int layer) override;
};

EugeneRhythmDisplay::EugeneRhythmDisplay(RareBreeds_Orbits_Eugene *module, Vec pos, Vec size)
{
        m_module = module;
        box.pos = pos;
        box.size = size;
        m_fb = new FramebufferWidget;
        addChild(m_fb);
        m_ub = new EugeneRhythmDisplayUnbuffered(size);
        m_fb->addChild(m_ub);
}

void EugeneRhythmDisplay::drawLayer(const DrawArgs &args, int layer)
{
        // Drawings to layer 1 don't dim when the room lights are dimmed
        if(layer == 1)
        {
                nvgGlobalTint(args.vg, color::WHITE);

                EugeneDisplayData data = RareBreeds_Orbits_Eugene::getDisplayData(m_module);
                if(data != m_ub->m_data)
                {
                        m_ub->m_data = data;
                        m_fb->dirty = true;
                }
        }

        Widget::drawLayer(args, layer);
}

#define EUGENE_POS(NAME) ORBITS_POS(EUGENE_##NAME)

RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module) : OrbitsWidget(&g_config)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->m_widget = this;
                beat_widget.m_module = &module->m_beat;
                eoc_widget.m_module = &module->m_eoc;
        }

        m_theme = ORBITS_THEME_DEFAULT;

        // clang-format off
        // TODO: MOOOO: OrbitsWidgetConfig
        // EugeneConfig file that contains all of the svgs?
        // and separately all of the positions?
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/eugene-dark-panel.svg")));

        OrbitsSkinnedScrewConfig screw = {
                {"res/dark-hex-screw.svg", "res/light-hex-screw.svg"},
                Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)
        };
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        OrbitsSkinnedKnobConfig knob = {
                {"res/red-knob-large.svg", "res/blue-knob-large.svg"},
                EUGENE_POS(LENGTH_KNOB)
        };
        addParam(createOrbitsSkinnedSnappyKnob(knob, module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));

        knob.pos = EUGENE_POS(HITS_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));

        knob.pos = EUGENE_POS(SHIFT_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));

        knob.imagePaths = {"res/red-knob-small.svg", "res/blue-knob-small.svg"};
        knob.pos = EUGENE_POS(LENGTH_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));

        knob.pos = EUGENE_POS(HITS_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));

        knob.pos = EUGENE_POS(SHIFT_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));

        OrbitsSkinnedSwitchConfig swtch = {
                {"res/dark-rev-switch-on.svg", "res/light-rev-switch-on.svg"},
                {"res/dark-rev-switch-off.svg", "res/light-rev-switch-off.svg"},
                EUGENE_POS(REVERSE_SWITCH)
        };
        addParam(createOrbitsSkinnedSwitch(swtch, module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));

        swtch.onImagePaths = {"res/dark-inv-switch-on.svg", "res/light-inv-switch-on.svg"};
        swtch.offImagePaths = {"res/dark-inv-switch-off.svg", "res/light-inv-switch-off.svg"};
        swtch.pos = EUGENE_POS(INVERT_SWITCH);
        addParam(createOrbitsSkinnedSwitch(swtch, module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

        swtch.onImagePaths = {"res/dark-sync-switch-on.svg", "res/light-sync-switch-on.svg"};
        swtch.offImagePaths = {"res/dark-sync-switch-off.svg", "res/light-sync-switch-off.svg"};
        swtch.pos = EUGENE_POS(SYNC_BUTTON);
        addParam(createOrbitsSkinnedButton(swtch, module, RareBreeds_Orbits_Eugene::SYNC_KNOB_PARAM));

        OrbitsSkinnedPortConfig port;
        port.imagePaths = {"res/dark-port.svg", "res/light-port.svg"};
        port.pos = EUGENE_POS(CLOCK_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
        port.pos = EUGENE_POS(SYNC_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
        port.pos = EUGENE_POS(LENGTH_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));

        port.pos = EUGENE_POS(HITS_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
        port.pos = EUGENE_POS(SHIFT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
        port.pos = EUGENE_POS(REVERSE_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
        port.pos = EUGENE_POS(INVERT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));
        port.pos = EUGENE_POS(RANDOM_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Eugene::RANDOM_CV_INPUT));

        port.pos = EUGENE_POS(BEAT_PORT);
        addOutput(createOrbitsSkinnedOutput(port, module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));
        port.pos = EUGENE_POS(EOC_PORT);
        addOutput(createOrbitsSkinnedOutput(port, module, RareBreeds_Orbits_Eugene::EOC_OUTPUT));
        // clang-format on

        rhythm_display = new EugeneRhythmDisplay(module, mm2px(Vec(14.48f, 15.1036f)), mm2px(Vec(32.f, 32.f)));
        addChild(rhythm_display);
}

void RareBreeds_Orbits_EugeneWidget::appendModuleContextMenu(Menu *menu)
{
        beat_widget.appendContextMenu(menu);
        eoc_widget.appendContextMenu(menu);

        RareBreeds_Orbits_Eugene *eugene = static_cast<RareBreeds_Orbits_Eugene *>(module);
	menu->addChild(createSubmenuItem("Randomize Filter", "",
                [=](Menu* menu) {
                        menu->addChild(createMenuLabel("Parameters that are randomized"));

                        std::vector<std::pair<std::string, size_t>> items = {
                                std::make_pair("Length", RareBreeds_Orbits_Eugene::RANDOMIZE_LENGTH),
                                std::make_pair("Length CV", RareBreeds_Orbits_Eugene::RANDOMIZE_LENGTH_CV),
                                std::make_pair("Hits", RareBreeds_Orbits_Eugene::RANDOMIZE_HITS),
                                std::make_pair("Hits CV", RareBreeds_Orbits_Eugene::RANDOMIZE_HITS_CV),
                                std::make_pair("Shift", RareBreeds_Orbits_Eugene::RANDOMIZE_SHIFT),
                                std::make_pair("Shift CV", RareBreeds_Orbits_Eugene::RANDOMIZE_SHIFT_CV),
                                std::make_pair("Reverse", RareBreeds_Orbits_Eugene::RANDOMIZE_REVERSE),
                                std::make_pair("Invert", RareBreeds_Orbits_Eugene::RANDOMIZE_INVERT)
                        };

                        for(auto i : items)
                        {
                                menu->addChild(createCheckMenuItem(i.first, "",
                                        [=]() {return eugene->m_randomization_mask & (1 << i.second);},
                                        [=]() {eugene->m_randomization_mask ^= (1 << i.second);}
                                ));
                        }
		}
	));
}

void RareBreeds_Orbits_EugeneWidget::draw(const DrawArgs& args)
{
        RareBreeds_Orbits_Eugene *module = static_cast<RareBreeds_Orbits_Eugene *>(getModule());

        // In Rack-2 dataFromJson is called on the module before the widget is created
        // The module remembers the json object so we can initialise it here
        if(module)
        {
                json_t *config = module->m_widget_config.exchange(nullptr);
                if(config)
                {
                        dataFromJson(config);
                        json_decref(config);
                }
        }

        OrbitsWidget::draw(args);
}
