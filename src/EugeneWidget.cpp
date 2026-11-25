#include "EugeneComponentPositions.h"
#include "EugeneThemes.h"
#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "OrbitsSkinned.hpp"

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
                std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, ORBITS_ASSET_FONTS_SHARETECHMONO_REGULAR_TTF));
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

                        if(orbitsEuclideanBeat(m_data.length, m_data.hits, m_data.shift, k, m_data.invert))
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
#define EUGENE_SIZE(NAME) ORBITS_SIZE(EUGENE_##NAME)

static const OrbitsSkinnedPanelConfig g_config = {{EUGENE_DARK_PANEL, EUGENE_LIGHT_PANEL}};
RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *eugene) : OrbitsWidget(g_config)
{
        setModule(eugene);

        // Module may be NULL if this is the module selection screen
        if(eugene)
        {
                eugene->m_widget = this;
                beat_widget.m_module = &eugene->m_beat;
                eoc_widget.m_module = &eugene->m_eoc;
        }

        // clang-format off
        OrbitsSkinnedScrewConfig screw;
        screw.imagePaths = {EUGENE_DARK_SCREW, EUGENE_LIGHT_SCREW};
        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        // TODO: Autogenerate config for each param/input/output?
        OrbitsSkinnedKnobConfig knob;
        knob.imagePaths = {EUGENE_DARK_LENGTH_KNOB, EUGENE_LIGHT_LENGTH_KNOB};
        knob.pos = EUGENE_POS(LENGTH_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, eugene, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));

        knob.imagePaths = {EUGENE_DARK_HITS_KNOB, EUGENE_LIGHT_HITS_KNOB};
        knob.pos = EUGENE_POS(HITS_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, eugene, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));

        knob.imagePaths = {EUGENE_DARK_SHIFT_KNOB, EUGENE_LIGHT_SHIFT_KNOB};
        knob.pos = EUGENE_POS(SHIFT_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, eugene, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));

        knob.imagePaths = {EUGENE_DARK_LENGTH_CV_KNOB, EUGENE_LIGHT_LENGTH_CV_KNOB};
        knob.pos = EUGENE_POS(LENGTH_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, eugene, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));

        knob.imagePaths = {EUGENE_DARK_HITS_CV_KNOB, EUGENE_LIGHT_HITS_CV_KNOB};
        knob.pos = EUGENE_POS(HITS_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, eugene, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));

        knob.imagePaths = {EUGENE_DARK_SHIFT_CV_KNOB, EUGENE_LIGHT_SHIFT_CV_KNOB};
        knob.pos = EUGENE_POS(SHIFT_CV_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, eugene, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));

        OrbitsSkinnedSwitchConfig swtch;
        swtch.onImagePaths = {EUGENE_DARK_REVERSE_SWITCH_ON, EUGENE_LIGHT_REVERSE_SWITCH_ON};
        swtch.offImagePaths = {EUGENE_DARK_REVERSE_SWITCH_OFF, EUGENE_LIGHT_REVERSE_SWITCH_OFF};
        swtch.pos = EUGENE_POS(REVERSE_SWITCH);
        addParam(createOrbitsSkinnedSwitch(swtch, eugene, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));

        swtch.onImagePaths = {EUGENE_DARK_INVERT_SWITCH_ON, EUGENE_LIGHT_INVERT_SWITCH_ON};
        swtch.offImagePaths = {EUGENE_DARK_INVERT_SWITCH_OFF, EUGENE_LIGHT_INVERT_SWITCH_OFF};
        swtch.pos = EUGENE_POS(INVERT_SWITCH);
        addParam(createOrbitsSkinnedSwitch(swtch, eugene, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

        swtch.onImagePaths = {EUGENE_DARK_SYNC_BUTTON_ON, EUGENE_LIGHT_SYNC_BUTTON_ON};
        swtch.offImagePaths = {EUGENE_DARK_SYNC_BUTTON_OFF, EUGENE_LIGHT_SYNC_BUTTON_OFF};
        swtch.pos = EUGENE_POS(SYNC_BUTTON);
        addParam(createOrbitsSkinnedButton(swtch, eugene, RareBreeds_Orbits_Eugene::SYNC_KNOB_PARAM));

        OrbitsSkinnedPortConfig port;
        port.imagePaths = {EUGENE_DARK_CLOCK_PORT, EUGENE_LIGHT_CLOCK_PORT};
        port.pos = EUGENE_POS(CLOCK_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
        port.imagePaths = {EUGENE_DARK_SYNC_PORT, EUGENE_LIGHT_SYNC_PORT};
        port.pos = EUGENE_POS(SYNC_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::SYNC_INPUT));
        port.imagePaths = {EUGENE_DARK_LENGTH_CV_PORT, EUGENE_LIGHT_LENGTH_CV_PORT};
        port.pos = EUGENE_POS(LENGTH_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));

        port.imagePaths = {EUGENE_DARK_HITS_CV_PORT, EUGENE_LIGHT_HITS_CV_PORT};
        port.pos = EUGENE_POS(HITS_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
        port.imagePaths = {EUGENE_DARK_SHIFT_CV_PORT, EUGENE_LIGHT_SHIFT_CV_PORT};
        port.pos = EUGENE_POS(SHIFT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
        port.imagePaths = {EUGENE_DARK_REVERSE_CV_PORT, EUGENE_LIGHT_REVERSE_CV_PORT};
        port.pos = EUGENE_POS(REVERSE_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
        port.imagePaths = {EUGENE_DARK_INVERT_CV_PORT, EUGENE_LIGHT_INVERT_CV_PORT};
        port.pos = EUGENE_POS(INVERT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));
        port.imagePaths = {EUGENE_DARK_RANDOM_CV_PORT, EUGENE_LIGHT_RANDOM_CV_PORT};
        port.pos = EUGENE_POS(RANDOM_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, eugene, RareBreeds_Orbits_Eugene::RANDOM_CV_INPUT));

        port.imagePaths = {EUGENE_DARK_BEAT_PORT, EUGENE_LIGHT_BEAT_PORT};
        port.pos = EUGENE_POS(BEAT_PORT);
        addOutput(createOrbitsSkinnedOutput(port, eugene, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));
        port.imagePaths = {EUGENE_DARK_EOC_PORT, EUGENE_LIGHT_EOC_PORT};
        port.pos = EUGENE_POS(EOC_PORT);
        addOutput(createOrbitsSkinnedOutput(port, eugene, RareBreeds_Orbits_Eugene::EOC_OUTPUT));
        // clang-format on

        rhythm_display = new EugeneRhythmDisplay(eugene, EUGENE_POS(DISPLAY), EUGENE_SIZE(DISPLAY));
        addChild(rhythm_display);

        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[toIndex(kOrbitsThemeDefault)]);
        setPanel(APP->window->loadSvg(panelPath));
}

