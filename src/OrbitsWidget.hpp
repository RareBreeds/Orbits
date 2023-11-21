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

struct EOCWidget
{
        EOCMode *m_module;
        void appendContextMenu(Menu *menu);
};

struct BeatWidget
{
        BeatMode *m_module;
        void appendContextMenu(Menu *menu);
};
