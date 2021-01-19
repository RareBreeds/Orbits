#include "EugeneWidget.hpp"
#include "EugeneModule.hpp"
#include "EugeneConfig.hpp"

// TODO: naming Eugene <Knob> <Length> <cv> ?
struct EugeneLengthKnob : RoundKnob
{
    EugeneLengthKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].length_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].length_knob));
        fb->dirty = true;
    }
};

struct EugeneHitsKnob : RoundKnob
{
    EugeneHitsKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].hits_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].hits_knob));
        fb->dirty = true;
    }
};

struct EugeneShiftKnob : RoundKnob
{
    EugeneShiftKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].shift_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].shift_knob));
        fb->dirty = true;
    }
};

struct EugeneLengthCvKnob : RoundKnob
{
    EugeneLengthCvKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].length_cv_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].length_cv_knob));
        fb->dirty = true;
    }
};

struct EugeneHitsCvKnob : RoundKnob
{
    EugeneHitsCvKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].hits_cv_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].hits_cv_knob));
        fb->dirty = true;
    }
};

struct EugeneShiftCvKnob : RoundKnob
{
    EugeneShiftCvKnob()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].shift_cv_knob));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].shift_cv_knob));
        fb->dirty = true;
    }
};

struct EugeneTopLeftScrew : app::SvgScrew
{
    EugeneTopLeftScrew()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].screw_top_left));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].screw_top_left));
        fb->dirty = true;
    }
};

struct EugeneTopRightScrew : app::SvgScrew
{
    EugeneTopRightScrew()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].screw_top_right));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].screw_top_right));
        fb->dirty = true;
    }
};

struct EugeneBottomLeftScrew : app::SvgScrew
{
    EugeneBottomLeftScrew()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].screw_bottom_left));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].screw_bottom_left));
        fb->dirty = true;
    }
};

struct EugeneBottomRightScrew : app::SvgScrew
{
    EugeneBottomRightScrew()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].screw_bottom_right));
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].screw_bottom_right));
        fb->dirty = true;
    }
};

struct EugeneReverseSwitch : app::SvgSwitch
{
    EugeneReverseSwitch()
    {
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].reverse_switch_off));
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].reverse_switch_on));
        shadow->opacity = 0.0;
    }

    void loadTheme(int theme)
    {
        frames[0] = APP->window->loadSvg(eugene_config.m_themes[theme].reverse_switch_off);
        frames[1] = APP->window->loadSvg(eugene_config.m_themes[theme].reverse_switch_on);

        event::Change change;
        onChange(change);
        onChange(change);
    }
};

struct EugeneInvertSwitch : app::SvgSwitch
{
    EugeneInvertSwitch()
    {
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].invert_switch_off));
        addFrame(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].invert_switch_on));
        shadow->opacity = 0.0;
    }

    void loadTheme(int theme)
    {
        frames[0] = APP->window->loadSvg(eugene_config.m_themes[theme].invert_switch_off);
        frames[1] = APP->window->loadSvg(eugene_config.m_themes[theme].invert_switch_on);

        event::Change change;
        onChange(change);
        onChange(change);
    }
};

struct EugeneBeatPort : app::SvgPort
{
    EugeneBeatPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].beat_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].beat_port));
        fb->dirty = true;
    }
};

struct EugeneSyncPort : app::SvgPort
{
    EugeneSyncPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].sync_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].sync_port));
        fb->dirty = true;
    }
};

struct EugeneClockPort : app::SvgPort
{
    EugeneClockPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].clock_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].clock_port));
        fb->dirty = true;
    }
};

struct EugeneLengthCvPort : app::SvgPort
{
    EugeneLengthCvPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].length_cv_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].length_cv_port));
        fb->dirty = true;
    }
};

struct EugeneShiftCvPort : app::SvgPort
{
    EugeneShiftCvPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].shift_cv_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].shift_cv_port));
        fb->dirty = true;
    }
};

struct EugeneHitsCvPort : app::SvgPort
{
    EugeneHitsCvPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].hits_cv_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].hits_cv_port));
        fb->dirty = true;
    }
};

struct EugeneInvertCvPort : app::SvgPort
{
    EugeneInvertCvPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].invert_cv_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].invert_cv_port));
        fb->dirty = true;
    }
};

