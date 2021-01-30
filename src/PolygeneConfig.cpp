#include <fstream>

#include "PolygeneConfig.hpp"
#include "plugin.hpp"

PolygeneConfig polygene_config;

static const char *componentJsonString(PolygeneComponents component)
{
        switch(component)
        {
        case POLYGENE_COMPONENT_PANEL:
                return "panel";
        case POLYGENE_COMPONENT_SCREW_TOP_LEFT:
                return "screw_top_left";
        case POLYGENE_COMPONENT_SCREW_TOP_RIGHT:
                return "screw_top_right";
        case POLYGENE_COMPONENT_SCREW_BOTTOM_LEFT:
                return "screw_bottom_left";
        case POLYGENE_COMPONENT_SCREW_BOTTOM_RIGHT:
                return "screw_bottom_right";
        case POLYGENE_COMPONENT_CLOCK_PORT:
                return "clock_port";
        case POLYGENE_COMPONENT_SYNC_PORT:
                return "sync_port";
        case POLYGENE_COMPONENT_BEAT_PORT:
                return "beat_port";
        case POLYGENE_COMPONENT_CHANNEL_KNOB:
                return "channel_knob";
        case POLYGENE_COMPONENT_LENGTH_KNOB:
                return "length_knob";
        case POLYGENE_COMPONENT_LENGTH_CV_KNOB:
                return "length_cv_knob";
        case POLYGENE_COMPONENT_LENGTH_CV_PORT:
                return "length_cv_port";
        case POLYGENE_COMPONENT_HITS_KNOB:
                return "hits_knob";
        case POLYGENE_COMPONENT_HITS_CV_KNOB:
                return "hits_cv_knob";
        case POLYGENE_COMPONENT_HITS_CV_PORT:
                return "hits_cv_port";
        case POLYGENE_COMPONENT_SHIFT_KNOB:
                return "shift_knob";
        case POLYGENE_COMPONENT_SHIFT_CV_KNOB:
                return "shift_cv_knob";
        case POLYGENE_COMPONENT_SHIFT_CV_PORT:
                return "shift_cv_port";
        case POLYGENE_COMPONENT_ODDITY_KNOB:
                return "oddity_knob";
        case POLYGENE_COMPONENT_ODDITY_CV_KNOB:
                return "oddity_cv_knob";
        case POLYGENE_COMPONENT_ODDITY_CV_PORT:
                return "oddity_cv_port";
        case POLYGENE_COMPONENT_REVERSE_SWITCH_ON:
                return "reverse_switch_on";
        case POLYGENE_COMPONENT_REVERSE_SWITCH_OFF:
                return "reverse_switch_off";
        case POLYGENE_COMPONENT_REVERSE_CV_PORT:
                return "reverse_cv_port";
        case POLYGENE_COMPONENT_INVERT_SWITCH_ON:
                return "invert_switch_on";
        case POLYGENE_COMPONENT_INVERT_SWITCH_OFF:
                return "invert_switch_off";
        case POLYGENE_COMPONENT_INVERT_CV_PORT:
                return "invert_cv_port";
        default:
                assert(false);
        }
}

