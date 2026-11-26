// doctest's test runner for the tests that draw the modules' widgets. Building a module widget loads
// its SVGs through Rack's window, so this opens a Rack window, as the drawing benchmarks do, and
// sets up the scene and the plugin's models with its directory to load its SVGs and fonts from.
//
// Like RackTestMain.cpp, this file includes the specific Rack headers it needs instead of rack.hpp,
// which marks creating the engine and window, and the framebuffer budget, as internal to Rack.
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "WidgetTestHelpers.hpp"

#include <app/ModuleWidget.hpp>
#include <app/Scene.hpp>
#include <asset.hpp>
#include <context.hpp>
#include <engine/Engine.hpp>
#include <history.hpp>
#include <patch.hpp>
#include <plugin/Plugin.hpp>
#include <random.hpp>
#include <system.hpp>
#include <widget/FramebufferWidget.hpp>
#include <widget/event.hpp>
#include <window/Window.hpp>

// Defined in plugin.cpp.
extern "C" void init(rack::plugin::Plugin *plugin);

// libRack isn't built with AddressSanitizer, so vectors it fills, e.g. SvgSwitch::frames, look
// overflowed to the plugin's instrumented code.
extern "C" const char *__asan_default_options()
{
        return "detect_container_overflow=0";
}

static rack::plugin::Plugin *g_plugin = NULL;

std::vector<rack::plugin::Model *> testModels()
{
        return std::vector<rack::plugin::Model *>(g_plugin->models.begin(), g_plugin->models.end());
}

// Whether a visible framebuffer in `widget` still has to render.
static bool hasDirtyFramebuffer(rack::widget::Widget *widget)
{
        if(!widget->visible)
        {
                return false;
        }
        rack::widget::FramebufferWidget *fb = dynamic_cast<rack::widget::FramebufferWidget *>(widget);
        if(fb && fb->dirty)
        {
                return true;
        }
        for(rack::widget::Widget *child : widget->children)
        {
                if(hasDirtyFramebuffer(child))
                {
                        return true;
                }
        }
        return false;
}

TestImage renderTestWidget(rack::app::ModuleWidget *widget, int margin)
{
        NVGcontext *vg = APP->window->vg;
        TestImage image;
        image.width = static_cast<int>(widget->box.size.x) + 2 * margin;
        image.height = static_cast<int>(widget->box.size.y) + 2 * margin;
        image.rgba.resize(static_cast<size_t>(image.width * image.height * 4));
        NVGLUframebuffer *fb = nvgluCreateFramebuffer(vg, image.width, image.height, 0);
        REQUIRE(fb);

        // Without Rack's window loop the frame never has time left, so Rack renders one dirty
        // framebuffer per frame. Draw frames until none are left to render.
        int frames = 0;
        do
        {
                nvgBeginFrame(vg, static_cast<float>(image.width), static_cast<float>(image.height), 1.f);
                nvgTranslate(vg, static_cast<float>(margin), static_cast<float>(margin));
                widget->step();
                APP->window->fbCount() = 0;
                rack::widget::Widget::DrawArgs args;
                args.vg = vg;
                args.clipBox = rack::math::Rect(
                        rack::math::Vec(static_cast<float>(-margin)),
                        rack::math::Vec(static_cast<float>(image.width), static_cast<float>(image.height)));
                widget->draw(args);
                widget->drawLayer(args, 1);

                // Only now, as framebuffers rendered while drawing bind the default framebuffer.
                nvgluBindFramebuffer(fb);
                glViewport(0, 0, image.width, image.height);
                glClearColor(0.f, 0.f, 0.f, 0.f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                nvgEndFrame(vg);
        } while(hasDirtyFramebuffer(widget) && ++frames < 200);
        CHECK_FALSE(hasDirtyFramebuffer(widget));

        glReadPixels(0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
        nvgluBindFramebuffer(NULL);
        nvgluDeleteFramebuffer(fb);
        return image;
}

int main(int argc, char **argv)
{
        // Rack's log isn't initialised, so its messages, e.g. one per SVG loaded, aren't shown.
        rack::system::init();
        rack::asset::systemDir = ORBITS_TEST_RACK_DIR;
        // Nothing is saved, so the user directory is never written to.
        rack::asset::userDir = ORBITS_TEST_RACK_DIR;
        rack::asset::init();
        rack::random::init();
        rack::window::init();

        g_plugin = new rack::plugin::Plugin;
        g_plugin->path = ORBITS_TEST_PLUGIN_DIR;
        init(g_plugin);

        rack::contextSet(new rack::Context);
        APP->engine = new rack::engine::Engine;
        APP->history = new rack::history::State;
        APP->event = new rack::widget::EventState;
        APP->scene = new rack::app::Scene;
        APP->patch = new rack::patch::Manager;
        APP->window = new rack::window::Window;

        doctest::Context tests(argc, argv);
        const int result = tests.run();

        // Deleting the context also deletes everything above that it holds.
        delete APP;
        rack::contextSet(NULL);
        rack::window::destroy();
        delete g_plugin;
        return result;
}
