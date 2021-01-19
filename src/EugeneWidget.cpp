#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "EugeneConfig.hpp"

struct EugeneSkinnedKnob : RoundKnob
{
    EugeneTheme::EugeneThemeComponents m_component;

    EugeneSkinnedKnob(EugeneTheme::EugeneThemeComponents component)
    {
        m_component = component;
        loadTheme(eugene_config.m_default);
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].m_components[m_component]));
        fb->dirty = true;
    }
};

struct EugeneSkinnedScrew : app::SvgScrew
{
    EugeneTheme::EugeneThemeComponents m_component;

    EugeneSkinnedScrew(EugeneTheme::EugeneThemeComponents component)
    {
        m_component = component;
        loadTheme(eugene_config.m_default);
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].m_components[m_component]));
        fb->dirty = true;
    }
};

struct EugeneSkinnedSwitch : app::SvgSwitch
{
    EugeneTheme::EugeneThemeComponents m_component;

    EugeneSkinnedSwitch(EugeneTheme::EugeneThemeComponents component)
    {
        m_component = component;
        // Relies on the OFF value being one after ON
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].m_components[m_component + 1]));
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].m_components[m_component]));
        shadow->opacity = 0.0;
    }

    void loadTheme(int theme)
    {
        // Relies on the OFF value being one after ON
        frames[0] = APP->window->loadSvg(eugene_config.m_themes[theme].m_components[m_component + 1]);
        frames[1] = APP->window->loadSvg(eugene_config.m_themes[theme].m_components[m_component]);

        event::Change change;
        onChange(change);
        onChange(change);
    }
};

struct EugeneSkinnedPort : app::SvgPort
{
    EugeneTheme::EugeneThemeComponents m_component;

    EugeneSkinnedPort(EugeneTheme::EugeneThemeComponents component)
    {
        m_component = component;
        loadTheme(eugene_config.m_default);
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].m_components[m_component]));
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
    const auto inverted_display = false;
    const auto background_color = inverted_display ? color::WHITE : color::BLACK;
    const auto foreground_color = inverted_display ? color::BLACK : color::WHITE;

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
    //nvgFill(args.vg);

    // Scale to [-1, 1]
    nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

    // Flip x and y so we start at the top and positive angle increments go clockwise
    nvgScale(args.vg, -1.f, -1.f);

    // Triangle showing direction
    const bool reverse = module->readReverse();
    //nvgBeginPath(args.vg);
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

struct ThemeChoiceItem : MenuItem
{
    RareBreeds_Orbits_EugeneWidget *m_widget;
    int m_id;

    ThemeChoiceItem(RareBreeds_Orbits_EugeneWidget *widget, int id, const char *name)
    {
        m_widget = widget;
        m_id = id;
        text = name;
        rightText = CHECKMARK(widget->m_theme == id);
    }

    void onAction(const event::Action& e) override
    {
        m_widget->loadTheme(m_id);
    }
};

static EugeneSkinnedScrew* createSkinnedScrew(EugeneTheme::EugeneThemeComponents component, math::Vec pos)
{
	EugeneSkinnedScrew* o = new EugeneSkinnedScrew(component);
    o->box.pos = pos.minus(o->box.size.div(2));
	return o;
}

template <class TParamWidget>
static TParamWidget* createSkinnedParam(EugeneTheme::EugeneThemeComponents component, math::Vec pos, engine::Module* module, int paramId)
{
	TParamWidget* o = new TParamWidget(component);
    o->box.pos = pos.minus(o->box.size.div(2));
	if (module)
    {
		o->paramQuantity = module->paramQuantities[paramId];
	}
	return o;
}

static EugeneSkinnedPort* createSkinnedPort(EugeneTheme::EugeneThemeComponents component, math::Vec pos, engine::Module* module, int portId)
{
	EugeneSkinnedPort* o = new EugeneSkinnedPort(component);
	o->box.pos = pos.minus(o->box.size.div(2));
	o->module = module;
	o->portId = portId;
	return o;
}

static EugeneSkinnedPort* createSkinnedInput(EugeneTheme::EugeneThemeComponents component, math::Vec pos, engine::Module* module, int inputId)
{
	EugeneSkinnedPort* o = createSkinnedPort(component, pos, module, inputId);
	o->type = app::PortWidget::INPUT;
	return o;
}

