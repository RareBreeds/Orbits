#pragma once

#include "OrbitsWidget.hpp"

struct RareBreeds_Orbits_Eugene;

struct RareBreeds_Orbits_EugeneWidget : OrbitsWidget
{
        EOCWidget eoc_widget;
        RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module);
        void appendModuleContextMenu(Menu *menu) override;
};
