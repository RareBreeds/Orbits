#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "EugeneConfig.hpp"

struct EugeneKnobLarge : RoundKnob
{
    EugeneKnobLarge()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].knob_large));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].knob_large));
        fb->dirty = true;
    }
};

struct EugeneKnobSmall : RoundKnob
{
    EugeneKnobSmall()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].knob_small));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].knob_small));
        fb->dirty = true;
    }
};

struct EugeneScrew : app::SvgScrew
{
    EugeneScrew()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].screw));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].screw));
        fb->dirty = true;
    }
};

struct EugeneSwitch : app::SvgSwitch
{
    EugeneSwitch()
    {
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].switch_off));
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].switch_on));
        shadow->opacity = 0.0;
    }

    void loadTheme(int theme)
    {
        frames[0] = APP->window->loadSvg(eugene_config.m_themes[theme].switch_off);
        frames[1] = APP->window->loadSvg(eugene_config.m_themes[theme].switch_on);

        event::Change change;
        onChange(change);
        onChange(change);
    }
};

struct EugenePort : app::SvgPort
{
    EugenePort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].port));
        fb->dirty = true;
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

    // Scale to [-1, 1]
    nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

    // Flip x and y so we start at the top and positive angle increments go clockwise
    nvgScale(args.vg, -1.f, -1.f);

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

        if(module->index == k)
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

RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module)
{
    setModule(module);

    // Module may be NULL if this is the module selection screen
    if(module)
    {
        module->widget = this;
    }

    setPanel(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].panel));

    addChild(createWidgetCentered<EugeneScrew>(Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneScrew>(Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneScrew>(Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneScrew>(Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

    addParam(createParamCentered<EugeneKnobLarge>(eugene_config.length_knob, module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
    addParam(createParamCentered<EugeneKnobLarge>(eugene_config.hits_knob, module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
    addParam(createParamCentered<EugeneKnobLarge>(eugene_config.shift_knob, module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
    addParam(createParamCentered<EugeneKnobSmall>(eugene_config.length_cv_knob, module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneKnobSmall>(eugene_config.hits_cv_knob, module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneKnobSmall>(eugene_config.shift_cv_knob, module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneSwitch>(eugene_config.reverse_knob, module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
    addParam(createParamCentered<EugeneSwitch>(eugene_config.invert_knob, module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

    addInput(createInputCentered<EugenePort>(eugene_config.clock, module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.sync, module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.length_cv, module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.hits_cv, module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.shift_cv, module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.reverse_cv, module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
    addInput(createInputCentered<EugenePort>(eugene_config.invert_cv, module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

    addOutput(createOutputCentered<EugenePort>(eugene_config.beat, module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

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
        menu->addChild(new ThemeChoiceItem(this, i, eugene_config.m_themes[i].name.c_str()));
    }
}

void RareBreeds_Orbits_EugeneWidget::loadTheme(const char *theme)
{
    for(size_t i = 0; i < eugene_config.m_themes.size(); ++i)
    {
        if(eugene_config.m_themes[i].name == theme)
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
        EugeneSwitch *swi = dynamic_cast<EugeneSwitch *>(child);
        if(swi)
        {
            swi->loadTheme(theme);
            continue;
        }

        EugeneKnobLarge *knob_large = dynamic_cast<EugeneKnobLarge *>(child);
        if(knob_large)
        {
            knob_large->loadTheme(theme);
            continue;
        }

        EugeneKnobSmall *knob_small = dynamic_cast<EugeneKnobSmall *>(child);
        if(knob_small)
        {
            knob_small->loadTheme(theme);
            continue;
        }

        EugenePort *port = dynamic_cast<EugenePort *>(child);
        if(port)
        {
            port->loadTheme(theme);
            continue;
        }

        EugeneScrew *screw = dynamic_cast<EugeneScrew *>(child);
        if(screw)
        {
            screw->loadTheme(theme);
            continue;
        }
    }

    // TODO: Save and load theme in json
    for(auto param : params)
    {
        EugeneSwitch *swi = dynamic_cast<EugeneSwitch *>(param);
        if(swi)
        {
            swi->loadTheme(theme);
            continue;
        }

        EugeneKnobLarge *knob_large = dynamic_cast<EugeneKnobLarge *>(param);
        if(knob_large)
        {
            knob_large->loadTheme(theme);
            continue;
        }

        EugeneKnobSmall *knob_small = dynamic_cast<EugeneKnobSmall *>(param);
        if(knob_small)
        {
            knob_small->loadTheme(theme);
            continue;
        }
    }

    setPanel(APP->window->loadSvg(eugene_config.m_themes[theme].panel));
}

json_t *RareBreeds_Orbits_EugeneWidget::dataToJson()
{
    json_t *root = json_object();
    if(root)
    {
        json_t *theme = json_string(eugene_config.m_themes[m_theme].name.c_str());
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