void RareBreeds_Orbits_EugeneWidget::appendModuleContextMenu(Menu *menu)
{
        beat_widget.appendContextMenu(menu);
        eoc_widget.appendContextMenu(menu);

        RareBreeds_Orbits_Eugene *eugene = static_cast<RareBreeds_Orbits_Eugene *>(module);
	menu->addChild(createSubmenuItem("Randomize Filter", "",
                [=](Menu* submenu) {
                        submenu->addChild(createMenuLabel("Parameters that are randomized"));

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
                                submenu->addChild(createCheckMenuItem(i.first, "",
                                        [=]() {return eugene->m_randomization_mask & (1 << i.second);},
                                        [=]() {eugene->m_randomization_mask ^= (1 << i.second);}
                                ));
                        }
		}
	));
}

void RareBreeds_Orbits_EugeneWidget::draw(const DrawArgs& args)
{
        RareBreeds_Orbits_Eugene *eugene = static_cast<RareBreeds_Orbits_Eugene *>(getModule());

        // In Rack-2 dataFromJson is called on the module before the widget is created
        // The module remembers the json object so we can initialise it here
        if(eugene)
        {
                json_t *config = eugene->m_widget_config.exchange(nullptr);
                if(config)
                {
                        dataFromJson(config);
                        json_decref(config);
                }
        }

        OrbitsWidget::draw(args);
}
