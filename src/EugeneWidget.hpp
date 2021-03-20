#pragma once

#include "OrbitsWidget.hpp"

struct RareBreeds_Orbits_Eugene;
struct EugeneRhythmDisplay;

struct RareBreeds_Orbits_EugeneWidget : OrbitsWidget
{
        BeatWidget beat_widget;
        EOCWidget eoc_widget;
        EugeneRhythmDisplay *rhythm_display;
        RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module);
        void appendModuleContextMenu(Menu *menu) override;
};
