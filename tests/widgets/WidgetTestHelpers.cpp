#include "WidgetTestHelpers.hpp"

#include "doctest.h"

#include "OrbitsWidget.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cxxabi.h>
#include <map>
#include <typeinfo>
#include <utility>

// Space around the module in rendered images, to see anything drawn outside it.
static const int kMargin = 16;

static Model *findModel(const std::string &slug)
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

static void checkControls(ModuleWidget *widget, Module *module, const std::set<int> &params_without_widgets)
{
        std::map<int, int> params;
        for(ParamWidget *param : widget->getParams())
        {
                ++params[param->paramId];
        }
        for(int id = 0; id < static_cast<int>(module->params.size()); ++id)
        {
                CAPTURE(module->getParamQuantity(id)->getLabel());
                CHECK(params[id] == (params_without_widgets.count(id) ? 0 : 1));
        }

        std::map<std::pair<int, int>, int> ports;
        for(PortWidget *port : widget->getPorts())
        {
                ++ports[std::make_pair(static_cast<int>(port->type), port->portId)];
        }
        for(int id = 0; id < static_cast<int>(module->inputs.size()); ++id)
        {
                CAPTURE(module->inputInfos[static_cast<size_t>(id)]->name);
                CHECK(ports[std::make_pair(static_cast<int>(engine::Port::INPUT), id)] == 1);
        }
        for(int id = 0; id < static_cast<int>(module->outputs.size()); ++id)
        {
                CAPTURE(module->outputInfos[static_cast<size_t>(id)]->name);
                CHECK(ports[std::make_pair(static_cast<int>(engine::Port::OUTPUT), id)] == 1);
        }
}

// Every SVG loaded and has finite shapes; nanosvg loads some malformed SVGs without an error.
static void checkSvgs(widget::Widget *widget)
{
        widget::SvgWidget *svg_widget = dynamic_cast<widget::SvgWidget *>(widget);
        if(svg_widget)
        {
                REQUIRE(svg_widget->svg);
                REQUIRE(svg_widget->svg->handle);
                int shapes = 0;
                for(NSVGshape *shape = svg_widget->svg->handle->shapes; shape; shape = shape->next)
                {
                        ++shapes;
                        for(float bound : shape->bounds)
                        {
                                CHECK(std::isfinite(bound));
                        }
                }
                CHECK(shapes > 0);
        }
        for(widget::Widget *child : widget->children)
        {
                checkSvgs(child);
        }
}

static bool samePixel(const TestImage &a, const TestImage &b, int x, int y)
{
        const uint8_t *pa = a.at(x, y);
        const uint8_t *pb = b.at(x, y);
        return pa[0] == pb[0] && pa[1] == pb[1] && pa[2] == pb[2] && pa[3] == pb[3];
}

// Pixels outside the module, allowing a pixel of anti-aliasing, that aren't transparent.
static int pixelsOutside(const TestImage &image, const math::Vec &size)
{
        int count = 0;
        for(int y = 0; y < image.height; ++y)
        {
                for(int x = 0; x < image.width; ++x)
                {
                        const float mx = static_cast<float>(x - kMargin);
                        const float my = static_cast<float>(y - kMargin);
                        const bool inside = mx >= -1.f && my >= -1.f && mx <= size.x && my <= size.y;
                        if(!inside && image.at(x, y)[3] != 0)
                        {
                                ++count;
                        }
                }
        }
        return count;
}

// Pixels in `box` that differ between the images.
static int pixelsChanged(const TestImage &a, const TestImage &b, const math::Rect &box)
{
        int count = 0;
        const int x0 = std::max(0, static_cast<int>(std::floor(box.pos.x)) + kMargin);
        const int y0 = std::max(0, static_cast<int>(std::floor(box.pos.y)) + kMargin);
        const int x1 = std::min(a.width, static_cast<int>(std::ceil(box.pos.x + box.size.x)) + kMargin);
        const int y1 = std::min(a.height, static_cast<int>(std::ceil(box.pos.y + box.size.y)) + kMargin);
        for(int y = y0; y < y1; ++y)
        {
                for(int x = x0; x < x1; ++x)
                {
                        if(!samePixel(a, b, x, y))
                        {
                                ++count;
                        }
                }
        }
        return count;
}

