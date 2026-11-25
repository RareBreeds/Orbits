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
