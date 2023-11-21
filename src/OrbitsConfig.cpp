#include "OrbitsConfig.hpp"
#include "plugin.hpp"

static json_t *orbits_json_load_file(const char *path, size_t flags, json_error_t *error)
{
        FILE *fp = std::fopen(path, "rb");
        json_t *json = json_loadf(fp, flags, error);
        if (fp)
        {
                std::fclose(fp);
        }
        return json;
}

static std::string read_file_contents(const std::string& filename)
{
        // Read a file into a string without using ifstream.
        // MINGW doesn't have an ifstream that supports being constructed
        // from a wide string, which is required on Windows for opening
        // paths containing UTF8 characters.
        FILE* file = std::fopen(filename.c_str(), "rb");
        if (file == NULL)
        {
                return "";
        }

        fseek(file, 0, SEEK_END);
        long ftell_result = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (ftell_result <= 0)
        {
                std::fclose(file);
                return "";
        }

        size_t file_size = (size_t) ftell_result;
        std::vector<char> buffer(file_size + 1);
        size_t bytes_read = fread(buffer.data(), 1, file_size, file);

        std::fclose(file);

        if (bytes_read != static_cast<size_t>(file_size))
        {
                return "";
        }

        buffer[file_size] = '\0';

        return std::string(buffer.data());
}

OrbitsConfig::OrbitsConfig(std::string path)
{
        m_path = path;
}

float OrbitsConfig::rFindFloatAttribute(std::string &content, std::string attribute, size_t search)
{
        search = content.rfind(attribute + "=", search);
        if(search == std::string::npos)
        {
                WARN("Unable to find attribute \"%s\"", attribute.c_str());
                return 0.0f;
        }

        size_t float_start = search + attribute.length() + 2;
        size_t float_end = content.find("\"", float_start);
        if(float_end == std::string::npos)
        {
                WARN("Unable to find content for attribute \"%s\"", attribute.c_str());
                return 0.0f;
        }

        float value = std::stof(content.substr(float_start, float_end - float_start));
        return value;
}

std::string OrbitsConfig::getSvg(std::string component, int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
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
        std::string content = read_file_contents(getSvg("panel"));
        size_t search = content.find("inkscape:label=\"components\"");
        if(search == std::string::npos)
        {
                WARN("Unable to find inkscape:label=\"components\"");
                return mm2px(Vec(0.f, 0.f));
        }

        search = content.find("inkscape:label=\"" + component, search);
        if(search == std::string::npos)
        {
                WARN("Unable to find inkscape:label=\"%s\"", component.c_str());
                return mm2px(Vec(0.f, 0.f));
        }

        float x = rFindFloatAttribute(content, "x", search);
        float y = rFindFloatAttribute(content, "y", search);
        return mm2px(Vec(x, y));
}

Vec OrbitsConfig::getSize(std::string component)
{
        std::string content = read_file_contents(getSvg("panel"));
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
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
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

std::vector<std::string> OrbitsConfig::getThemeNames()
{
        std::vector<std::string> names;

        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        size_t index;
        json_t *value;
        json_array_foreach(themes, index, value)
        {
                json_t *name = json_object_get(value, "name");
                const char *theme_name = json_string_value(name);
                names.push_back(theme_name);

        }
        json_decref(root);

        return names;
}

int OrbitsConfig::getDefaultThemeId()
{
        int default_theme_id = 0;
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
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
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        size_t count = json_array_size(themes);
        json_decref(root);
        return count;
}

std::array<uint8_t, 3> OrbitsConfig::getColour(std::string component, int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, m_path);
        json_t *root = orbits_json_load_file(path.c_str(), 0, &error);
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
