#pragma once

#include "OrbitsModule.hpp"
#include "OrbitsSkinned.hpp"
#include "OrbitsWidgetData.hpp"

struct OrbitsWidget : ModuleWidget
{
        OrbitsTheme m_theme = kOrbitsThemeDefault;
        OrbitsSkinnedPanelConfig m_config;
        OrbitsWidgetData *m_widget_data = NULL;

        OrbitsWidget(const OrbitsSkinnedPanelConfig &config);
        ~OrbitsWidget();
        // Connects the widget to its module's saved widget data.
        // Call from the constructor when there's a module - which there isn't
        // in the module browser.
        void attachWidgetData(OrbitsWidgetData *data);
        void draw(const DrawArgs &args) override;
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
