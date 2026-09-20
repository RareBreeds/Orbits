#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"

OrbitsWidget::OrbitsWidget(const OrbitsSkinnedPanelConfig& config) : m_config(config)
{
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);

#if ORBITS_THEME_SELECTION_ENABLED
        menu->addChild(createIndexSubmenuItem("Theme",
                std::vector<std::string>(kOrbitsThemeNames.begin(), kOrbitsThemeNames.end()),
                [this]() { return toIndex(this->m_theme); },
                [this](int mode) { switchTheme(static_cast<OrbitsTheme>(mode)); }
        ));
#endif

        appendModuleContextMenu(menu);
}

void OrbitsWidget::switchTheme(const char *theme)
{
#if ORBITS_THEME_SELECTION_ENABLED
        OrbitsTheme t;
        if (fromString(theme, t))
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

        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[toIndex(theme)]);
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
                json_t *theme = json_string(toString(m_theme));
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
        menu->addChild(createIndexPtrSubmenuItem("EOC Mode",
                m_module->getOptions(),
                &m_module->m_mode
        ));
}

void BeatWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(createIndexPtrSubmenuItem("Beat Mode",
                m_module->getOptions(),
                &m_module->m_mode
        ));
}
