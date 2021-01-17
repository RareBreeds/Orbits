#pragma once

#include "plugin.hpp"

struct RareBreeds_Orbits_Eugene;

struct RareBreeds_Orbits_EugeneWidget : ModuleWidget
{
	int m_theme = 0;

	RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *module);
	void appendContextMenu(Menu* menu) override;
	void loadTheme(const char *theme);
	void loadTheme(int theme);
	json_t *dataToJson();
	void dataFromJson(json_t *root);
};
