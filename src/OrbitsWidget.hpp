#pragma once

#include "OrbitsModule.hpp"
#include "OrbitsSkinned.hpp"

struct OrbitsWidget : ModuleWidget
{
        OrbitsTheme m_theme;
        OrbitsSkinnedPanelConfig m_config;

        OrbitsWidget(const OrbitsSkinnedPanelConfig& config);
        void appendContextMenu(Menu *menu) override;
        void step() override;
        virtual void appendModuleContextMenu(Menu *menu) = 0;
        void switchTheme(const char *theme);
        void switchTheme(OrbitsTheme theme);
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
