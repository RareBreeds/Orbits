#include "OrbitsWidget.hpp"
#include "OrbitsSkinned.hpp"

OrbitsThemeChoiceItem::OrbitsThemeChoiceItem(OrbitsWidget *widget, int id, const char *name)
{
        m_widget = widget;
        m_id = id;
        text = name;
        rightText = CHECKMARK(widget->m_theme == id);
}

void OrbitsThemeChoiceItem::onAction(const event::Action &e)
{
        m_widget->loadTheme(m_id);
}

OrbitsWidget::OrbitsWidget(OrbitsConfig *config)
{
        m_config = config;
}

void OrbitsWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);
        MenuLabel *theme_label = new MenuLabel;
        theme_label->text = "Theme";
        menu->addChild(theme_label);

        for(size_t i = 0; i < m_config->numThemes(); ++i)
        {
                menu->addChild(new OrbitsThemeChoiceItem(this, i, m_config->getThemeName(i).c_str()));
        }

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
        menu->addChild(new MenuSeparator);
        MenuLabel *eoc_label = new MenuLabel;
        eoc_label->text = "EOC Mode";
        menu->addChild(eoc_label);
        std::vector<std::string> options = m_module->getOptions();
        for(size_t i = 0; i < options.size(); ++i)
        {
                menu->addChild(new EOCModeItem(m_module, i, options[i].c_str()));
        }
}

void BeatWidget::appendContextMenu(Menu *menu)
{
        menu->addChild(new MenuSeparator);
        MenuLabel *beat_label = new MenuLabel;
        beat_label->text = "Beat Mode";
        menu->addChild(beat_label);
        std::vector<std::string> options = m_module->getOptions();
        for(size_t i = 0; i < options.size(); ++i)
        {
                menu->addChild(new BeatModeItem(m_module, i, options[i].c_str()));
        }
}
