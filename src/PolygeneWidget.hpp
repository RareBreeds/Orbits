#pragma once

#include "OrbitsWidget.hpp"

struct RareBreeds_Orbits_Polygene;

struct RareBreeds_Orbits_PolygeneWidget : OrbitsWidget
{
        EOCWidget eoc_widget;
        RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *module);
        void appendModuleContextMenu(Menu *menu) override;
};
