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

struct PolygeneTheme
{
        std::string m_name;
        std::string m_components[POLYGENE_COMPONENT_COUNT];

        bool fromJson(json_t *root);
};

struct PolygeneConfig
{
        Vec m_positions[POLYGENE_COMPONENT_COUNT];
        std::vector<PolygeneTheme> m_themes;
        size_t m_default;

        bool init();
        bool fromJson(std::string path);
        bool loadComponentPositions();

        std::string getSvg(PolygeneComponents component, int theme)
        {
                return m_themes[theme].m_components[component];
        }

        std::string getSvg(PolygeneComponents component)
        {
                return getSvg(component, m_default);
        }

        Vec getPos(PolygeneComponents component)
        {
                return m_positions[component];
        }

        std::string getThemeName(int theme)
        {
                return m_themes[theme].m_name;
        }

        std::string getThemeName()
        {
                return getThemeName(m_default);
        }
};

extern PolygeneConfig polygene_config;
