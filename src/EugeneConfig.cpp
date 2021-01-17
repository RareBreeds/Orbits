#include <fstream>

#include "plugin.hpp"
#include "EugeneConfig.hpp"

EugeneConfig eugene_config;

// TODO: Error handling, bad json can make it crash in horrible to debug ways
void EugeneTheme::fromJson(json_t *root)
{
	name = json_string_value(json_object_get(root, "name"));
	INFO("name:%s", name.c_str());
	panel = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "panel")));
	INFO("panel:%s", panel.c_str());
	knob_large = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "knob_large")));
	INFO("knob_large:%s", knob_large.c_str());
	knob_small = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "knob_small")));
	INFO("knob_small:%s", knob_small.c_str());
	switch_on = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "switch_on")));
	INFO("switch_on:%s", switch_on.c_str());
	switch_off = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "switch_off")));
	INFO("switch_off:%s", switch_off.c_str());
	port = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "port")));
	INFO("port:%s", port.c_str());
	screw = asset::plugin(pluginInstance, std::string("res/") + json_string_value(json_object_get(root, "screw")));
	INFO("screw:%s", screw.c_str());
}

// TODO: If we can't find the component positions use some sensible defaults
void EugeneConfig::loadComponentPositions(void)
{
	std::ifstream ifs(m_themes[m_default].panel);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

	// Find all component positions from the svg
	size_t search = content.find("inkscape:label=\"components\"");
	if(search == std::string::npos)
	{
		return;
	}

	for(auto i = 0; i < 17; ++i)
	{
		search = content.find("<", search);
		if(search == std::string::npos)
		{
			break;
		}

		search = content.find("x=", search);
		if(search == std::string::npos)
		{
			break;
		}

		size_t float_start = search + 3;
		size_t float_end = content.find("\"", float_start);
		if(float_end == std::string::npos)
		{
			break;
		}

		float x = std::stof(content.substr(float_start, float_end - float_start));

		search = content.find("y=", float_end);
		if(search == std::string::npos)
		{
			break;
		}

		float_start = search + 3;
		float_end = content.find("\"", float_start);
		if(float_end == std::string::npos)
		{
			break;
		}

		float y = std::stof(content.substr(float_start, float_end - float_start));

		search = content.find("inkscape:label=", float_end);
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
		if(name == "length_knob")
		{
			length_knob = pos;
		}
		else if(name == "hits_knob")
		{
			hits_knob = pos;
		}
		else if(name == "reverse_knob")
		{
			reverse_knob = pos;
		}
		else if(name == "invert_knob")
		{
			invert_knob = pos;
		}
		else if(name == "length_cv")
		{
			length_cv = pos;
		}
		else if(name == "hits_cv")
		{
			hits_cv = pos;
		}
		else if(name == "reverse_cv")
		{
			reverse_cv = pos;
		}
		else if(name == "invert_cv")
		{
			invert_cv = pos;
		}
		else if(name == "beat")
		{
			beat = pos;
		}
		else if(name == "clock")
		{
			clock = pos;
		}
		else if(name == "sync")
		{
			sync = pos;
		}
		else if(name == "shift_knob")
		{
			shift_knob = pos;
		}
		else if(name == "shift_cv")
		{
			shift_cv = pos;
		}
		else if(name == "length_cv_knob")
		{
			length_cv_knob = pos;
		}
		else if(name == "hits_cv_knob")
		{
			hits_cv_knob = pos;
		}
		else if(name == "shift_cv_knob")
		{
			shift_cv_knob = pos;
		}
		else if(name == "display")
		{
			display = pos;
		}

		search = name_end;
	}
}

void EugeneConfig::fromJson(std::string path)
{
	json_t *root;
	json_error_t error;
	root = json_load_file(path.c_str(), 0, &error);
	if(root)
	{
		json_t *themes = json_object_get(root, "themes");
		if(themes)
		{
			size_t index;
			json_t *value;
			json_array_foreach(themes, index, value)
			{
				EugeneTheme theme;
				theme.fromJson(value);
				m_themes.push_back(theme);
			}

			// Default to the first theme if no default theme is provided
			// Or if it doesn't reference a theme in the themes array
			m_default = 0;
			json_t *def = json_object_get(root, "default");
			if(def)
			{
				const char *default_name = json_string_value(def);
				for(size_t i = 0; i < m_themes.size(); ++i)
				{
					if(m_themes[i].name == default_name)
					{
						m_default = i;
						break;
					}
				}
			}
		}
		else
		{
			// TODO: No themes array
		}

		json_decref(root);
	}
	else
	{
		// TODO: Unable to find Eugene config file
		// Use default assets?
	}
}