static PolygeneComponents componentSvgEnum(std::string str)
{
        if(str == "clock")
                return POLYGENE_COMPONENT_CLOCK_PORT;
        if(str == "sync")
                return POLYGENE_COMPONENT_SYNC_PORT;
        if(str == "beat")
                return POLYGENE_COMPONENT_BEAT_PORT;
        if(str == "channel_knob")
                return POLYGENE_COMPONENT_CHANNEL_KNOB;
        if(str == "length_knob")
                return POLYGENE_COMPONENT_LENGTH_KNOB;
        if(str == "length_cv_knob")
                return POLYGENE_COMPONENT_LENGTH_CV_KNOB;
        if(str == "length_cv")
                return POLYGENE_COMPONENT_LENGTH_CV_PORT;
        if(str == "hits_knob")
                return POLYGENE_COMPONENT_HITS_KNOB;
        if(str == "hits_cv_knob")
                return POLYGENE_COMPONENT_HITS_CV_KNOB;
        if(str == "hits_cv")
                return POLYGENE_COMPONENT_HITS_CV_PORT;
        if(str == "shift_knob")
                return POLYGENE_COMPONENT_SHIFT_KNOB;
        if(str == "shift_cv_knob")
                return POLYGENE_COMPONENT_SHIFT_CV_KNOB;
        if(str == "shift_cv")
                return POLYGENE_COMPONENT_SHIFT_CV_PORT;
        if(str == "oddity_knob")
                return POLYGENE_COMPONENT_ODDITY_KNOB;
        if(str == "oddity_cv_knob")
                return POLYGENE_COMPONENT_ODDITY_CV_KNOB;
        if(str == "oddity_cv")
                return POLYGENE_COMPONENT_ODDITY_CV_PORT;
        if(str == "reverse_knob")
                return POLYGENE_COMPONENT_REVERSE_SWITCH_ON;
        if(str == "reverse_cv")
                return POLYGENE_COMPONENT_REVERSE_CV_PORT;
        if(str == "invert_knob")
                return POLYGENE_COMPONENT_INVERT_SWITCH_ON;
        if(str == "invert_cv")
                return POLYGENE_COMPONENT_INVERT_CV_PORT;
        if(str == "display")
                return POLYGENE_COMPONENT_DISPLAY;

        // The remaining components don't have positions in the SVG
        return POLYGENE_COMPONENT_COUNT;
}

// TODO: not calculating paths here would save some memory
#define LOAD_PATH(_enum)                                                                                               \
        do                                                                                                             \
        {                                                                                                              \
                obj = json_object_get(root, componentJsonString(_enum));                                               \
                if(!obj)                                                                                               \
                        return false;                                                                                  \
                str = json_string_value(obj);                                                                          \
                if(!str)                                                                                               \
                        return false;                                                                                  \
                std::string path = std::string("res/") + str;                                                          \
                m_components[_enum] = asset::plugin(pluginInstance, path);                                             \
        } while(0)

bool PolygeneTheme::fromJson(json_t *root)
{
        json_t *obj;

        obj = json_object_get(root, "name");
        if(!obj)
                return false;
        const char *str = json_string_value(obj);
        if(!str)
                return false;
        m_name = str;

        LOAD_PATH(POLYGENE_COMPONENT_PANEL);
        LOAD_PATH(POLYGENE_COMPONENT_SCREW_TOP_LEFT);
        LOAD_PATH(POLYGENE_COMPONENT_SCREW_TOP_RIGHT);
        LOAD_PATH(POLYGENE_COMPONENT_SCREW_BOTTOM_LEFT);
        LOAD_PATH(POLYGENE_COMPONENT_SCREW_BOTTOM_RIGHT);
        LOAD_PATH(POLYGENE_COMPONENT_CLOCK_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_SYNC_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_BEAT_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_CHANNEL_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_LENGTH_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_LENGTH_CV_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_LENGTH_CV_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_HITS_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_HITS_CV_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_HITS_CV_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_SHIFT_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_SHIFT_CV_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_SHIFT_CV_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_ODDITY_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_ODDITY_CV_KNOB);
        LOAD_PATH(POLYGENE_COMPONENT_ODDITY_CV_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_REVERSE_SWITCH_ON);
        LOAD_PATH(POLYGENE_COMPONENT_REVERSE_SWITCH_OFF);
        LOAD_PATH(POLYGENE_COMPONENT_REVERSE_CV_PORT);
        LOAD_PATH(POLYGENE_COMPONENT_INVERT_SWITCH_ON);
        LOAD_PATH(POLYGENE_COMPONENT_INVERT_SWITCH_OFF);
        LOAD_PATH(POLYGENE_COMPONENT_INVERT_CV_PORT);

        return true;
}

