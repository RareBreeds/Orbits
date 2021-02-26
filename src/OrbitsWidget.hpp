#pragma once

#include "OrbitsConfig.hpp"
#include "OrbitsModule.hpp"

struct OrbitsWidget : ModuleWidget
{
        int m_theme;
        OrbitsConfig *m_config;

        OrbitsWidget(OrbitsConfig *config);
        void appendContextMenu(Menu *menu) override;
        virtual void appendModuleContextMenu(Menu *menu) = 0;
        void loadTheme(const char *theme);
        void loadTheme(int theme);
        json_t *dataToJson();
        void dataFromJson(json_t *root);
};

struct OrbitsThemeChoiceItem : MenuItem
{
        OrbitsWidget *m_widget;
        int m_id;

        OrbitsThemeChoiceItem(OrbitsWidget *widget, int id, const char *name);
        void onAction(const event::Action &e) override;
};

struct EOCModeItem : MenuItem
{
        EOCMode *m_module;
        int m_id;

        EOCModeItem(EOCMode *module, int id, const char *name)
        {
                m_module = module;
                m_id = id;
                text = name;
                rightText = CHECKMARK(module->getMode() == id);
        }

        void onAction(const event::Action &e) override
        {
                m_module->setMode(m_id);
        }
};

struct EOCWidget
{
    EOCMode *m_module;
    void appendContextMenu(Menu *menu);
};
