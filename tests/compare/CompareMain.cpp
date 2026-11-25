// Runs the comparison scenarios, writing each one's log to a file in a directory:
//
//   orbits-compare <output directory> [scenario name filter]
//
// Like RackTestMain.cpp, this file includes the specific Rack headers it needs instead of
// rack.hpp, which marks creating the engine and setting its sample rate as internal to Rack. The
// rest is in files that include rack.hpp.
#include <context.hpp>
#include <engine/Engine.hpp>
#include <history.hpp>
#include <random.hpp>
#include <widget/event.hpp>

// Defined in ComparePatch.cpp.
int runCompareScenarios(int argc, char **argv);

void setCompareSampleRate(float sample_rate)
{
        APP->engine->setSampleRate(sample_rate);
}

int main(int argc, char **argv)
{
        // Each scenario seeds Rack's random number generator, but it needs initializing per thread.
        rack::random::init();

        rack::contextSet(new rack::Context);
        APP->engine = new rack::engine::Engine;
        APP->history = new rack::history::State;
        APP->event = new rack::widget::EventState;

        const int result = runCompareScenarios(argc, argv);

        // Deleting the context also deletes its engine, history and event state.
        delete APP;
        rack::contextSet(NULL);
        return result;
}
