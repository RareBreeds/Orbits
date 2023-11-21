#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"

OrbitsWidget::OrbitsWidget(OrbitsConfig *config)
{
        m_config = config;
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);

        menu->addChild(createIndexSubmenuItem("Theme",
                m_config->getThemeNames(),
                [=]() {return m_theme;},
                [=](int mode) {loadTheme(mode);}
        ));

        appendModuleContextMenu(menu);
}

void OrbitsWidget::loadTheme(const char *theme)
{
        for(size_t i = 0; i < m_config->numThemes(); ++i)
        {
                if(m_config->getThemeName(i) == theme)
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

        setPanel(APP->window->loadSvg(m_config->getSvg("panel", theme)));
}

json_t *OrbitsWidget::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_t *theme = json_string(m_config->getThemeName(m_theme).c_str());
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
