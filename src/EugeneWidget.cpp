#include "EugeneWidget.hpp"
#include "EugeneConfig.hpp"
#include "EugeneModule.hpp"

// Interface for components with the ability to change skins
struct EugeneSkinned
{
        virtual void loadTheme(int theme);
};

struct EugeneSkinnedKnob : RoundKnob, EugeneSkinned
{
        EugeneComponents m_component;

        EugeneSkinnedKnob(EugeneComponents component)
        {
                m_component = component;
                loadTheme(eugene_config.m_default);
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(eugene_config.getSvg(m_component, theme)));
                fb->dirty = true;
        }
};

struct EugeneSkinnedScrew : app::SvgScrew, EugeneSkinned
{
        EugeneComponents m_component;

        EugeneSkinnedScrew(EugeneComponents component)
        {
                m_component = component;
                loadTheme(eugene_config.m_default);
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(eugene_config.getSvg(m_component, theme)));
                fb->dirty = true;
        }
};

struct EugeneSkinnedSwitch : app::SvgSwitch, EugeneSkinned
{
        EugeneComponents m_component;

        EugeneSkinnedSwitch(EugeneComponents component)
        {
                m_component = component;
                // Relies on the OFF value being one after ON
                addFrame(APP->window->loadSvg(eugene_config.getSvg(EugeneComponents(m_component + 1))));
                addFrame(APP->window->loadSvg(eugene_config.getSvg(m_component)));
                shadow->opacity = 0.0;
        }

        void loadTheme(int theme) override
        {
                // Relies on the OFF value being one after ON
                frames[0] = APP->window->loadSvg(eugene_config.getSvg(EugeneComponents(m_component + 1), theme));
                frames[1] = APP->window->loadSvg(eugene_config.getSvg(m_component, theme));

                event::Change change;
                onChange(change);
                onChange(change);
        }
};

struct EugeneSkinnedPort : app::SvgPort, EugeneSkinned
{
        EugeneComponents m_component;

        EugeneSkinnedPort(EugeneComponents component)
        {
                m_component = component;
                loadTheme(eugene_config.m_default);
                shadow->opacity = 0.07;
        }

        void loadTheme(int theme) override
        {
                setSvg(APP->window->loadSvg(eugene_config.getSvg(m_component, theme)));
                // fb->dirty = true; // Already set by setSvg for SvgPorts
        }
};

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

struct EugeneThemeChoiceItem : MenuItem
{
        RareBreeds_Orbits_EugeneWidget *m_widget;
        int m_id;

        EugeneThemeChoiceItem(RareBreeds_Orbits_EugeneWidget *widget, int id, const char *name)
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
static EugeneSkinnedScrew *createSkinnedScrew(EugeneComponents component, math::Vec pos)
{
        EugeneSkinnedScrew *o = new EugeneSkinnedScrew(component);
        o->box.pos = pos.minus(o->box.size.div(2));
        return o;
}

template <class TParamWidget>
static TParamWidget *createSkinnedParam(EugeneComponents component, engine::Module *module, int paramId)
{
        TParamWidget *o = new TParamWidget(component);
        o->box.pos = eugene_config.getPos(component).minus(o->box.size.div(2));
        if(module)
        {
                o->paramQuantity = module->paramQuantities[paramId];
        }
        return o;
}

static EugeneSkinnedPort *createSkinnedPort(EugeneComponents component, engine::Module *module, int portId)
{
        EugeneSkinnedPort *o = new EugeneSkinnedPort(component);
        o->box.pos = eugene_config.getPos(component).minus(o->box.size.div(2));
        o->module = module;
        o->portId = portId;
        return o;
}

static EugeneSkinnedPort *createSkinnedInput(EugeneComponents component, engine::Module *module, int inputId)
{
        EugeneSkinnedPort *o = createSkinnedPort(component, module, inputId);
        o->type = app::PortWidget::INPUT;
        return o;
}

static EugeneSkinnedPort *createSkinnedOutput(EugeneComponents component, engine::Module *module, int outputId)
{
        EugeneSkinnedPort *o = createSkinnedPort(component, module, outputId);
        o->type = app::PortWidget::OUTPUT;
        return o;
}

RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module)
{
        setModule(module);

        // Module may be NULL if this is the module selection screen
        if(module)
        {
                module->widget = this;
        }

        // clang-format off
        setPanel(APP->window->loadSvg(eugene_config.getSvg(EUGENE_COMPONENT_PANEL)));

        // TODO: Screw positions are based on the panel size, could have a position for them in eugene_config based on panel size 
        addChild(createSkinnedScrew(EUGENE_COMPONENT_SCREW_TOP_LEFT, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(EUGENE_COMPONENT_SCREW_TOP_RIGHT, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(EUGENE_COMPONENT_SCREW_BOTTOM_LEFT, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
        addChild(createSkinnedScrew(EUGENE_COMPONENT_SCREW_BOTTOM_RIGHT, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_LENGTH_KNOB, module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_HITS_KNOB, module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_SHIFT_KNOB, module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_LENGTH_CV_KNOB, module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_HITS_CV_KNOB, module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedKnob>(EUGENE_COMPONENT_SHIFT_CV_KNOB, module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedSwitch>(EUGENE_COMPONENT_REVERSE_SWITCH_ON, module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
        addParam(createSkinnedParam<EugeneSkinnedSwitch>(EUGENE_COMPONENT_INVERT_SWITCH_ON, module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

        addInput(createSkinnedInput(EUGENE_COMPONENT_CLOCK_PORT, module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_SYNC_PORT, module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_LENGTH_CV_PORT, module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_HITS_CV_PORT, module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_SHIFT_CV_PORT, module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_REVERSE_CV_PORT, module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
        addInput(createSkinnedInput(EUGENE_COMPONENT_INVERT_CV_PORT, module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

        addOutput(createSkinnedOutput(EUGENE_COMPONENT_BEAT_PORT, module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));
        // clang-format on

        EugeneRhythmDisplay *r = createWidget<EugeneRhythmDisplay>(eugene_config.getPos(EUGENE_COMPONENT_DISPLAY));
        r->module = module;
        r->box.size = mm2px(Vec(32.0, 32.0));
        addChild(r);
}

void RareBreeds_Orbits_EugeneWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);
        MenuLabel *theme_label = new MenuLabel;
        theme_label->text = "Theme";
        menu->addChild(theme_label);

        for(size_t i = 0; i < eugene_config.m_themes.size(); ++i)
        {
                menu->addChild(new EugeneThemeChoiceItem(this, i, eugene_config.getThemeName(i).c_str()));
        }
}

void RareBreeds_Orbits_EugeneWidget::loadTheme(const char *theme)
{
        for(size_t i = 0; i < eugene_config.m_themes.size(); ++i)
        {
                if(eugene_config.getThemeName(i) == theme)
                {
                        loadTheme(i);
                        break;
                }
        }
}

void RareBreeds_Orbits_EugeneWidget::loadTheme(int theme)
{
        m_theme = theme;

        for(auto child : children)
        {
                EugeneSkinned *skinned = dynamic_cast<EugeneSkinned *>(child);
                if(skinned)
                {
                        skinned->loadTheme(theme);
                }
        }

        setPanel(APP->window->loadSvg(eugene_config.getSvg(EUGENE_COMPONENT_PANEL, theme)));
}

json_t *RareBreeds_Orbits_EugeneWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_t *theme = json_string(eugene_config.getThemeName(m_theme).c_str());
                if(theme)
                {
                        json_object_set_new(root, "theme", theme);
                }
        }
        return root;
}

void RareBreeds_Orbits_EugeneWidget::dataFromJson(json_t *root)
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
