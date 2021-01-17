#include "plugin.hpp"


Plugin *pluginInstance;
Config config;

void init(Plugin *p) {
	pluginInstance = p;
	config.fromJson(asset::plugin(pluginInstance, "res/eugene-layout.json"));

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

void Config::fromJson(std::string path)
{
	json_t *root;
	json_error_t error;
	root = json_load_file(path.c_str(), 0, &error);
	if (!root) {
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

	const char *default_name = json_string_value(json_object_get(root, "default_theme"));
	INFO("default_theme:%s", default_name);
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