struct EugeneReverseCvPort : app::SvgPort
{
    EugeneReverseCvPort()
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].reverse_cv_port));
        shadow->opacity = 0.07;
    }

    void loadTheme(int theme)
    {
        setSvg(APP->window->loadSvg(eugene_config.m_themes[theme].reverse_cv_port));
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

RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module)
{
    setModule(module);

    // Module may be NULL if this is the module selection screen
    if(module)
    {
        module->widget = this;
    }

    setPanel(APP->window->loadSvg(eugene_config.m_themes[eugene_config.m_default].panel));

    addChild(createWidgetCentered<EugeneTopLeftScrew>(Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneTopRightScrew>(Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneBottomLeftScrew>(Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));
    addChild(createWidgetCentered<EugeneBottomRightScrew>(Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2)));

    addParam(createParamCentered<EugeneLengthKnob>(eugene_config.length_knob, module, RareBreeds_Orbits_Eugene::LENGTH_KNOB_PARAM));
    addParam(createParamCentered<EugeneHitsKnob>(eugene_config.hits_knob, module, RareBreeds_Orbits_Eugene::HITS_KNOB_PARAM));
    addParam(createParamCentered<EugeneShiftKnob>(eugene_config.shift_knob, module, RareBreeds_Orbits_Eugene::SHIFT_KNOB_PARAM));
    addParam(createParamCentered<EugeneLengthCvKnob>(eugene_config.length_cv_knob, module, RareBreeds_Orbits_Eugene::LENGTH_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneHitsCvKnob>(eugene_config.hits_cv_knob, module, RareBreeds_Orbits_Eugene::HITS_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneShiftCvKnob>(eugene_config.shift_cv_knob, module, RareBreeds_Orbits_Eugene::SHIFT_CV_KNOB_PARAM));
    addParam(createParamCentered<EugeneReverseSwitch>(eugene_config.reverse_knob, module, RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM));
    addParam(createParamCentered<EugeneInvertSwitch>(eugene_config.invert_knob, module, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM));

    addInput(createInputCentered<EugeneClockPort>(eugene_config.clock, module, RareBreeds_Orbits_Eugene::CLOCK_INPUT));
    addInput(createInputCentered<EugeneSyncPort>(eugene_config.sync, module, RareBreeds_Orbits_Eugene::SYNC_INPUT));
    addInput(createInputCentered<EugeneLengthCvPort>(eugene_config.length_cv, module, RareBreeds_Orbits_Eugene::LENGTH_CV_INPUT));
    addInput(createInputCentered<EugeneHitsCvPort>(eugene_config.hits_cv, module, RareBreeds_Orbits_Eugene::HITS_CV_INPUT));
    addInput(createInputCentered<EugeneShiftCvPort>(eugene_config.shift_cv, module, RareBreeds_Orbits_Eugene::SHIFT_CV_INPUT));
    addInput(createInputCentered<EugeneReverseCvPort>(eugene_config.reverse_cv, module, RareBreeds_Orbits_Eugene::REVERSE_CV_INPUT));
    addInput(createInputCentered<EugeneInvertCvPort>(eugene_config.invert_cv, module, RareBreeds_Orbits_Eugene::INVERT_CV_INPUT));

    addOutput(createOutputCentered<EugeneBeatPort>(eugene_config.beat, module, RareBreeds_Orbits_Eugene::BEAT_OUTPUT));

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
        EugeneReverseSwitch *reverse_switch = dynamic_cast<EugeneReverseSwitch *>(child);
        if(reverse_switch)
        {
            reverse_switch->loadTheme(theme);
            continue;
        }

        EugeneInvertSwitch *invert_switch = dynamic_cast<EugeneInvertSwitch *>(child);
        if(invert_switch)
        {
            invert_switch->loadTheme(theme);
            continue;
        }

        EugeneLengthKnob *length_knob = dynamic_cast<EugeneLengthKnob *>(child);
        if(length_knob)
        {
            length_knob->loadTheme(theme);
            continue;
        }

        EugeneLengthCvKnob *length_cv_knob = dynamic_cast<EugeneLengthCvKnob *>(child);
        if(length_cv_knob)
        {
            length_cv_knob->loadTheme(theme);
            continue;
        }

        EugeneLengthCvPort *length_cv_port = dynamic_cast<EugeneLengthCvPort *>(child);
        if(length_cv_port)
        {
            length_cv_port->loadTheme(theme);
            continue;
        }

        EugeneHitsKnob *hits_knob = dynamic_cast<EugeneHitsKnob *>(child);
        if(hits_knob)
        {
            hits_knob->loadTheme(theme);
            continue;
        }

        EugeneHitsCvKnob *hits_cv_knob = dynamic_cast<EugeneHitsCvKnob *>(child);
        if(hits_cv_knob)
        {
            hits_cv_knob->loadTheme(theme);
            continue;
        }

        EugeneHitsCvPort *hits_cv_port = dynamic_cast<EugeneHitsCvPort *>(child);
        if(hits_cv_port)
        {
            hits_cv_port->loadTheme(theme);
            continue;
        }

        EugeneShiftKnob *shift_knob = dynamic_cast<EugeneShiftKnob *>(child);
        if(shift_knob)
        {
            shift_knob->loadTheme(theme);
            continue;
        }

        EugeneShiftCvKnob *shift_cv_knob = dynamic_cast<EugeneShiftCvKnob *>(child);
        if(shift_cv_knob)
        {
            shift_cv_knob->loadTheme(theme);
            continue;
        }

        EugeneShiftCvPort *shift_cv_port = dynamic_cast<EugeneShiftCvPort *>(child);
        if(shift_cv_port)
        {
            shift_cv_port->loadTheme(theme);
            continue;
        }

        EugeneBeatPort *beat_port = dynamic_cast<EugeneBeatPort *>(child);
        if(beat_port)
        {
            beat_port->loadTheme(theme);
            continue;
        }

        EugeneClockPort *clock_port = dynamic_cast<EugeneClockPort *>(child);
        if(clock_port)
        {
            clock_port->loadTheme(theme);
            continue;
        }

        EugeneSyncPort *sync_port = dynamic_cast<EugeneSyncPort *>(child);
        if(sync_port)
        {
            sync_port->loadTheme(theme);
            continue;
        }

        EugeneTopLeftScrew *screw_top_left = dynamic_cast<EugeneTopLeftScrew *>(child);
        if(screw_top_left)
        {
            screw_top_left->loadTheme(theme);
            continue;
        }

        EugeneTopRightScrew *screw_top_right = dynamic_cast<EugeneTopRightScrew *>(child);
        if(screw_top_right)
        {
            screw_top_right->loadTheme(theme);
            continue;
        }

        EugeneBottomLeftScrew *screw_bottom_left = dynamic_cast<EugeneBottomLeftScrew *>(child);
        if(screw_bottom_left)
        {
            screw_bottom_left->loadTheme(theme);
            continue;
        }

        EugeneBottomRightScrew *screw_bottom_right = dynamic_cast<EugeneBottomRightScrew *>(child);
        if(screw_bottom_right)
        {
            screw_bottom_right->loadTheme(theme);
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

