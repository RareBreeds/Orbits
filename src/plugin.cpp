#include <fstream>

#include "plugin.hpp"


Plugin *pluginInstance;
Config config;

void init(Plugin *p) {
	pluginInstance = p;
	config.fromJson(asset::plugin(pluginInstance, "res/eugene-layout.json"));
	config.loadComponentPositions();

	// Add modules here
	p->addModel(modelRareBreeds_Orbits_Eugene);
	p->addModel(modelRareBreeds_Orbits_Polygene);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

// TODO: Error handling, bad json can make it crash in horrible to debug ways
void Theme::fromJson(json_t *root)
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
// TODO: Error handling
// TODO: Avoid code duplication
void Config::loadComponentPositions(void)
{
	std::ifstream ifs(m_themes[m_default].panel);
	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

	// Find all component positions from the svg
	size_t search = content.find("inkscape:label=\"components\"");

	for(auto i = 0; i < 17; ++i)
	{
		search = content.find("<", search);
		search = content.find("x=", search);
		size_t float_start = search + 3;
		size_t float_end = content.find("\"", float_start);
		float x = std::stof(content.substr(float_start, float_end - float_start));

		search = content.find("y=", float_end);
		float_start = search + 3;
		float_end = content.find("\"", float_start);
		float y = std::stof(content.substr(float_start, float_end - float_start));

		search = content.find("inkscape:label=", float_end);
		size_t name_start = search + 16;
		size_t name_end = content.find("\"", name_start);
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

void Config::fromJson(std::string path)
{
	json_t *root;
	json_error_t error;
	root = json_load_file(path.c_str(), 0, &error);
	if (!root)
	{
		// TODO: Handle errors, the whole of rack crashes if this crashes
	}

	json_t *themes = json_object_get(root, "themes");
	size_t index;
	json_t *value;
	json_array_foreach(themes, index, value)
	{
		Theme theme;
		theme.fromJson(value);
		m_themes.push_back(theme);
	}

	const char *default_name = json_string_value(json_object_get(root, "default"));
	for(size_t i = 0; i < m_themes.size(); ++i)
	{
		if(m_themes[i].name == default_name)
		{
			m_default = i;
			break;
		}
	}

	json_decref(root);
}
