#pragma once

#include "plugin.hpp"

enum EugeneComponents
{
    EUGENE_COMPONENT_PANEL,

    EUGENE_COMPONENT_SCREW_TOP_LEFT,
    EUGENE_COMPONENT_SCREW_TOP_RIGHT,
    EUGENE_COMPONENT_SCREW_BOTTOM_LEFT,
    EUGENE_COMPONENT_SCREW_BOTTOM_RIGHT,

    EUGENE_COMPONENT_CLOCK_PORT,
    EUGENE_COMPONENT_SYNC_PORT,
    EUGENE_COMPONENT_BEAT_PORT,

    EUGENE_COMPONENT_LENGTH_KNOB,
    EUGENE_COMPONENT_LENGTH_CV_KNOB,
    EUGENE_COMPONENT_LENGTH_CV_PORT,

    EUGENE_COMPONENT_HITS_KNOB,
    EUGENE_COMPONENT_HITS_CV_KNOB,
    EUGENE_COMPONENT_HITS_CV_PORT,

    EUGENE_COMPONENT_SHIFT_KNOB,
    EUGENE_COMPONENT_SHIFT_CV_KNOB,
    EUGENE_COMPONENT_SHIFT_CV_PORT,

    EUGENE_COMPONENT_REVERSE_SWITCH_ON,
    EUGENE_COMPONENT_REVERSE_SWITCH_OFF,
    EUGENE_COMPONENT_REVERSE_CV_PORT,

    EUGENE_COMPONENT_INVERT_SWITCH_ON,
    EUGENE_COMPONENT_INVERT_SWITCH_OFF,
    EUGENE_COMPONENT_INVERT_CV_PORT,

    EUGENE_COMPONENT_DISPLAY,

    EUGENE_COMPONENT_COUNT
};

struct EugeneTheme
{
	std::string m_name;
	std::string m_components[EUGENE_COMPONENT_COUNT];

	bool fromJson(json_t *root);
};

// TODO: Accessors would simplify the callers code
// eugene_config.m_themes[theme].m_component[component]
// eugene_config.getSvg(theme, component)
// eugene_config.getDefaultSvg(compenent)
// eugene_config.getPos(component)
struct EugeneConfig
{
    Vec m_positions[EUGENE_COMPONENT_COUNT];
	std::vector<EugeneTheme> m_themes;
	size_t m_default;

	bool init();
	bool fromJson(std::string path);
	bool loadComponentPositions();
};

extern EugeneConfig eugene_config;
