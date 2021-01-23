#include "PolygeneWidget.hpp"
#include "PolygeneConfig.hpp"
#include "PolygeneModule.hpp"

// Interface for components with the ability to change skins
struct PolygeneSkinned
{
        virtual void loadTheme(int theme);
};

struct PolygeneSkinnedKnob : RoundKnob, PolygeneSkinned
{
        PolygeneComponents m_component;

        PolygeneSkinnedKnob(PolygeneComponents component)
        {
                m_component = component;
                loadTheme(polygene_config.m_default);
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(polygene_config.getSvg(m_component, theme)));
                fb->dirty = true;
        }
};

struct PolygeneSkinnedScrew : app::SvgScrew, PolygeneSkinned
{
        PolygeneComponents m_component;

        PolygeneSkinnedScrew(PolygeneComponents component)
        {
                m_component = component;
                loadTheme(polygene_config.m_default);
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(polygene_config.getSvg(m_component, theme)));
                fb->dirty = true;
        }
};

struct PolygeneSkinnedSwitch : app::SvgSwitch, PolygeneSkinned
{
        PolygeneComponents m_component;

        PolygeneSkinnedSwitch(PolygeneComponents component)
        {
                m_component = component;
                // Relies on the OFF value being one after ON
                addFrame(APP->window->loadSvg(polygene_config.getSvg(PolygeneComponents(m_component + 1))));
                addFrame(APP->window->loadSvg(polygene_config.getSvg(m_component)));
                shadow->opacity = 0.0;
        }

        void loadTheme(int theme) override
        {
                // Relies on the OFF value being one after ON
                frames[0] = APP->window->loadSvg(polygene_config.getSvg(PolygeneComponents(m_component + 1), theme));
                frames[1] = APP->window->loadSvg(polygene_config.getSvg(m_component, theme));

                event::Change change;
                onChange(change);
                onChange(change);
        }
};

struct PolygeneSkinnedPort : app::SvgPort, PolygeneSkinned
{
        PolygeneComponents m_component;

        PolygeneSkinnedPort(PolygeneComponents component)
        {
                m_component = component;
                loadTheme(polygene_config.m_default);
                shadow->opacity = 0.07;
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(polygene_config.getSvg(m_component, theme)));
                // fb->dirty = true; // Already set by setSvg for SvgPorts
        }
};

struct PolygeneRhythmDisplay : TransparentWidget
{
        RareBreeds_Orbits_Polygene *module = NULL;
        std::shared_ptr<Font> font;

        PolygeneRhythmDisplay();
        void draw(const DrawArgs &args) override;
};

PolygeneRhythmDisplay::PolygeneRhythmDisplay()
{
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/ShareTechMono-Regular.ttf"));
}

void PolygeneRhythmDisplay::draw(const DrawArgs &args)
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

        const auto inverted_display = false;
        const auto background_color = inverted_display ? color::WHITE : color::BLACK;
        const auto foreground_color = inverted_display ? color::BLACK : color::WHITE;
        nvgStrokeColor(args.vg, color::WHITE);
        nvgSave(args.vg);

        const Rect b = Rect(Vec(0, 0), box.size);
        nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

        // Only the background is drawn in the background colour
        nvgStrokeColor(args.vg, background_color);
        nvgFillColor(args.vg, background_color);
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y, 5.f);
        nvgFill(args.vg);

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

// TODO: Consider moving screw positions to the config
static PolygeneSkinnedScrew *createSkinnedScrew(PolygeneComponents component, math::Vec pos)
{
        PolygeneSkinnedScrew *o = new PolygeneSkinnedScrew(component);
        o->box.pos = pos.minus(o->box.size.div(2));
        return o;
}

template <class TParamWidget>
static TParamWidget *createSkinnedParam(PolygeneComponents component, engine::Module *module, int paramId)
{
        TParamWidget *o = new TParamWidget(component);
        o->box.pos = polygene_config.getPos(component).minus(o->box.size.div(2));
        if(module)
        {
                o->paramQuantity = module->paramQuantities[paramId];
        }
        return o;
}

static PolygeneSkinnedPort *createSkinnedPort(PolygeneComponents component, engine::Module *module, int portId)
{
        PolygeneSkinnedPort *o = new PolygeneSkinnedPort(component);
        o->box.pos = polygene_config.getPos(component).minus(o->box.size.div(2));
        o->module = module;
        o->portId = portId;
        return o;
}

static PolygeneSkinnedPort *createSkinnedInput(PolygeneComponents component, engine::Module *module, int inputId)
{
        PolygeneSkinnedPort *o = createSkinnedPort(component, module, inputId);
        o->type = app::PortWidget::INPUT;
        return o;
}

