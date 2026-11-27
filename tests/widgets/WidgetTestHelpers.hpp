#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <vector>

namespace rack
{
namespace app
{
struct ModuleWidget;
}
namespace plugin
{
struct Model;
}
} // namespace rack

// An image read back from OpenGL, so its rows are bottom to top.
struct TestImage
{
        int width = 0;
        int height = 0;
        std::vector<uint8_t> rgba;

        // The pixel at `x`, `y` from the top left.
        const uint8_t *at(int x, int y) const
        {
                return &rgba[static_cast<size_t>(((height - 1 - y) * width + x) * 4)];
        }
};

// Defined in WidgetTestMain.cpp.
std::vector<rack::plugin::Model *> testModels();
// Draws `widget` both layers, as Rack does, onto a transparent image with `margin` pixels around it,
// at `scale` pixels per Rack pixel. The image's colours are premultiplied by its alpha.
TestImage renderTestWidget(rack::app::ModuleWidget *widget, int margin, float scale = 1.f);

// Checks the widget of the model with `slug`, with and without a module and in each theme: every
// param except `params_without_widgets`, input and output has one widget; every widget, and
// everything drawn, is inside the panel; and every widget draws something.
void checkModuleWidget(const std::string &slug, const std::set<int> &params_without_widgets);
