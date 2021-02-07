#pragma once

#include "plugin.hpp"

struct OrbitsConfig
{
        OrbitsConfig(std::string path);
        std::string getSvg(std::string component, int theme);
        std::string getSvg(std::string component);
        Vec getPos(std::string component);
        Vec getSize(std::string component);
        std::string getThemeName(int theme);
        std::string getThemeName();
        std::array<uint8_t, 3> getColour(std::string component, int theme);
        std::array<uint8_t, 3> getColour(std::string component);
        int getDefaultThemeId();
        size_t numThemes();
private:
        std::string m_path;
        float rFindFloatAttribute(std::string &content, std::string attribute, size_t search);        
};
