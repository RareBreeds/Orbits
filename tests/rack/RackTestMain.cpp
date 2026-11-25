// doctest's test runner for the tests that use VCV Rack. Modules, the engine and the undo
// history expect a global Rack context, so a minimal one is created for the tests to share.
//
// Creating the engine and history is the job of Rack itself, so rack.hpp marks those functions
// as internal and plugins that call them don't compile. Like Rack's own sources, this file
// includes the specific Rack headers it needs instead, which don't mark anything internal. For
// the same reason it can't include RackTestHelpers.hpp, which includes rack.hpp, so the helpers
// defined here must match their declarations there.
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <context.hpp>
#include <engine/Engine.hpp>
#include <history.hpp>
#include <random.hpp>

void clearTestUndoHistory()
{
        APP->history->clear();
}

int main(int argc, char **argv)
{
        // Randomization uses Rack's random number generator, which needs seeding per thread.
        rack::random::init();

        rack::contextSet(new rack::Context);
        APP->engine = new rack::engine::Engine;
        APP->history = new rack::history::State;

        doctest::Context tests(argc, argv);
        const int result = tests.run();

        // Deleting the context also deletes its engine and history.
        delete APP;
        rack::contextSet(NULL);
        return result;
}
