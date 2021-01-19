#include <fstream>

#include "plugin.hpp"
#include "EugeneConfig.hpp"

EugeneConfig eugene_config;

// TODO: not calculating paths here would save some memory
#define LOAD_PATH(_name) do {\
	obj = json_object_get(root, #_name); \
	if(!obj) return false; \
	str = json_string_value(obj); \
	if(!str) return false; \
	std::string path = std::string("res/") + str; \
	m_components[_name] = asset::plugin(pluginInstance, path); \
} while(0)

bool EugeneTheme::fromJson(json_t *root)
{
	json_t *obj;

	obj = json_object_get(root, "name");
	if(!obj) return false;
	const char *str = json_string_value(obj);
	if(!str) return false;
	m_name = str;

	LOAD_PATH(panel);
	LOAD_PATH(screw_top_left);
	LOAD_PATH(screw_top_right);
	LOAD_PATH(screw_bottom_left);
	LOAD_PATH(screw_bottom_right);
	LOAD_PATH(screw_top_left);
	LOAD_PATH(clock_port);
	LOAD_PATH(sync_port);
	LOAD_PATH(beat_port);
	LOAD_PATH(length_knob);
	LOAD_PATH(length_cv_knob);
	LOAD_PATH(length_cv_port);
	LOAD_PATH(shift_knob);
	LOAD_PATH(hits_knob);
	LOAD_PATH(hits_cv_knob);
	LOAD_PATH(hits_cv_port);
	LOAD_PATH(shift_cv_knob);
	LOAD_PATH(shift_cv_port);
	LOAD_PATH(reverse_switch_on);
	LOAD_PATH(reverse_switch_off);
	LOAD_PATH(reverse_cv_port);
	LOAD_PATH(invert_switch_on);
	LOAD_PATH(invert_switch_off);
	LOAD_PATH(invert_cv_port);

	return true;
}

bool EugeneConfig::init()
{
	bool result = fromJson(asset::plugin(pluginInstance, "res/eugene-layout.json"));
	if(!result) return false;

	return loadComponentPositions();
}

bool EugeneConfig::loadComponentPositions()
{
	std::ifstream ifs(m_themes[m_default].m_components[EugeneTheme::panel]);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

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

	return true;
}

bool EugeneConfig::fromJson(std::string path)
{
	json_t *root;
	json_error_t error;
	root = json_load_file(path.c_str(), 0, &error);
	if(!root) return false;

	json_t *themes = json_object_get(root, "themes");
	if(!themes)
	{
		json_decref(root);
		return false;
	}

	size_t index;
	json_t *value;
	json_array_foreach(themes, index, value)
	{
		EugeneTheme theme;
		theme.fromJson(value);
		m_themes.push_back(theme);
	}

	json_t *def = json_object_get(root, "default");
	if(!def)
	{
		json_decref(root);
		return false;
	}

	const char *default_name = json_string_value(def);
	if(!default_name)
	{
		json_decref(root);
	}

	size_t i;
	for(i = 0; i < m_themes.size(); ++i)
	{
		if(m_themes[i].m_name == default_name)
		{
			m_default = i;
			break;
		}
	}

	if(i == m_themes.size())
	{
		json_decref(root);
		return false;
	}

	json_decref(root);

	return true;
}
