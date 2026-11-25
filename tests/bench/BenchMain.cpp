// Runs the benchmarks:
//
//   orbits-bench <Rack directory> <plugin directory>
//
// The drawing benchmarks need much of what Rack itself sets up: a window with an OpenGL context,
// a scene, and the plugin's models with its directory, to load its SVGs and fonts from.
//
// Like RackTestMain.cpp, this file includes the specific Rack headers it needs instead of
// rack.hpp, which marks creating the engine, window and so on as internal to Rack.
#include <app/Scene.hpp>
#include <asset.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>
#include <history.hpp>
#include <patch.hpp>
#include <plugin/Plugin.hpp>
#include <random.hpp>
#include <system.hpp>
#include <widget/event.hpp>
#include <window/Window.hpp>

#include <cstdio>

// Defined in EngineBench.cpp and DrawBench.cpp.
void runEngineBenchmarks();
void runDrawBenchmarks();

// Defined in plugin.cpp.
extern "C" void init(rack::plugin::Plugin *plugin);

// Used by DrawBench.cpp. Rack lets only a few framebuffers re-render per frame unless the frame has
// time left, and only Rack's own window loop resets the count.
void resetFramebufferBudget()
{
        APP->window->fbCount() = 0;
}

int main(int argc, char **argv)
{
        if(argc != 3)
        {
                std::fprintf(stderr, "Usage: %s <Rack directory> <plugin directory>\n", argv[0]);
                return 1;
        }

        // Rack's log isn't initialised, so its messages, e.g. one per SVG loaded, aren't shown.
        rack::system::init();
        rack::asset::systemDir = argv[1];
        // Nothing is saved, so the user directory is never written to.
        rack::asset::userDir = argv[1];
        rack::asset::init();
        // Randomization uses Rack's random number generator, which needs seeding per thread.
        rack::random::init();
        rack::window::init();

        rack::plugin::Plugin *plugin = new rack::plugin::Plugin;
        plugin->path = argv[2];
        init(plugin);

        rack::contextSet(new rack::Context);
        APP->engine = new rack::engine::Engine;
        APP->history = new rack::history::State;
        APP->event = new rack::widget::EventState;
        APP->scene = new rack::app::Scene;
        APP->patch = new rack::patch::Manager;
        APP->window = new rack::window::Window;

        runEngineBenchmarks();
        runDrawBenchmarks();

        // Deleting the context also deletes everything above that it holds.
        delete APP;
        rack::contextSet(NULL);
        rack::window::destroy();
        delete plugin;
        return 0;
}
