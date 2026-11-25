#include "doctest.h"

#include "RackTestHelpers.hpp"

#include <cmath>
#include <string>
#include <vector>

// The plugin's res/ directory, set by the Makefile.
#ifndef ORBITS_TEST_RES_DIR
#error ORBITS_TEST_RES_DIR must be defined
#endif

static std::vector<std::string> svgPaths()
{
        std::vector<std::string> paths;
        const std::vector<std::string> entries = system::getEntries(ORBITS_TEST_RES_DIR);
        for(const std::string &entry : entries)
        {
                if(system::getExtension(entry) == ".svg")
                {
                        paths.push_back(entry);
                }
        }
        return paths;
}

static bool isFinite(const float *values, int count)
{
        for(int i = 0; i < count; ++i)
        {
                if(!std::isfinite(values[i]))
                {
                        return false;
                }
        }
        return true;
}

// An empty SVG, e.g. from a wrong path, makes the framebuffer it's drawn into NaN-sized (f705bba).
// nanosvg loads some malformed SVGs too, so `make -C tests svg` checks that they're well-formed.
TEST_CASE("Every SVG loads with a size and finite shapes")
{
        const std::vector<std::string> paths = svgPaths();
        REQUIRE(!paths.empty());
        for(const std::string &path : paths)
        {
                CAPTURE(path);
                window::Svg svg;
                REQUIRE_NOTHROW(svg.loadFile(path));
                const NSVGimage *image = svg.handle;
                CHECK(std::isfinite(image->width));
                CHECK(std::isfinite(image->height));
                CHECK(image->width > 0.f);
                CHECK(image->height > 0.f);
                REQUIRE(image->shapes != NULL);
                for(const NSVGshape *shape = image->shapes; shape; shape = shape->next)
                {
                        CHECK(isFinite(shape->bounds, 4));
                        for(const NSVGpath *svg_path = shape->paths; svg_path; svg_path = svg_path->next)
                        {
                                CHECK(isFinite(svg_path->pts, svg_path->npts * 2));
                        }
                }
        }
}
