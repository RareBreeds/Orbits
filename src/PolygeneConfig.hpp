#pragma once

#include "plugin.hpp"

struct PolygeneConfig
{
        std::string getSvg(std::string component, int theme);
        std::string getSvg(std::string component);
        Vec getPos(std::string component);
        Vec getSize(std::string component);
        std::string getThemeName(int theme);
        std::string getThemeName();
        int getDefaultThemeId();
        size_t numThemes();
private:
        float rFindFloatAttribute(std::string &content, std::string attribute, size_t search);        
};

extern PolygeneConfig polygene_config;
