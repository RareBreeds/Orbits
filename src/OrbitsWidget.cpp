#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"
#include "OrbitsUndoQueue.hpp"

OrbitsWidget::OrbitsWidget(const OrbitsSkinnedPanelConfig &config) : m_config(config)
{
        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[orbitsThemeIndex(m_theme)]);
        setPanel(APP->window->loadSvg(panelPath));
}

OrbitsWidget::~OrbitsWidget()
{
        if(m_widget_data)
        {
                m_widget_data->detach();
        }
}

void OrbitsWidget::attachWidgetData(OrbitsWidgetData *data)
{
        m_widget_data = data;
        m_widget_data->attach(this);
}

void OrbitsWidget::draw(const DrawArgs &args)
{
        if(m_widget_data)
        {
                json_t *loaded = m_widget_data->takeLoaded();
                if(loaded)
                {
                        dataFromJson(loaded);
                        json_decref(loaded);
                }
        }

        ModuleWidget::draw(args);
}

void OrbitsWidget::step()
{
        // Modules that make undoable changes on the engine thread queue them,
        // they're are added to the undo history here on the UI thread.
        OrbitsUndoSource *undo_source = dynamic_cast<OrbitsUndoSource *>(module);
        if(undo_source)
        {
                undo_source->flushUndoHistory();
        }

#if ORBITS_THEME_SELECTION_ENABLED
        // Rack has no event for its dark panels setting changing, so check it
        // every frame.
        const OrbitsTheme shown = m_theme_setting.shownTheme();
        if(shown != m_theme)
        {
                switchTheme(shown);
        }
#endif

        ModuleWidget::step();
}

#if ORBITS_ROOM_LIGHTING_ENABLED
static const float kOrbitsDefaultLightBoost = 0.3f;
static float g_light_boost = kOrbitsDefaultLightBoost;

float orbitsLightBrightness()
{
        return std::min(settings::rackBrightness + g_light_boost, 1.f);
}

float orbitsLightBoost()
{
        return g_light_boost;
}

void orbitsSetLightBoost(float boost)
{
        g_light_boost = clamp(boost, 0.f, 1.f);
}

// Rack calls these to save and load plugin-wide settings.
extern "C" json_t *settingsToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "light_boost", json_real(g_light_boost));
        }
        return root;
}

extern "C" void settingsFromJson(json_t *root)
{
        json_t *boost = json_object_get(root, "light_boost");
        if(json_is_number(boost))
        {
                const double value = json_number_value(boost);
                // Rejects NaN as well as values out of range, keeping the default.
                if(value >= 0.0 && value <= 1.0)
                {
                        g_light_boost = static_cast<float>(value);
                }
        }
}

struct OrbitsLightBoostQuantity : Quantity
{
        void setValue(float value) override
        {
                orbitsSetLightBoost(value);
        }
        float getValue() override
        {
                return orbitsLightBoost();
        }
        float getMinValue() override
        {
                return 0.f;
        }
        float getMaxValue() override
        {
                return 1.f;
        }
        float getDefaultValue() override
        {
                return kOrbitsDefaultLightBoost;
        }
        float getDisplayValue() override
        {
                return getValue() * 100.f;
        }
        void setDisplayValue(float displayValue) override
        {
                setValue(displayValue / 100.f);
        }
        std::string getLabel() override
        {
                return "Light boost";
        }
        std::string getUnit() override
        {
                return "%";
        }
        int getDisplayPrecision() override
        {
                return 3;
        }
};

struct OrbitsLightBoostSlider : ui::Slider
{
        OrbitsLightBoostSlider()
        {
                quantity = new OrbitsLightBoostQuantity;
        }
        ~OrbitsLightBoostSlider()
        {
                delete quantity;
        }
};
#endif