static PolygeneSkinnedPort *createSkinnedOutput(PolygeneComponents component, engine::Module *module, int outputId)
{
        PolygeneSkinnedPort *o = createSkinnedPort(component, module, outputId);
        o->type = app::PortWidget::OUTPUT;
        return o;
}

RareBreeds_Orbits_PolygeneWidget::RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->widget = this;
        }

        // clang-format off
        setPanel(APP->window->loadSvg(polygene_config.getSvg(POLYGENE_COMPONENT_PANEL)));

        // TODO: Screw positions are based on the panel size, could have a position for them in polygene_config based on panel size 
        addChild(createSkinnedScrew(POLYGENE_COMPONENT_SCREW_TOP_LEFT, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(POLYGENE_COMPONENT_SCREW_TOP_RIGHT, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(POLYGENE_COMPONENT_SCREW_BOTTOM_LEFT, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(POLYGENE_COMPONENT_SCREW_BOTTOM_RIGHT, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_CHANNEL_KNOB, module, RareBreeds_Orbits_Polygene::CHANNEL_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_LENGTH_KNOB, module, RareBreeds_Orbits_Polygene::LENGTH_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_HITS_KNOB, module, RareBreeds_Orbits_Polygene::HITS_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_SHIFT_KNOB, module, RareBreeds_Orbits_Polygene::SHIFT_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_ODDITY_KNOB, module, RareBreeds_Orbits_Polygene::ODDITY_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_LENGTH_CV_KNOB, module, RareBreeds_Orbits_Polygene::LENGTH_CV_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_HITS_CV_KNOB, module, RareBreeds_Orbits_Polygene::HITS_CV_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_SHIFT_CV_KNOB, module, RareBreeds_Orbits_Polygene::SHIFT_CV_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedKnob>(POLYGENE_COMPONENT_ODDITY_CV_KNOB, module, RareBreeds_Orbits_Polygene::ODDITY_CV_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedSwitch>(POLYGENE_COMPONENT_REVERSE_SWITCH_ON, module, RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM));
        addParam(createSkinnedParam<PolygeneSkinnedSwitch>(POLYGENE_COMPONENT_INVERT_SWITCH_ON, module, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM));

        addInput(createSkinnedInput(POLYGENE_COMPONENT_CLOCK_PORT, module, RareBreeds_Orbits_Polygene::CLOCK_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_SYNC_PORT, module, RareBreeds_Orbits_Polygene::SYNC_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_LENGTH_CV_PORT, module, RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_HITS_CV_PORT, module, RareBreeds_Orbits_Polygene::HITS_CV_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_SHIFT_CV_PORT, module, RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_ODDITY_CV_PORT, module, RareBreeds_Orbits_Polygene::ODDITY_CV_INPUT));	
        addInput(createSkinnedInput(POLYGENE_COMPONENT_REVERSE_CV_PORT, module, RareBreeds_Orbits_Polygene::REVERSE_CV_INPUT));
        addInput(createSkinnedInput(POLYGENE_COMPONENT_INVERT_CV_PORT, module, RareBreeds_Orbits_Polygene::INVERT_CV_INPUT));

        addOutput(createSkinnedOutput(POLYGENE_COMPONENT_BEAT_PORT, module, RareBreeds_Orbits_Polygene::BEAT_OUTPUT));
        // clang-format on

        PolygeneRhythmDisplay *r =
                createWidget<PolygeneRhythmDisplay>(polygene_config.getPos(POLYGENE_COMPONENT_DISPLAY));
        r->module = module;
        r->box.size = mm2px(Vec(32.0, 32.0));
        addChild(r);
}

void RareBreeds_Orbits_PolygeneWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);
        MenuLabel *theme_label = new MenuLabel;
        theme_label->text = "Theme";
        menu->addChild(theme_label);

        for(size_t i = 0; i < polygene_config.m_themes.size(); ++i)
        {
                menu->addChild(new PolygeneThemeChoiceItem(this, i, polygene_config.getThemeName(i).c_str()));
        }
}

void RareBreeds_Orbits_PolygeneWidget::loadTheme(const char *theme)
{
        for(size_t i = 0; i < polygene_config.m_themes.size(); ++i)
        {
                if(polygene_config.getThemeName(i) == theme)
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
                PolygeneSkinned *skinned = dynamic_cast<PolygeneSkinned *>(child);
                if(skinned)
                {
                        skinned->loadTheme(theme);
                }
        }

        setPanel(APP->window->loadSvg(polygene_config.getSvg(POLYGENE_COMPONENT_PANEL, theme)));
}

json_t *RareBreeds_Orbits_PolygeneWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_t *theme = json_string(polygene_config.getThemeName(m_theme).c_str());
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
