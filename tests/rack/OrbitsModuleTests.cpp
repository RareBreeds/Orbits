#include "doctest.h"

#include "OrbitsModule.hpp"
#include "OrbitsSkinned.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

// Parses a JSON value as it might appear in a patch. JSON_DECODE_ANY allows bare values like 3.
static json_t *parseJson(const char *text)
{
        json_t *json = json_loads(text, JSON_DECODE_ANY, NULL);
        REQUIRE(json);
        return json;
}

TEST_CASE("clampRounded rounds to the nearest value in the range")
{
        CHECK(clampRounded(7.4f, 1, 32) == 7);
        CHECK(clampRounded(7.5f, 1, 32) == 8);
        CHECK(clampRounded(0.4f, 1, 32) == 1);
        CHECK(clampRounded(40.f, 1, 32) == 32);
        CHECK(clampRounded(std::numeric_limits<float>::infinity(), 1, 32) == 32);
}

TEST_CASE("clampRounded clamps negative values and NaN to the minimum")
{
        // v2 cast negative values to unsigned int, which on x86 wrapped them round to the maximum.
        CHECK(clampRounded(-0.6f, 1, 32) == 1);
        CHECK(clampRounded(-3.2f, 1, 32) == 1);
        CHECK(clampRounded(-40.f, 0, 16) == 0);
        CHECK(clampRounded(-std::numeric_limits<float>::infinity(), 0, 16) == 0);
        CHECK(clampRounded(std::numeric_limits<float>::quiet_NaN(), 0, 16) == 0);
}

TEST_CASE("EOC and beat mode options keep the order their numbers are saved in")
{
        // Patches store the option's index, so reordering these would change existing patches.
        CHECK(EOCMode().getOptions() == std::vector<std::string>{"On Repeat", "First", "Last"});
        CHECK(BeatMode().getOptions() == std::vector<std::string>{"Pulse", "Gate", "Hold"});
}

template <typename Mode> static void checkModeLoading()
{
        for(uint8_t mode = 0; mode < 3; ++mode)
        {
                Mode loaded;
                json_t *json = json_integer(mode);
                loaded.dataFromJson(json);
                json_decref(json);
                CHECK(loaded.getMode() == mode);
        }

        // Out of range or non-integer values, e.g. from a hand-edited patch or a future version with
        // more modes, keep the current mode. Index 3 used to call through a bad function pointer.
        for(const char *text : {"3", "-1", "256", "1000000000000", "2.0", "\"Gate\"", "null", "true"})
        {
                Mode loaded;
                loaded.setMode(1);
                json_t *json = parseJson(text);
                loaded.dataFromJson(json);
                json_decref(json);
                CAPTURE(text);
                CHECK(loaded.getMode() == 1);
        }

        Mode set;
        set.setMode(2);
        set.setMode(3);
        CHECK(set.getMode() == 2);
}

TEST_CASE("EOC modes load valid values from a patch and ignore invalid ones")
{
        checkModeLoading<EOCMode>();
}

TEST_CASE("Beat modes load valid values from a patch and ignore invalid ones")
{
        checkModeLoading<BeatMode>();
}

TEST_CASE("json_load_real loads real and integer numbers")
{
        json_t *root = parseJson("{\"real\": 16.5, \"integer\": 16}");
        float real = 0.f;
        float integer = 0.f;
        json_load_real(root, "real", &real);
        json_load_real(root, "integer", &integer);
        json_decref(root);

        CHECK(real == 16.5f);
        CHECK(integer == 16.f);
}

TEST_CASE("json_load_real keeps the current value when the value isn't a number")
{
        json_t *root = parseJson("{\"string\": \"16\", \"bool\": true, \"null\": null, \"object\": {}}");
        for(const char *key : {"string", "bool", "null", "object", "missing"})
        {
                float value = 7.5f;
                json_load_real(root, key, &value);
                CAPTURE(key);
                CHECK(value == 7.5f);
        }
        json_decref(root);
}

TEST_CASE("json_load_real keeps values finite")
{
        // An infinite value couldn't be saved again, json_real() rejects it.
        json_t *root = parseJson("{\"big\": 1e300, \"small\": -1e300}");
        float big = 0.f;
        float small = 0.f;
        json_load_real(root, "big", &big);
        json_load_real(root, "small", &small);
        json_decref(root);

        CHECK(big == std::numeric_limits<float>::max());
        CHECK(small == -std::numeric_limits<float>::max());
}

TEST_CASE("Themes are found by their saved names")
{
        // Compared by index because doctest would use the plugin's toString(OrbitsTheme) to print them.
        OrbitsTheme theme = OrbitsTheme::Dark;
        CHECK(fromString("Light", theme));
        CHECK(toIndex(theme) == toIndex(OrbitsTheme::Light));
        CHECK(fromString("Dark", theme));
        CHECK(toIndex(theme) == toIndex(OrbitsTheme::Dark));

        CHECK(std::string(toString(OrbitsTheme::Dark)) == "Dark");
        CHECK(std::string(toString(OrbitsTheme::Light)) == "Light");
}

TEST_CASE("Unknown, empty and missing theme names aren't found")
{
        OrbitsTheme theme = OrbitsTheme::Light;
        CHECK_FALSE(fromString("Purple", theme));
        CHECK_FALSE(fromString("", theme));
        // A patch's theme that isn't a string gives NULL from json_string_value(). This used to crash.
        CHECK_FALSE(fromString(NULL, theme));
        CHECK(toIndex(theme) == toIndex(OrbitsTheme::Light));
}
