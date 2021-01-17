#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// TODO: Better place / name for plugin config / theme code?
struct Theme
{
	std::string name;
	std::string panel;
	std::string knob_large;
	std::string knob_small;
	std::string switch_on;
	std::string switch_off;
	std::string screw;
	std::string port;
	void fromJson(json_t *root);
};

struct Config
{
	std::vector<Theme> m_themes;
	size_t m_default;
	void fromJson(std::string path);
};

extern Config config;

// Declare each Model, defined in each module source file
extern Model *modelRareBreeds_Orbits_Eugene;
extern Model *modelRareBreeds_Orbits_Polygene;
