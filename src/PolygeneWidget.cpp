#include "PolygeneComponentPositions.h"
#include "PolygeneThemes.h"
#include "PolygeneWidget.hpp"
#include "OrbitsAssets.h"
#include "OrbitsSkinned.hpp"
#include "PolygeneModule.hpp"

struct PolygeneRhythmDisplayConfig
{
        std::array<NVGcolor, ORBITS_THEME_COUNT> accent;
};

struct PolygeneRhythmDisplay : TransparentWidget, OrbitsSkinned
{
        RareBreeds_Orbits_Polygene *module = NULL;
        PolygeneRhythmDisplayConfig m_config;
        NVGcolor m_display_accent;

        PolygeneRhythmDisplay();
        void drawLayer(const DrawArgs &args, int layer) override;
        void loadTheme(int theme) override;
};

PolygeneRhythmDisplay::PolygeneRhythmDisplay()
{
        loadTheme(ORBITS_THEME_DEFAULT);
}

void PolygeneRhythmDisplay::loadTheme(int theme)
{
        m_display_accent = m_config.accent[theme];
}

void PolygeneRhythmDisplay::drawLayer(const DrawArgs &args, int layer)
{
        // Drawings to layer 1 don't dim when the room lights are dimmed
        if(layer == 1)
        {
                PolygeneDisplayData data = RareBreeds_Orbits_Polygene::getDisplayData(module);

                nvgGlobalTint(args.vg, color::WHITE);
                const auto foreground_color = color::WHITE;
                nvgStrokeColor(args.vg, foreground_color);
                nvgSave(args.vg);

                const Rect b = Rect(Vec(0, 0), box.size);
                nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

                // Everything drawn after here is in the foreground colour
                nvgStrokeColor(args.vg, foreground_color);
                nvgFillColor(args.vg, foreground_color);

                // Translate so (0, 0) is the center of the screen
                nvgTranslate(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Draw length text center bottom and hits text center top
                std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, ORBITS_ASSET_FONTS_SHARETECHMONO_REGULAR_TTF));
                if (font)
                {
                        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                        nvgFontSize(args.vg, 18);

                        nvgFontFaceId(args.vg, font->handle);

                        const auto active_hits = data.channels[data.active_channel_id].hits;
                        nvgText(args.vg, 0.f, -6.f, std::to_string(active_hits).c_str(), NULL);

                        const auto active_length = data.channels[data.active_channel_id].length;
                        nvgText(args.vg, 0.f, 6.f, std::to_string(active_length).c_str(), NULL);

                        const auto active_channel = data.active_channel_id + 1;
                        nvgFontSize(args.vg, 12);
                        nvgText(args.vg, -b.size.x / 2.f + 23.f, b.size.y / 2.f - 9.f, std::to_string(active_channel).c_str(), NULL);
                }

                // Scale to [-1, 1]
                nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Flip x and y so we start at the top and positive angle
                // increments go clockwise
                nvgScale(args.vg, -1.f, -1.f);

                // Inner circle radius
                const float inner_circle_radius = 0.17f;
                const float channel_width = (1.0f - inner_circle_radius) / (float)PORT_MAX_CHANNELS;
                // Width of the line when drawing circles
                const float arc_stroke_width = channel_width / 2.0f;

                // Add a border so we don't draw over the edge
                nvgScale(args.vg, 1.0f - channel_width, 1.0f - channel_width);

                nvgStrokeWidth(args.vg, arc_stroke_width);
                for(uint8_t c = 0; c < PORT_MAX_CHANNELS; ++c)
                {
                        const unsigned int length = data.channels[c].length;
                        const float radius = 1.0f - c * channel_width;
                        const float mpif = static_cast<float>(M_PI);
                        const float lenf = static_cast<float>(length);
                        const float pi2_len = 2.0f * mpif / lenf;
                        const float beat_gap = 0.06f;
                        const float len = pi2_len - beat_gap;

                        NVGcolor dash_colour;
                        if(c == data.active_channel_id)
                        {
                                dash_colour = m_display_accent;
                        }
                        else if(c < data.active_channels)
                        {
                                dash_colour = nvgRGB(0xff, 0xff, 0xff);
                        }
                        else
                        {
                                dash_colour = nvgRGB(0x50, 0x50, 0x50);
                        }

                        nvgStrokeColor(args.vg, dash_colour);
                        nvgFillColor(args.vg, dash_colour);
                        // The engine is run in a different thread, m_current_step is only updated on each clock
                        // cycle so may not have been wrapped to a new length parameter yet. If the current step
                        // is out of bounds then display it at 0.
                        unsigned int current_step = data.channels[c].current_step;
                        for(unsigned int k = 0; k < length; ++k)
                        {
                                const float mpif2 = static_cast<float>(M_PI_2);
                                const float kf = static_cast<float>(k);
                                const float a0 = kf * pi2_len + mpif2;
                                const float a1 = a0 + len;

                                if(current_step == k)
                                {
                                        const auto center = a0 - beat_gap / 2.f;
                                        nvgBeginPath(args.vg);
                                        nvgCircle(args.vg,
                                                  radius * cosf(center),
                                                  radius * sinf(center),
                                                  arc_stroke_width / 2.0f);
                                        nvgFill(args.vg);
                                }

                                auto on_beat = RareBreeds_Orbits_Polygene::Channel::isOnBeat(length, data.channels[c].hits, data.channels[c].shift,
                                                                                   data.channels[c].variation, k, data.channels[c].invert);
                                if(on_beat)
                                {
                                        nvgBeginPath(args.vg);
                                        nvgArc(args.vg, 0.0f, 0.0f, radius, a0, a1, NVG_CW);
                                        nvgStroke(args.vg);
                                }
                        }
                }

                nvgResetScissor(args.vg);
                nvgRestore(args.vg);
        }

        Widget::drawLayer(args, layer);
}

