#include <fstream>

#include "OrbitsConfig.hpp"
#include "plugin.hpp"

OrbitsConfig::OrbitsConfig(std::string path)
{
        m_path = path;
}

float OrbitsConfig::rFindFloatAttribute(std::string &content, std::string attribute, size_t search)
{
        search = content.rfind(attribute + "=", search);
        if(search == std::string::npos)
        {
                return 0.0f;
        }

        size_t float_start = search + attribute.length() + 2;
        size_t float_end = content.find("\"", float_start);
        if(float_end == std::string::npos)
        {
                return 0.0f;
        }

        search = float_end;
        float value = std::stof(content.substr(float_start, float_end - float_start));
        return value;
}

std::string OrbitsConfig::getSvg(std::string component, int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        json_t *entry = json_array_get(themes, theme);
        json_t *obj = json_object_get(entry, component.c_str());
        const char *str = json_string_value(obj);
        std::string svg = str ? asset::plugin(pluginInstance, std::string("res/") + str) : "";
        json_decref(root);
        return svg;
}

std::string OrbitsConfig::getSvg(std::string component)
{
        return getSvg(component, getDefaultThemeId());
}

Vec OrbitsConfig::getPos(std::string component)
{
        std::ifstream ifs(getSvg("panel"));
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        size_t search = content.find("inkscape:label=\"components\"");
        search = content.find("inkscape:label=\"" + component, search);
        float x = rFindFloatAttribute(content, "x", search);
        float y = rFindFloatAttribute(content, "y", search);
        return mm2px(Vec(x, y));
}

Vec OrbitsConfig::getSize(std::string component)
{
        std::ifstream ifs(getSvg("panel"));
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        size_t search = content.find("inkscape:label=\"components\"");
        search = content.find("inkscape:label=\"" + component, search);
        float w = rFindFloatAttribute(content, "width", search);
        float h = rFindFloatAttribute(content, "height", search);
        return mm2px(Vec(w, h));
}

std::string OrbitsConfig::getThemeName(int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        json_t *entry = json_array_get(themes, theme);
        json_t *name = json_object_get(entry, "name");
        const char *theme_name = json_string_value(name);
        std::string theme_name_str = theme_name ? theme_name : "";
        json_decref(root);
        return theme_name_str;
}

std::string OrbitsConfig::getThemeName()
{
        return getThemeName(getDefaultThemeId());
}

int OrbitsConfig::getDefaultThemeId()
{
        int default_theme_id = 0;
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *def = json_object_get(root, "default");
        const char *default_name = json_string_value(def);
        json_t *themes = json_object_get(root, "themes");
        size_t index;
        json_t *value;
        json_array_foreach(themes, index, value)
        {
                json_t *name = json_object_get(value, "name");
                const char *theme_name = json_string_value(name);
                if(theme_name && default_name && (std::string(theme_name) == std::string(default_name)))
                {
                        default_theme_id = index;
                        break;
                }
        }
        json_decref(root);

        return default_theme_id;
}

size_t OrbitsConfig::numThemes()
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        size_t count = json_array_size(themes);
        json_decref(root);
        return count;
}

std::array<uint8_t, 3> OrbitsConfig::getColour(std::string component, int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        json_t *entry = json_array_get(themes, theme);
        json_t *obj = json_object_get(entry, component.c_str());

        std::array<uint8_t, 3> colour;
        for(auto i = 0u; i < 3; ++i)
        {
                json_t *c = json_array_get(obj, i);
                colour[i] = json_integer_value(c);
        }

        json_decref(root);
        return colour;
}

std::array<uint8_t, 3> OrbitsConfig::getColour(std::string component)
{
        return getColour(component, getDefaultThemeId());
}