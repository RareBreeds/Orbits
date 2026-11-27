// Not a test: saves screenshots of each module in each theme, with and without bypass, and with every
// button off and on, to the directory in ORBITS_SCREENSHOT_DIR. Skipped unless run with doctest's
// --no-skip, which `make -C tests screenshots RACK_DIR=<path>` does.
#include "doctest.h"

#include "EugeneModule.hpp"
#include "OrbitsWidget.hpp"
#include "PolygeneModule.hpp"
#include "WidgetTestHelpers.hpp"

#include <stb_image_write.h>

#include <algorithm>
#include <cstdlib>
#include <vector>

// Rack pixels to image pixels, as at 200% zoom.
static const float kScreenshotScale = 2.f;

struct ScreenshotModule
{
        const char *slug;
        const char *name;
        // Buttons that latch, set before the module processes so the display shows them.
        std::vector<int> toggles;
        // Momentary buttons, set after processing so they light without acting, e.g. Random.
        std::vector<int> momentaries;
};

static Model *findScreenshotModel(const std::string &slug)
{
        for(Model *model : testModels())
        {
                if(model->slug == slug)
                {
                        return model;
                }
        }
        return NULL;
}

// Writes `image` as a PNG, top row first and with straight alpha, as PNGs store it.
static void writePng(const std::string &path, const TestImage &image)
{
        std::vector<uint8_t> rgba(image.rgba.size());
        for(int y = 0; y < image.height; ++y)
        {
                for(int x = 0; x < image.width; ++x)
                {
                        const uint8_t *in = image.at(x, y);
                        uint8_t *out = &rgba[static_cast<size_t>((y * image.width + x) * 4)];
                        for(int c = 0; c < 3; ++c)
                        {
                                out[c] = in[3] ? static_cast<uint8_t>(std::min(255, in[c] * 255 / in[3])) : 0;
                        }
                        out[3] = in[3];
                }
        }
        CHECK(stbi_write_png(path.c_str(), image.width, image.height, 4, rgba.data(), image.width * 4) != 0);
        MESSAGE(path);
}

static void saveScreenshots(const ScreenshotModule &config, const std::string &dir)
{
        Model *model = findScreenshotModel(config.slug);
        REQUIRE(model);
        for(size_t theme = 0; theme < kOrbitsThemeNames.size(); ++theme)
        {
                for(int buttons_on = 0; buttons_on < 2; ++buttons_on)
                {
                        for(int bypassed = 0; bypassed < 2; ++bypassed)
                        {
                                Module *module = model->createModule();
                                // The widget deletes its module, which it expects to find in the engine.
                                APP->engine->addModule(module);
                                ModuleWidget *widget = model->createModuleWidget(module);
                                OrbitsWidget *orbits = dynamic_cast<OrbitsWidget *>(widget);
                                REQUIRE(orbits);
                                // As chosen from the menu, so step() doesn't switch to Rack's theme.
                                orbits->m_theme_setting.m_follow_rack = false;
                                orbits->m_theme_setting.m_theme = static_cast<OrbitsTheme>(theme);
                                orbits->switchTheme(static_cast<OrbitsTheme>(theme));

                                // Process a little, as the engine would before the module was
                                // bypassed, so the display shows the module's state. Polygene sets
                                // its knobs and buttons from the active channel on the first
                                // process, so the toggles are set after that.
                                Module::ProcessArgs args;
                                args.sampleRate = 48000.f;
                                args.sampleTime = 1.f / args.sampleRate;
                                for(int i = 0; i < 64; ++i)
                                {
                                        if(i == 1 && buttons_on)
                                        {
                                                for(int id : config.toggles)
                                                {
                                                        module->params[static_cast<size_t>(id)].setValue(1.f);
                                                }
                                        }
                                        args.frame = i;
                                        module->process(args);
                                }
                                if(buttons_on)
                                {
                                        for(int id : config.momentaries)
                                        {
                                                module->params[static_cast<size_t>(id)].setValue(1.f);
                                        }
                                }
                                APP->engine->bypassModule(module, bypassed != 0);

                                const std::string path = dir + "/" + config.name + "-" + kOrbitsThemeNames[theme] +
                                                         (buttons_on ? "-ButtonsOn" : "-ButtonsOff") +
                                                         (bypassed ? "-Bypassed" : "") + ".png";
                                writePng(path, renderTestWidget(widget, 0, kScreenshotScale));
                                delete widget;
                        }
                }
        }
}

TEST_CASE("Screenshots" * doctest::skip())
{
        const char *dir = std::getenv("ORBITS_SCREENSHOT_DIR");
        REQUIRE(dir);

        ScreenshotModule eugene = {
                "RareBreeds_Orbits_Eugene",
                "Eugene",
                {RareBreeds_Orbits_Eugene::REVERSE_KNOB_PARAM, RareBreeds_Orbits_Eugene::INVERT_KNOB_PARAM},
                {RareBreeds_Orbits_Eugene::SYNC_KNOB_PARAM}};
        ScreenshotModule polygene = {
                "RareBreeds_Orbits_Polygene",
                "Polygene",
                {RareBreeds_Orbits_Polygene::REVERSE_KNOB_PARAM, RareBreeds_Orbits_Polygene::INVERT_KNOB_PARAM},
                {RareBreeds_Orbits_Polygene::RANDOM_KNOB_PARAM, RareBreeds_Orbits_Polygene::SYNC_KNOB_PARAM}};
        saveScreenshots(eugene, dir);
        saveScreenshots(polygene, dir);
}
