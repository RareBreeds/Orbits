#pragma once

#include "OrbitsConfig.hpp"
#include "plugin.hpp"

struct OrbitsWidget : ModuleWidget
{
        int m_theme;
        OrbitsConfig *m_config;

        OrbitsWidget(OrbitsConfig *config);
        void appendContextMenu(Menu *menu) override;
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