float orbitsBypassAlpha(Widget *widget)
{
#if ORBITS_BYPASS_FADE_ENABLED
        ModuleWidget *module_widget = widget->getAncestorOfType<ModuleWidget>();
        if(module_widget && module_widget->module && module_widget->module->isBypassed())
        {
                // ModuleWidget::draw()'s alpha for a bypassed module
                return 0.33f;
        }
#else
        UNUSED(widget);
#endif
        return 1.f;
}

NVGcolor orbitsLightTint()
{
#if ORBITS_ROOM_LIGHTING_ENABLED
        const float brightness = orbitsLightBrightness();
        return nvgRGBAf(brightness, brightness, brightness, 1.f);
#else
        return color::WHITE;
#endif
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        // Rack's section above is about this module too, so its own settings follow it without a heading
        menu->addChild(new MenuSeparator);
        appendModuleContextMenu(menu);

#if ORBITS_THEME_SELECTION_ENABLED
        // After the settings that change what the module does, as it only changes how it looks
        menu->addChild(new MenuSeparator);

        // "Follow Rack", then the themes in order
        std::vector<std::string> theme_names = {"Follow Rack"};
        theme_names.insert(theme_names.end(), kOrbitsThemeNames.begin(), kOrbitsThemeNames.end());
        menu->addChild(createIndexSubmenuItem(
                "Theme", theme_names,
                [this]()
                {
                        const OrbitsThemeSetting &setting = this->m_theme_setting;
                        return setting.m_follow_rack ? 0 : static_cast<size_t>(orbitsThemeIndex(setting.m_theme)) + 1;
                },
                [this](size_t index)
                {
                        OrbitsThemeSetting &setting = this->m_theme_setting;
                        setting.m_follow_rack = index == 0;
                        if(index > 0)
                        {
                                setting.m_theme = static_cast<OrbitsTheme>(index - 1);
                        }
                        switchTheme(setting.shownTheme());
                }));
#endif

#if ORBITS_ROOM_LIGHTING_ENABLED
        // Last, as it's shared by every Eugene and Polygene module and saved with Rack's settings
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuLabel("Plugin"));
        OrbitsLightBoostSlider *boost = new OrbitsLightBoostSlider;
        boost->box.size.x = 220.f;
        menu->addChild(boost);
#endif
}

OrbitsTheme OrbitsThemeSetting::shownTheme() const
{
        if(m_follow_rack)
        {
#if ORBITS_THEME_SELECTION_ENABLED
                return settings::preferDarkPanels ? OrbitsTheme::Dark : OrbitsTheme::Light;
#else
                // Only the default theme is shown
                return kOrbitsThemeDefault;
#endif
        }
        return m_theme;
}

void OrbitsThemeSetting::toJson(json_t *root) const
{
        // The theme shown, which is what versions before Follow Rack load
        json_t *theme = json_string(orbitsThemeName(shownTheme()));
        if(theme)
        {
                json_object_set_new(root, "theme", theme);
        }
        json_object_set_new(root, "follow_rack", json_boolean(m_follow_rack));
}

void OrbitsThemeSetting::fromJson(json_t *root)
{
        OrbitsTheme theme;
        if(orbitsThemeFromName(json_string_value(json_object_get(root, "theme")), theme))
        {
                m_theme = theme;
                m_follow_rack = false;
        }

        json_t *follow_rack = json_object_get(root, "follow_rack");
        if(json_is_boolean(follow_rack))
        {
                m_follow_rack = json_is_true(follow_rack);
        }
}

void OrbitsWidget::loadInitialTheme()
{
#if ORBITS_THEME_SELECTION_ENABLED
        switchTheme(m_theme_setting.shownTheme());
#else
        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[orbitsThemeIndex(m_theme)]);
        setPanel(APP->window->loadSvg(panelPath));
#endif
}

void OrbitsWidget::switchTheme(OrbitsTheme theme)
{
#if ORBITS_THEME_SELECTION_ENABLED
        m_theme = theme;

        for(auto child : children)
        {
                OrbitsSkinned *skinned = dynamic_cast<OrbitsSkinned *>(child);
                if(skinned)
                {
                        skinned->loadTheme(theme);
                }
        }

        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[orbitsThemeIndex(theme)]);
        setPanel(APP->window->loadSvg(panelPath));
