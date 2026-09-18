#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"

// TODO: MOOOOO: OrbitsWidgetConfig? we'll need the theme configuration here, panels
OrbitsWidget::OrbitsWidget(const OrbitsSkinnedPanelConfig& config) : m_config(config)
{
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);

        menu->addChild(createIndexSubmenuItem("Theme",
                ORBITS_THEME_NAMES,
                [this]() { return this->m_theme; },
                [this](int mode) { loadTheme(mode); }
        ));

        appendModuleContextMenu(menu);
}

// TODO: MOOOOOO
void OrbitsWidget::loadTheme(const char *theme)
{
        const char *themes[ORBITS_THEME_COUNT] = ORBITS_THEME_NAMES;
        for(int i = 0; i < ORBITS_THEME_COUNT; ++i)
        {
                if(strcmp(themes[i], theme) == 0)
                {
                        loadTheme(i);
                        break;
                }
        }
}

void OrbitsWidget::loadTheme(int theme)
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

        std::string panelPath = asset::plugin(pluginInstance, m_config.panelPaths[theme]);
        setPanel(APP->window->loadSvg(panelPath));
}

json_t *OrbitsWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                const char *themes[ORBITS_THEME_COUNT] = ORBITS_THEME_NAMES;
                json_t *theme = json_string(themes[m_theme]);
                if(theme)
                {
                        json_object_set_new(root, "theme", theme);
                }
        }
        return root;
}

void OrbitsWidget::dataFromJson(json_t *root)
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
