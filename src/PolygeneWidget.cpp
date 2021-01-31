#include "PolygeneWidget.hpp"
#include "OrbitsConfig.hpp"
#include "OrbitsSkinned.hpp"
#include "PolygeneModule.hpp"

static OrbitsConfig config("res/polygene-layout.json");

struct PolygeneRhythmDisplay : TransparentWidget
{
        RareBreeds_Orbits_Polygene *module = NULL;
        std::shared_ptr<Font> font;

        PolygeneRhythmDisplay();
        void draw(const DrawArgs &args) override;
        void drawCircles(const DrawArgs &args);
        void drawArcs(const DrawArgs &args);
};

PolygeneRhythmDisplay::PolygeneRhythmDisplay()
{
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
}

void PolygeneRhythmDisplay::draw(const DrawArgs &args)
{
        drawArcs(args);
}

void PolygeneRhythmDisplay::drawCircles(const DrawArgs &args)
{
        if(!module)
        {
                return;
        }

        const auto length = module->m_active_channel->readLength();
        const auto hits = module->m_active_channel->readHits(length);
        const auto shift = module->m_active_channel->readShift(length);
        const auto oddity = module->m_active_channel->readOddity(length, hits);
        const auto invert = module->m_active_channel->readInvert();
        const auto reverse = module->m_active_channel->readReverse();

        const auto foreground_color = color::WHITE;
        nvgStrokeColor(args.vg, foreground_color);
        nvgSave(args.vg);

        const Rect b = Rect(Vec(0, 0), box.size);
        nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

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

                auto on_beat = module->m_active_channel->isOnBeat(length, hits, shift, oddity, k, invert);
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

void PolygeneRhythmDisplay::drawArcs(const DrawArgs &args)
{
        if(!module)
        {
                return;
        }

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

        // Scale to [-1, 1]
        nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Flip x and y so we start at the top and positive angle
        // increments go clockwise
        nvgScale(args.vg, -1.f, -1.f);

        // Inner circle radius
        const auto inner_circle_radius = 0.1f;
        const auto channel_width = (1.0f - inner_circle_radius) / 16.0f;
        // Width of the line when drawing circles
        const auto arc_stroke_width = channel_width / 2.0f;
        nvgStrokeWidth(args.vg, arc_stroke_width);

        // Add a border so we don't draw over the edge
        nvgScale(args.vg, 1.0 - channel_width, 1.0 - channel_width);

        int c = 0;
        for(auto channel : module->m_channels)
        {
                const auto length = channel.readLength();
                const auto hits = channel.readHits(length);
                const auto shift = channel.readShift(length);
                const auto oddity = channel.readOddity(length, hits);
                const auto invert = channel.readInvert();
                const auto reverse = channel.readReverse();

                for(auto k = 0u; k < length; ++k)
                {
                        auto current_step = channel.m_current_step == k;
                        auto on_beat = channel.isOnBeat(length, hits, shift, oddity, k, invert);
                        if(on_beat)
                        {
                                if(current_step)
                                {
                                        nvgStrokeColor(args.vg, color::WHITE);
                                }
                                else
                                {
                                        nvgStrokeColor(args.vg, nvgRGB(0xcc, 0xcc, 0xcc));
                                }
                        }
                        else if(current_step)
                        {
                                nvgStrokeColor(args.vg, nvgRGB(0x50, 0x50, 0x50));
                        }

                        if(on_beat || current_step)
                        {
                                const auto radius = 1.0f - c * channel_width;
                                const auto pi2_len = 2.0f * M_PI / length;
                                auto a0 = k * pi2_len + M_PI_2;
                                const auto len = pi2_len - 0.05f;
                                auto a1 = a0 + len;
                                nvgBeginPath(args.vg);
                                nvgArc(args.vg, 0.0f, 0.0f, radius, a0, a1, NVG_CW);
                                nvgStroke(args.vg);
                        }
                }
                ++c;
        }

        nvgResetScissor(args.vg);
        nvgRestore(args.vg);
}

struct PolygeneThemeChoiceItem : MenuItem
{
        RareBreeds_Orbits_PolygeneWidget *m_widget;
        int m_id;

        PolygeneThemeChoiceItem(RareBreeds_Orbits_PolygeneWidget *widget, int id, const char *name)
        {
                m_widget = widget;
                m_id = id;
                text = name;
                rightText = CHECKMARK(widget->m_theme == id);
        }

        void onAction(const event::Action &e) override
        {
                m_widget->loadTheme(m_id);
        }
};

RareBreeds_Orbits_PolygeneWidget::RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->widget = this;
        }

        m_theme = config.getDefaultThemeId();

        // clang-format off
        setPanel(APP->window->loadSvg(config.getSvg("panel")));

        // TODO: Screw positions are based on the panel size, could have a position for them in config based on panel size
        addChild(createOrbitsSkinnedScrew(&config, "screw_top_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(&config, "screw_top_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(&config, "screw_bottom_left", Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
        addChild(createOrbitsSkinnedScrew(&config, "screw_bottom_right", Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "channel_knob", module, RareBreeds_Orbits_Polygene::CHANNEL_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "length_knob", module, RareBreeds_Orbits_Polygene::LENGTH_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "hits_knob", module, RareBreeds_Orbits_Polygene::HITS_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "shift_knob", module, RareBreeds_Orbits_Polygene::SHIFT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "oddity_knob", module, RareBreeds_Orbits_Polygene::ODDITY_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "length_cv_knob", module, RareBreeds_Orbits_Polygene::LENGTH_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "hits_cv_knob", module, RareBreeds_Orbits_Polygene::HITS_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "shift_cv_knob", module, RareBreeds_Orbits_Polygene::SHIFT_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(&config, "oddity_cv_knob", module, RareBreeds_Orbits_Polygene::ODDITY_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(&config, "reverse_switch", module, RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(&config, "invert_switch", module, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM));

        addInput(createOrbitsSkinnedInput(&config, "clock_port", module, RareBreeds_Orbits_Polygene::CLOCK_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "sync_port", module, RareBreeds_Orbits_Polygene::SYNC_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "length_cv_port", module, RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "hits_cv_port", module, RareBreeds_Orbits_Polygene::HITS_CV_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "shift_cv_port", module, RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "oddity_cv_port", module, RareBreeds_Orbits_Polygene::ODDITY_CV_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "reverse_cv_port", module, RareBreeds_Orbits_Polygene::REVERSE_CV_INPUT));
        addInput(createOrbitsSkinnedInput(&config, "invert_cv_port", module, RareBreeds_Orbits_Polygene::INVERT_CV_INPUT));

        addOutput(createOrbitsSkinnedOutput(&config, "beat_port", module, RareBreeds_Orbits_Polygene::BEAT_OUTPUT));
        // clang-format on

        PolygeneRhythmDisplay *r = createWidget<PolygeneRhythmDisplay>(config.getPos("display"));
        r->module = module;
        r->box.size = config.getSize("display");
        addChild(r);
}

void RareBreeds_Orbits_PolygeneWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);
        MenuLabel *theme_label = new MenuLabel;
        theme_label->text = "Theme";
        menu->addChild(theme_label);

        for(size_t i = 0; i < config.numThemes(); ++i)
        {
                menu->addChild(new PolygeneThemeChoiceItem(this, i, config.getThemeName(i).c_str()));
        }
}

void RareBreeds_Orbits_PolygeneWidget::loadTheme(const char *theme)
{
        for(size_t i = 0; i < config.numThemes(); ++i)
        {
                if(config.getThemeName(i) == theme)
                {
                        loadTheme(i);
                        break;
                }
        }
}

void RareBreeds_Orbits_PolygeneWidget::loadTheme(int theme)
{
        m_theme = theme;

        for(auto child : children)
        {
                OrbitsSkinned *skinned = dynamic_cast<OrbitsSkinned *>(child);
                if(skinned)
                {
                        skinned->loadTheme(theme);
                }
        }

        setPanel(APP->window->loadSvg(config.getSvg("panel", theme)));
}

json_t *RareBreeds_Orbits_PolygeneWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_t *theme = json_string(config.getThemeName(m_theme).c_str());
                if(theme)
                {
                        json_object_set_new(root, "theme", theme);
                }
        }
        return root;
}

void RareBreeds_Orbits_PolygeneWidget::dataFromJson(json_t *root)
{
        if(root)
        {
                json_t *obj = json_object_get(root, "theme");
                if(obj)
                {
                        loadTheme(json_string_value(obj));
                }
        }
}
