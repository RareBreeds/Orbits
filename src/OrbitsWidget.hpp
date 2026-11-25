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

// One option in the Randomize Filter menu: its name, and its bit in the module's randomization mask.
struct OrbitsRandomizeFilterOption
{
        const char *name;
        unsigned int bit;
};

// Adds the "Randomize Filter" submenu, with a check item for each option that toggles its bit in
// `*mask`. Bits that are set are randomized.
void orbitsAppendRandomizeFilterMenu(Menu *menu, unsigned int *mask,
                                     const std::vector<OrbitsRandomizeFilterOption> &options);

struct BeatWidget
{
        BeatMode *m_module;
        void appendContextMenu(Menu *menu);
};
