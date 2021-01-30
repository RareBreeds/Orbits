#pragma once

#include "plugin.hpp"

enum PolygeneComponents
{
        POLYGENE_COMPONENT_PANEL,

        POLYGENE_COMPONENT_SCREW_TOP_LEFT,
        POLYGENE_COMPONENT_SCREW_TOP_RIGHT,
        POLYGENE_COMPONENT_SCREW_BOTTOM_LEFT,
        POLYGENE_COMPONENT_SCREW_BOTTOM_RIGHT,

        POLYGENE_COMPONENT_CLOCK_PORT,
        POLYGENE_COMPONENT_SYNC_PORT,
        POLYGENE_COMPONENT_BEAT_PORT,
        POLYGENE_COMPONENT_CHANNEL_KNOB,

        POLYGENE_COMPONENT_LENGTH_KNOB,
        POLYGENE_COMPONENT_LENGTH_CV_KNOB,
        POLYGENE_COMPONENT_LENGTH_CV_PORT,

        POLYGENE_COMPONENT_HITS_KNOB,
        POLYGENE_COMPONENT_HITS_CV_KNOB,
        POLYGENE_COMPONENT_HITS_CV_PORT,

        POLYGENE_COMPONENT_SHIFT_KNOB,
        POLYGENE_COMPONENT_SHIFT_CV_KNOB,
        POLYGENE_COMPONENT_SHIFT_CV_PORT,

        POLYGENE_COMPONENT_ODDITY_KNOB,
        POLYGENE_COMPONENT_ODDITY_CV_KNOB,
        POLYGENE_COMPONENT_ODDITY_CV_PORT,

        POLYGENE_COMPONENT_REVERSE_SWITCH_ON,
        POLYGENE_COMPONENT_REVERSE_SWITCH_OFF,
        POLYGENE_COMPONENT_REVERSE_CV_PORT,

        POLYGENE_COMPONENT_INVERT_SWITCH_ON,
        POLYGENE_COMPONENT_INVERT_SWITCH_OFF,
        POLYGENE_COMPONENT_INVERT_CV_PORT,

        POLYGENE_COMPONENT_DISPLAY,

        POLYGENE_COMPONENT_COUNT
};

struct PolygeneConfig
{
        Vec m_positions[POLYGENE_COMPONENT_COUNT];
        Vec m_sizes[POLYGENE_COMPONENT_COUNT];

        bool init();
        bool loadComponentPositions();
        float readFloatAttribute(std::string &content, std::string attribute, size_t &search);

        std::string getSvg(PolygeneComponents component, int theme);
        std::string getSvg(PolygeneComponents component);
        Vec getPos(PolygeneComponents component);
        Vec getSize(PolygeneComponents component);
        std::string getThemeName(int theme);
        std::string getThemeName();
        int getDefaultThemeId();
        size_t numThemes();
};

extern PolygeneConfig polygene_config;
