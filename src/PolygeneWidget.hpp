#pragma once

#include "plugin.hpp"

struct RareBreeds_Orbits_Polygene;

struct RareBreeds_Orbits_PolygeneWidget : ModuleWidget
{
        int m_theme = 0;

        RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module);
        void appendContextMenu(Menu *menu) override;
        void loadTheme(const char *theme);
        void loadTheme(int theme);
        json_t *dataToJson();
        void dataFromJson(json_t *root);
};