static EugeneSkinnedPort* createSkinnedOutput(EugeneTheme::EugeneThemeComponents component, math::Vec pos, engine::Module* module, int outputId)
{
	EugeneSkinnedPort* o = createSkinnedPort(component, pos, module, outputId);
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

    setPanel(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].m_components[EugeneTheme::panel]));

    // TODO: Screw positions are based on the panel size, could have an svg path and position for all EugeneTheme::EugeneThemeCompenents
    addChild(createSkinnedScrew(EugeneTheme::screw_top_left, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createSkinnedScrew(EugeneTheme::screw_top_right, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createSkinnedScrew(EugeneTheme::screw_bottom_left, Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
    addChild(createSkinnedScrew(EugeneTheme::screw_bottom_right, Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::length_knob, eugene_config.length_knob, module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::hits_knob, eugene_config.hits_knob, module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::shift_knob, eugene_config.shift_knob, module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::length_cv_knob, eugene_config.length_cv_knob, module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::hits_cv_knob, eugene_config.hits_cv_knob, module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedKnob>(EugeneTheme::shift_cv_knob, eugene_config.shift_cv_knob, module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedSwitch>(EugeneTheme::reverse_switch_on, eugene_config.reverse_knob, module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
    addParam(createSkinnedParam<EugeneSkinnedSwitch>(EugeneTheme::invert_switch_on, eugene_config.invert_knob, module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

    addInput(createSkinnedInput(EugeneTheme::clock_port, eugene_config.clock, module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
    addInput(createSkinnedInput(EugeneTheme::sync_port, eugene_config.sync, module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
    addInput(createSkinnedInput(EugeneTheme::length_cv_port, eugene_config.length_cv, module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
    addInput(createSkinnedInput(EugeneTheme::hits_cv_port, eugene_config.hits_cv, module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
    addInput(createSkinnedInput(EugeneTheme::shift_cv_port, eugene_config.shift_cv, module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
    addInput(createSkinnedInput(EugeneTheme::reverse_cv_port, eugene_config.reverse_cv, module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
    addInput(createSkinnedInput(EugeneTheme::invert_cv_port, eugene_config.invert_cv, module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

    addOutput(createSkinnedOutput(EugeneTheme::beat_port, eugene_config.beat, module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

    EugeneRhythmDisplay *r = createWidget<EugeneRhythmDisplay>(eugene_config.display);
    r->module = module;
    r->box.size = mm2px(Vec(32.0, 32.0));
    addChild(r);
}

void RareBreeds_Orbits_EugeneWidget::appendContextMenu(Menu* menu)
{
    menu->addChild(new MenuSeparator);
    MenuLabel *theme_label = new MenuLabel;
    theme_label->text = "Theme";
    menu->addChild(theme_label);

    for(size_t i = 0; i < eugene_config.m_themes.size(); ++i)
    {
        menu->addChild(new ThemeChoiceItem(this, i, eugene_config.m_themes[i].m_name.c_str()));
    }
}

void RareBreeds_Orbits_EugeneWidget::loadTheme(const char *theme)
{
    for(size_t i = 0; i < eugene_config.m_themes.size(); ++i)
    {
        if(eugene_config.m_themes[i].m_name == theme)
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
        // TODO: Get rid of the dynamic_cast chain
        EugeneSkinnedSwitch *_switch = dynamic_cast<EugeneSkinnedSwitch *>(child);
        if(_switch)
        {
            _switch->loadTheme(theme);
            continue;
        }

        EugeneSkinnedKnob *knob = dynamic_cast<EugeneSkinnedKnob *>(child);
        if(knob)
        {
            knob->loadTheme(theme);
            continue;
        }

        EugeneSkinnedPort *port = dynamic_cast<EugeneSkinnedPort *>(child);
        if(port)
        {
            port->loadTheme(theme);
            continue;
        }

        EugeneSkinnedScrew *screw = dynamic_cast<EugeneSkinnedScrew *>(child);
        if(screw)
        {
            screw->loadTheme(theme);
            continue;
        }
    }

    setPanel(APP->window->loadSvg(eugene_config.m_themes[theme].m_components[EugeneTheme::panel]));
}

json_t *RareBreeds_Orbits_EugeneWidget::dataToJson()
{
    json_t *root = json_object();
    if(root)
    {
        json_t *theme = json_string(eugene_config.m_themes[m_theme].m_name.c_str());
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

