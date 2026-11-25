#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"
#include "OrbitsUndoQueue.hpp"

OrbitsWidget::OrbitsWidget(const OrbitsSkinnedPanelConfig &config) : m_config(config)
{
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

        ModuleWidget::step();
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);

#if ORBITS_THEME_SELECTION_ENABLED
        menu->addChild(createIndexSubmenuItem(
                "Theme", std::vector<std::string>(kOrbitsThemeNames.begin(), kOrbitsThemeNames.end()),
                [this]() { return orbitsThemeIndex(this->m_theme); },
                [this](int mode) { switchTheme(static_cast<OrbitsTheme>(mode)); }));
#endif

        appendModuleContextMenu(menu);
}

void OrbitsWidget::switchTheme(const char *theme)
{
#if ORBITS_THEME_SELECTION_ENABLED
        OrbitsTheme t;
        if(orbitsThemeFromName(theme, t))
        {
                switchTheme(t);
        }
        else
        {
                switchTheme(kOrbitsThemeDefault);
        }
#else
        UNUSED(theme);
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
                json_t *theme = json_string(orbitsThemeName(m_theme));
                if(theme)
                {
                        json_object_set_new(root, "theme", theme);
                }
        }
        return root;
}

void OrbitsWidget::dataFromJson(json_t *root)
{
#if ORBITS_THEME_SELECTION_ENABLED
        if(root)
        {
                json_t *obj = json_object_get(root, "theme");
                if(obj)
                {
                        switchTheme(json_string_value(obj));
                }
        }
#else
        UNUSED(root);
#endif
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