#define POLYGENE_POS(NAME) ORBITS_POS(POLYGENE_##NAME)
#define POLYGENE_SIZE(NAME) ORBITS_SIZE(POLYGENE_##NAME)

static const OrbitsSkinnedPanelConfig g_config = {.panelPaths = {POLYGENE_DARK_PANEL, POLYGENE_LIGHT_PANEL}};
RareBreeds_Orbits_PolygeneWidget::RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module)
        : OrbitsWidget(g_config)
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
        OrbitsSkinnedScrewConfig screw;
        screw.imagePaths = {POLYGENE_DARK_SCREW, POLYGENE_LIGHT_SCREW};
        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));


        OrbitsSkinnedKnobConfig knob;
        knob.imagePaths = {POLYGENE_DARK_CHANNEL_KNOB, POLYGENE_LIGHT_CHANNEL_KNOB};
        knob.pos = POLYGENE_POS(CHANNEL_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, module, RareBreeds_Orbits_Polygene::CHANNEL_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_LENGTH_KNOB, POLYGENE_LIGHT_LENGTH_KNOB};
        knob.pos = POLYGENE_POS(LENGTH_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, module, RareBreeds_Orbits_Polygene::LENGTH_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_HITS_KNOB, POLYGENE_LIGHT_HITS_KNOB};
        knob.pos = POLYGENE_POS(HITS_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Polygene::HITS_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_SHIFT_KNOB, POLYGENE_LIGHT_SHIFT_KNOB};
        knob.pos = POLYGENE_POS(SHIFT_KNOB);
        addParam(createOrbitsSkinnedSnappyKnob(knob, module, RareBreeds_Orbits_Polygene::SHIFT_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_VARIATION_KNOB, POLYGENE_LIGHT_VARIATION_KNOB};
        knob.pos = POLYGENE_POS(VARIATION_KNOB);
        addParam(createOrbitsSkinnedKnob(knob, module, RareBreeds_Orbits_Polygene::VARIATION_KNOB_PARAM));

        OrbitsSkinnedSwitchConfig swtch;
        swtch.onImagePaths = {POLYGENE_DARK_REVERSE_SWITCH_ON, POLYGENE_LIGHT_REVERSE_SWITCH_ON};
        swtch.offImagePaths = {POLYGENE_DARK_REVERSE_SWITCH_OFF, POLYGENE_LIGHT_REVERSE_SWITCH_OFF};
        swtch.pos = POLYGENE_POS(REVERSE_SWITCH);
        addParam(createOrbitsSkinnedSwitch(swtch, module, RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_INVERT_SWITCH_ON, POLYGENE_LIGHT_INVERT_SWITCH_ON};
        swtch.offImagePaths = {POLYGENE_DARK_INVERT_SWITCH_OFF, POLYGENE_LIGHT_INVERT_SWITCH_OFF};
        swtch.pos = POLYGENE_POS(INVERT_SWITCH);
        addParam(createOrbitsSkinnedSwitch(swtch, module, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_RANDOM_BUTTON_ON, POLYGENE_LIGHT_RANDOM_BUTTON_ON};
        swtch.offImagePaths = {POLYGENE_DARK_RANDOM_BUTTON_OFF, POLYGENE_LIGHT_RANDOM_BUTTON_OFF};
        swtch.pos = POLYGENE_POS(RANDOM_BUTTON);
        addParam(createOrbitsSkinnedButton(swtch, module, RareBreeds_Orbits_Polygene::RANDOM_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_SYNC_BUTTON_ON, POLYGENE_LIGHT_SYNC_BUTTON_ON};
        swtch.offImagePaths = {POLYGENE_DARK_SYNC_BUTTON_OFF, POLYGENE_LIGHT_SYNC_BUTTON_OFF};
        swtch.pos = POLYGENE_POS(SYNC_BUTTON);
        addParam(createOrbitsSkinnedButton(swtch, module, RareBreeds_Orbits_Polygene::SYNC_KNOB_PARAM));

        OrbitsSkinnedPortConfig port;
        port.imagePaths = {POLYGENE_DARK_CLOCK_PORT, POLYGENE_LIGHT_CLOCK_PORT};
        port.pos = POLYGENE_POS(CLOCK_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::CLOCK_INPUT));
        port.imagePaths = {POLYGENE_DARK_SYNC_PORT, POLYGENE_LIGHT_SYNC_PORT};
        port.pos = POLYGENE_POS(SYNC_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::SYNC_INPUT));
        port.imagePaths = {POLYGENE_DARK_LENGTH_CV_PORT, POLYGENE_LIGHT_LENGTH_CV_PORT};
        port.pos = POLYGENE_POS(LENGTH_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_HITS_CV_PORT, POLYGENE_LIGHT_HITS_CV_PORT};
        port.pos = POLYGENE_POS(HITS_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::HITS_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_SHIFT_CV_PORT, POLYGENE_LIGHT_SHIFT_CV_PORT};
        port.pos = POLYGENE_POS(SHIFT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_VARIATION_CV_PORT, POLYGENE_LIGHT_VARIATION_CV_PORT};
        port.pos = POLYGENE_POS(VARIATION_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::VARIATION_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_REVERSE_CV_PORT, POLYGENE_LIGHT_REVERSE_CV_PORT};
        port.pos = POLYGENE_POS(REVERSE_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::REVERSE_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_INVERT_CV_PORT, POLYGENE_LIGHT_INVERT_CV_PORT};
        port.pos = POLYGENE_POS(INVERT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::INVERT_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_RANDOM_CV_PORT, POLYGENE_LIGHT_RANDOM_CV_PORT};
        port.pos = POLYGENE_POS(RANDOM_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, module, RareBreeds_Orbits_Polygene::RANDOM_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_BEAT_PORT, POLYGENE_LIGHT_BEAT_PORT};
        port.pos = POLYGENE_POS(BEAT_PORT);
        addOutput(createOrbitsSkinnedOutput(port, module, RareBreeds_Orbits_Polygene::BEAT_OUTPUT));
        port.imagePaths = {POLYGENE_DARK_EOC_PORT, POLYGENE_LIGHT_EOC_PORT};
        port.pos = POLYGENE_POS(EOC_PORT);
        addOutput(createOrbitsSkinnedOutput(port, module, RareBreeds_Orbits_Polygene::EOC_OUTPUT));
        // clang-format on

        PolygeneRhythmDisplay *r = createWidget<PolygeneRhythmDisplay>(POLYGENE_POS(DISPLAY));
        r->m_config = {{POLYGENE_DARK_DISPLAY_ACCENT, POLYGENE_LIGHT_DISPLAY_ACCENT}};
        r->module = module;
        r->box.size = POLYGENE_SIZE(DISPLAY);
        addChild(r);

        loadTheme(ORBITS_THEME_DEFAULT);
}

static void ToggleInputMode(InputMode *mode)
{
        switch(*mode)
        {
                case INPUT_MODE_MONOPHONIC_COPIES_TO_ALL:
                        *mode = INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST;
                        break;
                case INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST:
                default:
                        *mode = INPUT_MODE_MONOPHONIC_COPIES_TO_ALL;
                        break;
        }
}

void RareBreeds_Orbits_PolygeneWidget::appendModuleContextMenu(Menu *menu)
{
        beat_widget.appendContextMenu(menu);
        eoc_widget.appendContextMenu(menu);

        RareBreeds_Orbits_Polygene *polygene = static_cast<RareBreeds_Orbits_Polygene *>(module);

        menu->addChild(createSubmenuItem("Randomize Filter", "",
                [=](Menu* menu) {
                        menu->addChild(createMenuLabel("Parameters that are randomized"));

                        std::vector<std::pair<std::string, size_t>> items = {
                                std::make_pair("Length", RareBreeds_Orbits_Polygene::RANDOMIZE_LENGTH),
                                std::make_pair("Hits", RareBreeds_Orbits_Polygene::RANDOMIZE_HITS),
                                std::make_pair("Shift", RareBreeds_Orbits_Polygene::RANDOMIZE_SHIFT),
                                std::make_pair("Variation", RareBreeds_Orbits_Polygene::RANDOMIZE_VARIATION),
                                std::make_pair("Reverse", RareBreeds_Orbits_Polygene::RANDOMIZE_REVERSE),
                                std::make_pair("Invert", RareBreeds_Orbits_Polygene::RANDOMIZE_INVERT)
                        };

                        for(auto i : items)
                        {
                                menu->addChild(createCheckMenuItem(i.first, "",
                                        [=]() {return polygene->m_randomization_mask & (1 << i.second);},
                                        [=]() {polygene->m_randomization_mask ^= (1 << i.second);}
                                ));
                        }
                }
        ));

        menu->addChild(createSubmenuItem("Mono Input Behavior", "",
                [=](Menu* menu) {
                        menu->addChild(createMenuLabel("Make mono cables only control the first channel"));

                        InputMode *input_mode_array = polygene->m_input_mode;
                        std::vector<std::pair<std::string, size_t>> items = {
                                std::make_pair("Sync", RareBreeds_Orbits_Polygene::SYNC_INPUT),
                                std::make_pair("Length", RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT),
                                std::make_pair("Hits", RareBreeds_Orbits_Polygene::HITS_CV_INPUT),
                                std::make_pair("Shift", RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT),
                                std::make_pair("Variation", RareBreeds_Orbits_Polygene::VARIATION_CV_INPUT),
                                std::make_pair("Random", RareBreeds_Orbits_Polygene::RANDOM_CV_INPUT)

                        };

                        for(auto i : items)
                        {
                                menu->addChild(createCheckMenuItem(i.first, "",
                                        [=]() {return input_mode_array[i.second];},
                                        [=]() {ToggleInputMode(&input_mode_array[i.second]);}
                                ));
                        }
                }
        ));
}

void RareBreeds_Orbits_PolygeneWidget::draw(const DrawArgs& args)
{
        RareBreeds_Orbits_Polygene *module = static_cast<RareBreeds_Orbits_Polygene *>(getModule());

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