bool PolygeneConfig::init()
{
        return loadComponentPositions();
}

float PolygeneConfig::readFloatAttribute(std::string &content, std::string attribute, size_t &search)
{
        search = content.find(attribute + "=", search);
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

bool PolygeneConfig::loadComponentPositions()
{
        std::ifstream ifs(getSvg(POLYGENE_COMPONENT_PANEL));
        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

        // Find all component positions from the svg
        size_t search = content.find("inkscape:label=\"components\"");
        if(search == std::string::npos)
        {
                return false;
        }

        // TODO: Should validate that all the positions have been found.
        while(true)
        {
                search = content.find("<", search);
                if(search == std::string::npos)
                {
                        break;
                }

                float width = 0.0f;
                float height = 0.0f;
                if(content.substr(search + 1, 4) == "rect")
                {
                        // Read width and height
                        width = readFloatAttribute(content, "width", search);
                        height = readFloatAttribute(content, "height", search);
                }

                float x = readFloatAttribute(content, "x", search);
                float y = readFloatAttribute(content, "y", search);

                search = content.find("inkscape:label=", search);
                if(search == std::string::npos)
                {
                        break;
                }

                size_t name_start = search + 16;
                size_t name_end = content.find("\"", name_start);
                if(name_end == std::string::npos)
                {
                        break;
                }

                std::string name = content.substr(name_start, name_end - name_start);

                Vec pos = mm2px(Vec(x, y));
                Vec size = mm2px(Vec(width, height));
                PolygeneComponents component = componentSvgEnum(name);
                if(component < POLYGENE_COMPONENT_COUNT)
                {
                        m_positions[component] = pos;
                        m_sizes[component] = size;
                }

                search = name_end;
        }

        return true;
}


std::string PolygeneConfig::getSvg(PolygeneComponents component, int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, "res/polygene-layout.json");
        json_t *root = json_load_file(path.c_str(), 0, &error);
        // TODO: root and error handling

        // Find this theme ID
        json_t *themes = json_object_get(root, "themes");
        json_t *entry = json_array_get(themes, theme);

        // TODO: Replace component enum with string
        json_t *obj = json_object_get(entry, componentJsonString(component));
        const char *str = json_string_value(obj);
        return asset::plugin(pluginInstance, std::string("res/") + str);
}

std::string PolygeneConfig::getSvg(PolygeneComponents component)
{
        return getSvg(component, getDefaultThemeId());
}

Vec PolygeneConfig::getPos(PolygeneComponents component)
{
        return m_positions[component];
}

Vec PolygeneConfig::getSize(PolygeneComponents component)
{
        return m_sizes[component];
}

std::string PolygeneConfig::getThemeName(int theme)
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, "res/polygene-layout.json");
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        json_t *entry = json_array_get(themes, theme);
        json_t *name = json_object_get(entry, "name");
        std::string theme_name(json_string_value(name));
        json_decref(root);
        return theme_name;
}

std::string PolygeneConfig::getThemeName()
{
        return getThemeName(getDefaultThemeId());
}

int PolygeneConfig::getDefaultThemeId()
{
        int default_theme_id = 0;
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, "res/polygene-layout.json");
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *def = json_object_get(root, "default");
        const char *default_name = json_string_value(def);
        json_t *themes = json_object_get(root, "themes");
        size_t index;
        json_t *value;
        json_array_foreach(themes, index, value)
        {
            json_t *name = json_object_get(value, "name");
            if(std::string(json_string_value(name)) == std::string(default_name))
            {
                default_theme_id = index;
                break;
            }
        }
        json_decref(root);

        return default_theme_id;
}

size_t PolygeneConfig::numThemes()
{
        json_error_t error;
        std::string path = asset::plugin(pluginInstance, "res/polygene-layout.json");
        json_t *root = json_load_file(path.c_str(), 0, &error);
        json_t *themes = json_object_get(root, "themes");
        size_t count = json_array_size(themes);
        json_decref(root);
        return count;
}