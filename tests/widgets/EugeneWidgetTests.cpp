#include "doctest.h"

#include "EugeneModule.hpp"
#include "WidgetTestHelpers.hpp"

TEST_CASE("Eugene's widget has every control, all drawn inside the panel")
{
        checkModuleWidget("RareBreeds_Orbits_Eugene", {RareBreeds_Orbits_Eugene::UNUSED_PARAM});
}
