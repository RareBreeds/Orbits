#include "PolygeneWidget.hpp"
#include "OrbitsConfig.hpp"
#include "OrbitsSkinned.hpp"
#include "PolygeneModule.hpp"

static OrbitsConfig g_config("res/polygene-layout.json");

struct PolygeneRhythmDisplay : TransparentWidget, OrbitsSkinned
{
        RareBreeds_Orbits_Polygene *module = NULL;
        NVGcolor m_display_accent;

        PolygeneRhythmDisplay();
        void drawLayer(const DrawArgs &args, int layer) override;
        void loadTheme(int theme) override;
};

PolygeneRhythmDisplay::PolygeneRhythmDisplay()
{
        loadTheme(g_config.getDefaultThemeId());
}

void PolygeneRhythmDisplay::loadTheme(int theme)
{
        std::array<uint8_t, 3> colour = g_config.getColour("display_accent", theme);
        m_display_accent = nvgRGB(colour[0], colour[1], colour[2]);
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
                nvgBeginPath(args.vg);
                nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                nvgFontSize(args.vg, 18);
                std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
                nvgFontFaceId(args.vg, font->handle);

                const auto active_hits = data.channels[data.active_channel_id].hits;
                nvgText(args.vg, 0.f, -6.f, std::to_string(active_hits).c_str(), NULL);

                const auto active_length = data.channels[data.active_channel_id].length;
                nvgText(args.vg, 0.f, 6.f, std::to_string(active_length).c_str(), NULL);

                const auto active_channel = data.active_channel_id + 1;
                nvgFontSize(args.vg, 12);
                nvgText(args.vg, -b.size.x / 2. + 23.f, b.size.y / 2. - 9.f, std::to_string(active_channel).c_str(), NULL);

                nvgFill(args.vg);

                // Scale to [-1, 1]
                nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

                // Flip x and y so we start at the top and positive angle
                // increments go clockwise
                nvgScale(args.vg, -1.f, -1.f);

                // Inner circle radius
                const float inner_circle_radius = 0.17f;
                const float channel_width = (1.0f - inner_circle_radius) / 16.0f;
                // Width of the line when drawing circles
                const float arc_stroke_width = channel_width / 2.0f;

                // Add a border so we don't draw over the edge
                nvgScale(args.vg, 1.0f - channel_width, 1.0f - channel_width);

                nvgStrokeWidth(args.vg, arc_stroke_width);
                for(unsigned int c = 0; c < PORT_MAX_CHANNELS; ++c)
                {
                        const unsigned int length = data.channels[c].length;
                        const float radius = 1.0f - c * channel_width;
                        const float pi2_len = 2.0f * (float)M_PI / length;
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
                                const float a0 = k * pi2_len + (float)M_PI_2;
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

RareBreeds_Orbits_PolygeneWidget::RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module)
        : OrbitsWidget(&g_config)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->m_widget = this;
                beat_widget.m_module = &module->m_beat;
                eoc_widget.m_module = &module->m_eoc;
        }

        m_theme = m_config->getDefaultThemeId();

        // clang-format off
        setPanel(APP->window->loadSvg(m_config->getSvg("panel")));

        // TODO: Screw positions are based on the panel size, could have a position for them in config based on panel size
        addChild(createOrbitsSkinnedScrew(m_config, "screw_top_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_top_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_bottom_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(m_config, "screw_bottom_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(m_config, "channel_knob", module, RareBreeds_Orbits_Polygene::CHANNEL_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(m_config, "length_knob", module, RareBreeds_Orbits_Polygene::LENGTH_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "hits_knob", module, RareBreeds_Orbits_Polygene::HITS_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(m_config, "shift_knob", module, RareBreeds_Orbits_Polygene::SHIFT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(m_config, "variation_knob", module, RareBreeds_Orbits_Polygene::VARIATION_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(m_config, "reverse_switch", module, RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(m_config, "invert_switch", module, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedButton>(m_config, "random_button", module, RareBreeds_Orbits_Polygene::RANDOM_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedButton>(m_config, "sync_button", module, RareBreeds_Orbits_Polygene::SYNC_KNOB_PARAM));

        addInput(createOrbitsSkinnedInput(m_config, "clock_port", module, RareBreeds_Orbits_Polygene::CLOCK_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "sync_port", module, RareBreeds_Orbits_Polygene::SYNC_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "length_cv_port", module, RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "hits_cv_port", module, RareBreeds_Orbits_Polygene::HITS_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "shift_cv_port", module, RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "variation_cv_port", module, RareBreeds_Orbits_Polygene::VARIATION_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "reverse_cv_port", module, RareBreeds_Orbits_Polygene::REVERSE_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "invert_cv_port", module, RareBreeds_Orbits_Polygene::INVERT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(m_config, "random_cv_port", module, RareBreeds_Orbits_Polygene::RANDOM_CV_INPUT));

        addOutput(createOrbitsSkinnedOutput(m_config, "beat_port", module, RareBreeds_Orbits_Polygene::BEAT_OUTPUT));
        addOutput(createOrbitsSkinnedOutput(m_config, "eoc_port", module, RareBreeds_Orbits_Polygene::EOC_OUTPUT));
        // clang-format on

        PolygeneRhythmDisplay *r = createWidget<PolygeneRhythmDisplay>(m_config->getPos("display"));
        r->module = module;
        r->box.size = m_config->getSize("display");
        addChild(r);
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
