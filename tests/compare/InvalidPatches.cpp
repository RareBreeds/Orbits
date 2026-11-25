// Patches with invalid settings, e.g. hand-edited, corrupted or saved by other versions, must load
// without crashing, undefined behaviour or outputs other than 0V and 10V. These run in the working
// tree's sanitizer build only, as older versions crash on some of them, and their logs aren't
// compared.

#include "ComparePatch.hpp"

#include <string>

static json_t *junk(CompareRandom &random)
{
        switch(random.next(12))
        {
        case 0:
                return json_null();
        case 1:
                return json_true();
        case 2:
                return json_false();
        case 3:
                return json_string("junk");
        case 4:
                return json_integer(-1);
        case 5:
                return json_integer(1000000);
        case 6:
                return json_integer(static_cast<json_int_t>(1) << 40);
        case 7:
                return json_real(-1e30);
        case 8:
                return json_real(1e30);
        case 9:
                return json_real(0.5);
        case 10:
                return json_object();
        default:
                return json_array();
        }
}

// Replaces or removes a random value in `container`, or in an object or array inside it, or adds
// to it.
static void mutate(json_t *container, CompareRandom &random, int depth)
{
        if(json_is_object(container))
        {
                const int size = static_cast<int>(json_object_size(container));
                if(size == 0 || random.next(10) == 0)
                {
                        json_object_set_new(container, "junk", junk(random));
                        return;
                }
                const int index = random.next(size);
                int i = 0;
                const char *found = NULL;
                const char *key;
                json_t *value;
                json_object_foreach(container, key, value)
                {
                        if(i++ == index)
                        {
                                found = key;
                                break;
                        }
                }
                const std::string name = found;
                json_t *child = json_object_get(container, name.c_str());
                if(depth < 3 && (json_is_object(child) || json_is_array(child)) && random.next(2))
                {
                        mutate(child, random, depth + 1);
                }
                else if(random.next(5) == 0)
                {
                        json_object_del(container, name.c_str());
                }
                else
                {
                        json_object_set_new(container, name.c_str(), junk(random));
                }
        }
        else if(json_is_array(container))
        {
                const int size = static_cast<int>(json_array_size(container));
                if(size == 0 || random.next(10) == 0)
                {
                        // Sometimes more entries than there are channels.
                        const int count = 1 + random.next(20);
                        json_t *entry = size > 0 ? json_array_get(container, 0) : NULL;
                        for(int c = 0; c < count; ++c)
                        {
                                json_array_append_new(container, entry ? json_deep_copy(entry) : junk(random));
                        }
                        return;
                }
                const size_t index = static_cast<size_t>(random.next(size));
                json_t *child = json_array_get(container, index);
                if(depth < 3 && (json_is_object(child) || json_is_array(child)) && random.next(2))
                {
                        mutate(child, random, depth + 1);
                }
                else if(random.next(5) == 0)
                {
                        json_array_remove(container, index);
                }
                else
                {
                        json_array_set_new(container, index, junk(random));
                }
        }
}

static void invalidSettings(ComparePatch &patch)
{
        // Every input connected, with assorted voltages.
        CompareRandom random;
        static const float voltages[] = {-10.f, -1.f, 0.f, 0.5f, 2.f, 10.f};
        for(size_t i = 0; i < patch.m_module->inputInfos.size(); ++i)
        {
                const char *name = patch.m_module->inputInfos[i]->name.c_str();
                patch.connect(name, 4);
                for(int c = 0; c < 4; ++c)
                {
                        patch.setVoltage(name, c, voltages[random.next(6)]);
                }
        }
        patch.setClock("Clock", kComparePeriod);

        json_t *valid = NULL;
        for(int i = 0; i < 400; ++i)
        {
                // Start from different valid settings now and then.
                if(i % 50 == 0)
                {
                        patch.randomize();
                        patch.run(10);
                        json_decref(valid);
                        valid = patch.moduleData();
                }
                json_t *data = json_deep_copy(valid);
                const int mutations = 1 + random.next(3);
                for(int m = 0; m < mutations; ++m)
                {
                        mutate(data, random, 0);
                }
                patch.setModuleData(data);
                patch.run(1 + random.next(2 * kComparePeriod));
                if(i % 50 == 25)
                {
                        patch.reloadPatch();
                }
        }
        json_decref(valid);
}

const CompareScenario kInvalidPatchScenarios[] = {
        {"eugene-invalid-settings", "RareBreeds_Orbits_Eugene", NULL, invalidSettings},
        {"polygene-invalid-settings", "RareBreeds_Orbits_Polygene", NULL, invalidSettings},
};
const size_t kInvalidPatchScenarioCount = sizeof(kInvalidPatchScenarios) / sizeof(kInvalidPatchScenarios[0]);