#else
        UNUSED(theme);
#endif
}

json_t *OrbitsWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                m_theme_setting.toJson(root);
        }
        return root;
}

void OrbitsWidget::dataFromJson(json_t *root)
{
        if(root)
        {
                m_theme_setting.fromJson(root);
                switchTheme(m_theme_setting.shownTheme());
        }
}

void EOCWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(createIndexPtrSubmenuItem("EOC Mode", m_module->getOptions(), &m_module->m_mode));
}

void orbitsAppendRandomizeFilterMenu(Menu *menu, unsigned int *mask,
                                     const std::vector<OrbitsRandomizeFilterOption> &options)
{
        // The submenu is built when it's opened, so the options are copied into the lambda.
        menu->addChild(createSubmenuItem("Randomize Filter", "",
                                         [=](Menu *submenu)
                                         {
                                                 submenu->addChild(createMenuLabel("Parameters that are randomized"));
                                                 for(const OrbitsRandomizeFilterOption &option : options)
                                                 {
                                                         const unsigned int bit = 1u << option.bit;
                                                         submenu->addChild(createCheckMenuItem(
                                                                 option.name, "", [=]() { return (*mask & bit) != 0; },
                                                                 [=]() { *mask ^= bit; }));
                                                 }
                                         }));
}

void BeatWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(createIndexPtrSubmenuItem("Beat Mode", m_module->getOptions(), &m_module->m_mode));
}

OrbitsWidgetData::~OrbitsWidgetData()
{
        json_decref(m_loaded.exchange(nullptr));
}

void OrbitsWidgetData::save(json_t *root)
{
        if(m_widget)
        {
                json_t *data = m_widget->dataToJson();
                if(data)
                {
                        json_object_set_new(root, "widget", data);
                }
        }
}

void OrbitsWidgetData::load(json_t *root)
{
        json_t *data = json_object_get(root, "widget");
        if(data)
        {
                // Release data loaded earlier that the widget hasn't applied yet, e.g. when two
                // presets are loaded before the widget next draws.
                json_decref(m_loaded.exchange(json_incref(data)));
        }
}

void OrbitsWidgetData::attach(OrbitsWidget *widget)
{
        m_widget = widget;
}

void OrbitsWidgetData::detach()
{
        m_widget = NULL;
}

json_t *OrbitsWidgetData::takeLoaded()
{
        return m_loaded.exchange(nullptr);
}

OrbitsDisplay::OrbitsDisplay(Vec pos, Vec size)
{
        box.pos = pos;
        box.size = size;
#if ORBITS_FRAMEBUFFER_CACHING_ENABLED
        m_fb = new FramebufferWidget;
        m_fb->box.size = size;
        addChild(m_fb);
#endif
}

void OrbitsDisplay::setContent(Widget *content)
{
        m_content = content;
#if ORBITS_FRAMEBUFFER_CACHING_ENABLED
        m_fb->addChild(content);
        m_fb->setDirty();
#else
        addChild(content);
#endif
}

void OrbitsDisplay::setDirty()
{
#if ORBITS_FRAMEBUFFER_CACHING_ENABLED
        m_fb->setDirty();
#endif
}

void OrbitsDisplay::draw(const DrawArgs & /*args*/)
{
        // Everything is drawn in layer 1
}

void OrbitsDisplay::drawLayer(const DrawArgs &args, int layer)
{
        if(layer == 1 && m_content)
        {
                NVGcolor tint = orbitsLightTint();
                tint.a *= orbitsBypassAlpha(this);
                nvgGlobalTint(args.vg, tint);
#if ORBITS_FRAMEBUFFER_CACHING_ENABLED
                drawChild(m_fb, args);
#else
                drawChild(m_content, args);
#endif
        }
}
