#include <fstream>

#include "plugin.hpp"
#include "EugeneConfig.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
	pluginInstance = p;

	// Add modules here
	p->addModel(modelRareBreeds_Orbits_Eugene);
	p->addModel(modelRareBreeds_Orbits_Polygene);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	eugene_config.init();
}