static std::string describe(widget::Widget *widget)
{
        int status = 0;
        char *name = abi::__cxa_demangle(typeid(*widget).name(), NULL, NULL, &status);
        const std::string text =
                string::f("%s at (%g, %g)", name ? name : typeid(*widget).name(),
                          static_cast<double>(widget->box.pos.x), static_cast<double>(widget->box.pos.y));
        std::free(name);
        return text;
}

static void checkLayoutAndDrawing(ModuleWidget *widget)
{
        const math::Rect module_box(math::Vec(0.f, 0.f), widget->box.size);
        const TestImage everything = renderTestWidget(widget, kMargin);
        CHECK(pixelsOutside(everything, widget->box.size) == 0);

        widget::Widget *panel = widget->getPanel();
        REQUIRE(panel);
        CHECK(panel->box.pos.x == 0.f);
        CHECK(panel->box.pos.y == 0.f);
        CHECK(panel->box.size.x == widget->box.size.x);
        CHECK(panel->box.size.y == widget->box.size.y);

        for(widget::Widget *child : widget->children)
        {
                CAPTURE(describe(child));
                CHECK(module_box.contains(child->box));
        }

        // Each widget alone on the panel draws something, and nothing outside the module.
        for(widget::Widget *child : widget->children)
        {
                child->visible = child == panel;
        }
        const TestImage bare_panel = renderTestWidget(widget, kMargin);
        TestImage transparent = bare_panel;
        std::fill(transparent.rgba.begin(), transparent.rgba.end(), 0);
        CHECK(pixelsChanged(transparent, bare_panel, module_box) > 0);
        for(widget::Widget *child : widget->children)
        {
                if(child == panel)
                {
                        continue;
                }
                CAPTURE(describe(child));
                child->visible = true;
                const TestImage with_child = renderTestWidget(widget, kMargin);
                child->visible = false;
                CHECK(pixelsChanged(bare_panel, with_child, child->box.grow(math::Vec(1.f, 1.f))) > 0);
                CHECK(pixelsOutside(with_child, widget->box.size) == 0);
        }
        for(widget::Widget *child : widget->children)
        {
                child->visible = true;
        }
}

void checkModuleWidget(const std::string &slug, const std::set<int> &params_without_widgets)
{
        Model *model = findModel(slug);
        REQUIRE(model);
        for(int with_module = 0; with_module < 2; ++with_module)
        {
                CAPTURE(with_module);
                Module *module = NULL;
                if(with_module)
                {
                        module = model->createModule();
                        // The widget deletes its module, which it expects to find in the engine.
                        APP->engine->addModule(module);
                }
                ModuleWidget *widget = model->createModuleWidget(module);
                OrbitsWidget *orbits = dynamic_cast<OrbitsWidget *>(widget);
                REQUIRE(orbits);
                if(module)
                {
                        checkControls(widget, module, params_without_widgets);
                }

                for(size_t theme = 0; theme < kOrbitsThemeNames.size(); ++theme)
                {
                        CAPTURE(kOrbitsThemeNames[theme]);
                        // As chosen from the menu, so step() doesn't switch to the theme Rack's
                        // setting gives.
                        orbits->m_theme_setting.m_follow_rack = false;
                        orbits->m_theme_setting.m_theme = static_cast<OrbitsTheme>(theme);
                        orbits->switchTheme(static_cast<OrbitsTheme>(theme));
                        checkSvgs(widget);
                        checkLayoutAndDrawing(widget);
                }
                delete widget;
        }
}
