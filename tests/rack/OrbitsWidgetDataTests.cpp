#include "doctest.h"

#include "OrbitsWidgetData.hpp"
#include "RackTestHelpers.hpp"

// Pointer comparisons are in double parentheses because doctest can't print json_t pointers.

// A module's saved data with a "widget" entry, as Rack would pass it to dataFromJson().
static json_t *moduleDataWithWidget(json_t *widget)
{
        json_t *root = json_object();
        json_object_set(root, "widget", widget);
        return root;
}

TEST_CASE("Loaded widget data is kept until the widget takes it")
{
        json_t *widget = json_object();
        json_t *root = moduleDataWithWidget(widget);
        {
                OrbitsWidgetData data;
                data.load(root);
                json_decref(root);

                json_t *taken = data.takeLoaded();
                CHECK((taken == widget));
                json_decref(taken);
                CHECK((data.takeLoaded() == NULL));
        }
        CHECK(referenceCount(widget) == 1);
        json_decref(widget);
}

TEST_CASE("Only the most recently loaded widget data is kept")
{
        json_t *first = json_object();
        json_t *second = json_object();
        json_t *first_root = moduleDataWithWidget(first);
        json_t *second_root = moduleDataWithWidget(second);
        {
                OrbitsWidgetData data;
                data.load(first_root);
                data.load(second_root);
                CHECK(referenceCount(first) == 2);

                json_t *taken = data.takeLoaded();
                CHECK((taken == second));
                json_decref(taken);
        }
        json_decref(first_root);
        json_decref(second_root);
        CHECK(referenceCount(first) == 1);
        CHECK(referenceCount(second) == 1);
        json_decref(first);
        json_decref(second);
}

TEST_CASE("Widget data that's never taken is released with the module")
{
        // E.g. headless Rack, or a module deleted before its widget draws.
        json_t *widget = json_object();
        json_t *root = moduleDataWithWidget(widget);
        {
                OrbitsWidgetData data;
                data.load(root);
        }
        json_decref(root);
        CHECK(referenceCount(widget) == 1);
        json_decref(widget);
}

TEST_CASE("Module data without a widget entry leaves nothing to take")
{
        json_t *root = json_object();
        OrbitsWidgetData data;
        data.load(root);
        CHECK((data.takeLoaded() == NULL));
        json_decref(root);
}

TEST_CASE("Without a widget, nothing is saved for it")
{
        // In headless Rack, or before the widget is created.
        json_t *root = json_object();
        OrbitsWidgetData data;
        data.save(root);
        CHECK((json_object_get(root, "widget") == NULL));
        json_decref(root);
}
