#include "doctest.h"

#include "OrbitsWidget.hpp"

#include <string>
#include <vector>

// The menu items in a menu, in order. Labels and separators aren't MenuItems so are skipped.
static std::vector<MenuItem *> menuItems(Menu *menu)
{
        std::vector<MenuItem *> items;
        for(widget::Widget *child : menu->children)
        {
                MenuItem *item = dynamic_cast<MenuItem *>(child);
                if(item)
                {
                        items.push_back(item);
                }
        }
        return items;
}

TEST_CASE("The Randomize Filter menu has an item per option that toggles its bit")
{
        unsigned int mask = 0x5; // Options 0 and 2 randomized.
        Menu *menu = new Menu;
        orbitsAppendRandomizeFilterMenu(menu, &mask, {{"Length", 0}, {"Hits", 1}, {"Shift", 2}});

        std::vector<MenuItem *> top = menuItems(menu);
        REQUIRE(top.size() == 1);
        CHECK(top[0]->text == "Randomize Filter");

        // Opening the submenu builds it.
        Menu *submenu = top[0]->createChildMenu();
        REQUIRE(submenu);
        std::vector<MenuItem *> options = menuItems(submenu);
        REQUIRE(options.size() == 3);
        CHECK(options[0]->text == "Length");
        CHECK(options[1]->text == "Hits");
        CHECK(options[2]->text == "Shift");

        // Choosing an item toggles only its bit.
        MenuItem::ActionEvent action;
        options[1]->onAction(action);
        CHECK(mask == 0x7);
        options[0]->onAction(action);
        CHECK(mask == 0x6);
        options[0]->onAction(action);
        CHECK(mask == 0x7);

        delete submenu;
        delete menu;
}

// Restores the light boost setting and the room brightness when a test ends.
struct LightSettingsRestorer
{
        float light_boost = orbitsLightBoost();
        float rack_brightness = settings::rackBrightness;
        ~LightSettingsRestorer()
        {
                orbitsSetLightBoost(light_boost);
                settings::rackBrightness = rack_brightness;
        }
};

static void loadSettings(const char *text)
{
        json_t *root = json_loads(text, 0, NULL);
        REQUIRE(root);
        settingsFromJson(root);
        json_decref(root);
}

TEST_CASE("The light boost setting is saved and loaded")
{
        LightSettingsRestorer restorer;
        orbitsSetLightBoost(0.25f);
        json_t *saved = settingsToJson();
        REQUIRE(saved);

        orbitsSetLightBoost(1.f);
        settingsFromJson(saved);
        CHECK(orbitsLightBoost() == doctest::Approx(0.25f));
        json_decref(saved);
}

TEST_CASE("Invalid light boost settings are ignored")
{
        LightSettingsRestorer restorer;
        orbitsSetLightBoost(0.5f);

        loadSettings("{}");
        CHECK(orbitsLightBoost() == 0.5f);
        loadSettings("{\"light_boost\": \"bright\"}");
        CHECK(orbitsLightBoost() == 0.5f);
        loadSettings("{\"light_boost\": 1.5}");
        CHECK(orbitsLightBoost() == 0.5f);
        loadSettings("{\"light_boost\": -0.5}");
        CHECK(orbitsLightBoost() == 0.5f);

        // Integers are numbers too, e.g. from a hand-edited settings file.
        loadSettings("{\"light_boost\": 0}");
        CHECK(orbitsLightBoost() == 0.f);
}

TEST_CASE("Lights are as bright as the room plus the boost, up to full brightness")
{
        LightSettingsRestorer restorer;
        settings::rackBrightness = 0.2f;

        orbitsSetLightBoost(0.f);
        CHECK(orbitsLightBrightness() == doctest::Approx(0.2f));
        orbitsSetLightBoost(0.3f);
        CHECK(orbitsLightBrightness() == doctest::Approx(0.5f));
        orbitsSetLightBoost(1.f);
        CHECK(orbitsLightBrightness() == doctest::Approx(1.f));

        // No brighter than full.
        settings::rackBrightness = 0.9f;
        orbitsSetLightBoost(0.3f);
        CHECK(orbitsLightBrightness() == doctest::Approx(1.f));

        // In the dark, only the boost.
        settings::rackBrightness = 0.f;
        CHECK(orbitsLightBrightness() == doctest::Approx(0.3f));
}

TEST_CASE("Displays are tinted by the light brightness")
{
        LightSettingsRestorer restorer;
        settings::rackBrightness = 0.2f;
        orbitsSetLightBoost(0.3f);
        const NVGcolor tint = orbitsLightTint();
        CHECK(tint.r == doctest::Approx(0.5f));
        CHECK(tint.g == tint.r);
        CHECK(tint.b == tint.r);
        CHECK(tint.a == 1.f);
}

TEST_CASE("The light boost setting is clamped")
{
        LightSettingsRestorer restorer;
        orbitsSetLightBoost(2.f);
        CHECK(orbitsLightBoost() == 1.f);
        orbitsSetLightBoost(-1.f);
        CHECK(orbitsLightBoost() == 0.f);
}
