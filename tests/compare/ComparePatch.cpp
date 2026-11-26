#include "ComparePatch.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstdlib>
#include <cstring>

static const char *const kOrbitsSlug = "RareBreeds_Orbits";
static const char *const kCompareSlug = "OrbitsCompare";

// Enough outputs and inputs to connect to every input and output of the modules under test.
static const int kCompareSourceOutputs = 16;
static const int kCompareRecorderInputs = 4;

// Sends the voltages and clocks that the patch's scenario sets to the module's inputs.
struct CompareSource : Module
{
        ComparePatch *m_patch = NULL;

        CompareSource()
        {
                config(0, 0, kCompareSourceOutputs, 0);
        }

        void process(const ProcessArgs &) override
        {
                // The source is the first module the engine processes, so it counts the samples.
                const int64_t frame = ++m_patch->m_frame;
                for(size_t o = 0; o < m_patch->m_signals.size(); ++o)
                {
                        const ComparePatch::Signal &signal = m_patch->m_signals[o];
                        Output &output = outputs[o];
                        output.setChannels(static_cast<uint8_t>(signal.channels));
                        for(int c = 0; c < signal.channels; ++c)
                        {
                                float voltage = signal.voltage[c];
                                if(signal.period[c] > 0)
                                {
                                        const int64_t phase = (frame - signal.start[c]) % signal.period[c];
                                        voltage = frame >= signal.start[c] && phase < signal.period[c] / 2 ? 10.f : 0.f;
                                }
                                output.setVoltage(voltage, static_cast<uint8_t>(c));
                        }
                }
        }
};

// Writes every change of the module's outputs to the patch's log.
struct CompareRecorder : Module
{
        ComparePatch *m_patch = NULL;

        CompareRecorder()
        {
                config(0, kCompareRecorderInputs, 0, 0);
        }

        void process(const ProcessArgs &) override
        {
                const long long frame = static_cast<long long>(m_patch->m_frame);
                for(size_t i = 0; i < m_patch->m_recorded_channels.size(); ++i)
                {
                        const char *name = m_patch->m_module->outputInfos[i]->name.c_str();
                        const int channels = inputs[i].getChannels();
                        if(channels != m_patch->m_recorded_channels[i])
                        {
                                std::fprintf(m_patch->m_log, "%lld %s channels %d\n", frame, name, channels);
                                m_patch->m_recorded_channels[i] = channels;
                        }

                        std::vector<float> &recorded = m_patch->m_recorded_voltages[i];
                        for(int c = 0; c < channels; ++c)
                        {
                                const float voltage = inputs[i].getVoltage(static_cast<uint8_t>(c));
                                if(voltage != 0.f && voltage != 10.f && m_patch->m_invalid_output.empty())
                                {
                                        m_patch->m_invalid_output = string::f("%s[%d] was %gV on sample %lld", name, c,
                                                                              static_cast<double>(voltage), frame);
                                }
                                if(voltage != recorded[static_cast<size_t>(c)])
                                {
                                        std::fprintf(m_patch->m_log, "%lld %s[%d] %g\n", frame, name, c,
                                                     static_cast<double>(voltage));
                                        recorded[static_cast<size_t>(c)] = voltage;
                                }
                        }
                }
        }
};

static Model *modelCompareSource = createModel<CompareSource, app::ModuleWidget>("Source");
static Model *modelCompareRecorder = createModel<CompareRecorder, app::ModuleWidget>("Recorder");

// Defined in plugin.cpp.
extern "C" void init(Plugin *plugin);

void registerComparePlugins()
{
        // Both builds being compared save the same version, so their patches are the same.
        Plugin *orbits = new Plugin;
        orbits->slug = kOrbitsSlug;
        orbits->version = "compare";
        init(orbits);
        plugin::plugins.push_back(orbits);

        Plugin *compare = new Plugin;
        compare->slug = kCompareSlug;
        compare->version = "compare";
        compare->addModel(modelCompareSource);
        compare->addModel(modelCompareRecorder);
        plugin::plugins.push_back(compare);
}

ComparePatch::ComparePatch(const char *model_slug, std::FILE *log) : m_log(log)
{
        Model *model = plugin::getModel(kOrbitsSlug, model_slug);
        if(!model)
        {
                throw Exception("Unknown model %s", model_slug);
        }

        // The engine processes modules in the order they're added.
        Module *source = modelCompareSource->createModule();
        APP->engine->addModule(source);
        Module *module = model->createModule();
        APP->engine->addModule(module);
        Module *recorder = modelCompareRecorder->createModule();
        APP->engine->addModule(recorder);

        m_signals.resize(module->inputs.size());
        m_input_cable_ids.resize(module->inputs.size(), -1);
        m_recorded_channels.resize(module->outputs.size(), 0);
        m_recorded_voltages.resize(module->outputs.size(), std::vector<float>(PORT_MAX_CHANNELS, 0.f));
        findModules();

        for(size_t o = 0; o < module->outputs.size(); ++o)
        {
                Cable *cable = new Cable;
                cable->outputModule = module;
                cable->outputId = static_cast<int>(o);
                cable->inputModule = recorder;
                cable->inputId = static_cast<int>(o);
                APP->engine->addCable(cable);
        }
}

ComparePatch::~ComparePatch()
{
        APP->engine->clear();
}

void ComparePatch::findModules()
{
        // By model rather than ID, as a patch saved by another build may have other IDs.
        m_module = m_source = m_recorder = NULL;
        const std::vector<int64_t> module_ids = APP->engine->getModuleIds();
        for(int64_t id : module_ids)
        {
                Module *module = APP->engine->getModule(id);
                if(module->model == modelCompareSource)
                {
                        m_source = module;
                }
                else if(module->model == modelCompareRecorder)
                {
                        m_recorder = module;
                }
                else
                {
                        m_module = module;
                }
        }
        if(!m_module || !m_source || !m_recorder)
        {
                throw Exception("A module is missing from the patch");
        }
        static_cast<CompareSource *>(m_source)->m_patch = this;
        static_cast<CompareRecorder *>(m_recorder)->m_patch = this;

        std::fill(m_input_cable_ids.begin(), m_input_cable_ids.end(), -1);
        const std::vector<int64_t> cable_ids = APP->engine->getCableIds();
        for(int64_t id : cable_ids)
        {
                const Cable *cable = APP->engine->getCable(id);
                if(cable->inputModule == m_module)
                {
                        m_input_cable_ids[static_cast<size_t>(cable->inputId)] = id;
                }
        }
}

int ComparePatch::findParam(const char *name) const
{
        for(size_t i = 0; i < m_module->paramQuantities.size(); ++i)
        {
                if(m_module->paramQuantities[i]->name == name)
                {
                        return static_cast<int>(i);
                }
        }
        throw Exception("Unknown param %s", name);
}

int ComparePatch::findInput(const char *name) const
{
        for(size_t i = 0; i < m_module->inputInfos.size(); ++i)
        {
                if(m_module->inputInfos[i]->name == name)
                {
                        return static_cast<int>(i);
                }
        }

        // Eugene's Invert CV input was named "Invert" before v2.0.5.
        if(std::strcmp(name, "Invert CV") == 0)
        {
                return findInput("Invert");
        }
        throw Exception("Unknown input %s", name);
}

void ComparePatch::connect(const char *input, int channels)
{
        note("connect %s with %d channels", input, channels);
        const int id = findInput(input);
        const size_t index = static_cast<size_t>(id);
        m_signals[index] = Signal();
        m_signals[index].channels = channels;
        if(m_input_cable_ids[index] < 0)
        {
                Cable *cable = new Cable;
                cable->outputModule = m_source;
                cable->outputId = id;
                cable->inputModule = m_module;
                cable->inputId = id;
                APP->engine->addCable(cable);
                m_input_cable_ids[index] = cable->id;
        }
}

void ComparePatch::disconnect(const char *input)
{
        note("disconnect %s", input);
        const size_t index = static_cast<size_t>(findInput(input));
        Cable *cable = APP->engine->getCable(m_input_cable_ids[index]);
        if(cable)
        {
                APP->engine->removeCable(cable);
                delete cable;
        }
        m_input_cable_ids[index] = -1;
        m_signals[index] = Signal();
}

void ComparePatch::setVoltage(const char *input, float voltage)
{
        note("%s %gV", input, static_cast<double>(voltage));
        Signal &signal = m_signals[static_cast<size_t>(findInput(input))];
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                signal.voltage[c] = voltage;
                signal.period[c] = 0;
        }
}

void ComparePatch::setVoltage(const char *input, int channel, float voltage)
{
        note("%s[%d] %gV", input, channel, static_cast<double>(voltage));
        Signal &signal = m_signals[static_cast<size_t>(findInput(input))];
        signal.voltage[channel] = voltage;
        signal.period[channel] = 0;
}

void ComparePatch::setClock(const char *input, int period)
{
        note("%s clock every %d samples", input, period);
        Signal &signal = m_signals[static_cast<size_t>(findInput(input))];
        for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                signal.period[c] = period;
                signal.start[c] = m_frame + 1;
        }
}

void ComparePatch::setClock(const char *input, int channel, int period)
{
        note("%s[%d] clock every %d samples", input, channel, period);
        Signal &signal = m_signals[static_cast<size_t>(findInput(input))];
        signal.period[channel] = period;
        signal.start[channel] = m_frame + 1;
}

void ComparePatch::setParam(const char *param, float value)
{
        note("%s %g", param, static_cast<double>(value));
        APP->engine->setParamValue(m_module, findParam(param), value);
}

void ComparePatch::turnKnob(const char *param, float value)
{
        note("turn %s to %g", param, static_cast<double>(value));
        APP->engine->setParamSmoothValue(m_module, findParam(param), value);
}

void ComparePatch::pressButton(const char *param, int samples)
{
        setParam(param, 1.f);
        run(samples);
        setParam(param, 0.f);
}

void ComparePatch::setData(const char *key, json_t *value)
{
        char *dump = json_dumps(value, JSON_ENCODE_ANY);
        note("data %s %s", key, dump);
        std::free(dump);

        json_t *module_json = APP->engine->moduleToJson(m_module);
        json_t *data = json_object_get(module_json, "data");
        if(!data)
        {
                data = json_object();
                json_object_set_new(module_json, "data", data);
        }
        json_object_set_new(data, key, value);
        APP->engine->moduleFromJson(m_module, module_json);
        json_decref(module_json);
}

void ComparePatch::trigger(const char *input, int samples)
{
        setVoltage(input, 10.f);
        run(samples);
        setVoltage(input, 0.f);
}

json_t *ComparePatch::moduleData()
{
        json_t *module_json = APP->engine->moduleToJson(m_module);
        json_t *data = json_incref(json_object_get(module_json, "data"));
        json_decref(module_json);
        return data ? data : json_object();
}

void ComparePatch::setModuleData(json_t *data)
{
        char *dump = json_dumps(data, JSON_ENCODE_ANY | JSON_COMPACT | JSON_SORT_KEYS);
        note("data %s", dump);
        std::free(dump);

        json_t *module_json = APP->engine->moduleToJson(m_module);
        json_object_set_new(module_json, "data", data);
        APP->engine->moduleFromJson(m_module, module_json);
        json_decref(module_json);
}

void ComparePatch::run(int samples)
{
        APP->engine->stepBlock(samples);
        if(!m_invalid_output.empty())
        {
                throw Exception("%s", m_invalid_output.c_str());
        }
}

void ComparePatch::runClocks(int clocks)
{
        run(clocks * kComparePeriod);
}

void ComparePatch::reloadPatch()
{
        note("reload the patch");
        APP->engine->prepareSave();
        json_t *patch = APP->engine->toJson();

        ++m_reloads;
        const std::string suffix = "-" + std::to_string(m_reloads) + ".json";
        if(!m_save_prefix.empty())
        {
                const std::string path = m_save_prefix + suffix;
                if(json_dump_file(patch, path.c_str(), JSON_INDENT(2) | JSON_SORT_KEYS) != 0)
                {
                        throw Exception("Can't write %s", path.c_str());
                }
        }
        if(!m_load_prefix.empty())
        {
                // Load the patch another build saved at the same point instead.
                json_decref(patch);
                const std::string path = m_load_prefix + suffix;
                json_error_t error;
                patch = json_load_file(path.c_str(), 0, &error);
                if(!patch)
                {
                        throw Exception("Can't load %s: %s", path.c_str(), error.text);
                }
        }

        APP->engine->fromJson(patch);
        json_decref(patch);
        findModules();
}

void ComparePatch::reloadPreset()
{
        note("reload the module's settings");
        APP->engine->prepareSaveModule(m_module);
        json_t *module_json = APP->engine->moduleToJson(m_module);
        APP->engine->moduleFromJson(m_module, module_json);
        json_decref(module_json);
}

void ComparePatch::loadPreset(const std::string &name)
{
        note("load preset %s", name.c_str());
        const std::string path = system::join(m_presets_directory, m_module->model->slug, name);
        json_error_t error;
        json_t *module_json = json_load_file(path.c_str(), 0, &error);
        if(!module_json)
        {
                throw Exception("Can't load %s: %s", path.c_str(), error.text);
        }
        // As ModuleWidget::loadFile() does, so the module keeps its ID.
        json_object_del(module_json, "id");
        APP->engine->moduleFromJson(m_module, module_json);
        json_decref(module_json);
}

std::vector<std::string> ComparePatch::presetNames() const
{
        std::vector<std::string> names;
        const std::vector<std::string> paths =
                system::getEntries(system::join(m_presets_directory, m_module->model->slug));
        for(const std::string &path : paths)
        {
                if(system::getExtension(path) == ".vcvm")
                {
                        names.push_back(system::getFilename(path));
                }
        }
        std::sort(names.begin(), names.end());
        if(names.empty())
        {
                // Including when --presets wasn't given.
                throw Exception("No presets for %s in %s", m_module->model->slug.c_str(), m_presets_directory.c_str());
        }
        return names;
}

void ComparePatch::reset()
{
        note("initialize");
        APP->engine->resetModule(m_module);
}

void ComparePatch::randomize()
{
        note("randomize");
        APP->engine->randomizeModule(m_module);
}

void ComparePatch::bypass(bool bypassed)
{
        note(bypassed ? "bypass" : "stop bypassing");
        APP->engine->bypassModule(m_module, bypassed);
}

void ComparePatch::note(const char *format, ...)
{
        std::fprintf(m_log, "%lld ", static_cast<long long>(m_frame));
        va_list args;
        va_start(args, format);
        std::vfprintf(m_log, format, args);
        va_end(args);
        std::fputc('\n', m_log);
}

// Where the scenarios save the patch each time they reload it, or load it from instead of their own.
struct ComparePatchFiles
{
        std::string presets_directory;
        std::string save_directory;
        std::string load_directory;
};

static void runScenario(const CompareScenario &scenario, const std::string &directory, const ComparePatchFiles &files)
{
        const std::string path = directory + "/" + scenario.name + ".txt";
        std::FILE *log = std::fopen(path.c_str(), "w");
        if(!log)
        {
                throw Exception("Can't write %s", path.c_str());
        }
        // The first line is the same for every revision, so the comparison can read it from either.
        if(scenario.expected_difference)
        {
                std::fprintf(log, "# Expected to differ from the last release: %s\n", scenario.expected_difference);
        }
        else
        {
                std::fprintf(log, "# Expected to be the same as the last release\n");
        }

        // Randomizing gives the same results each run, as long as each revision's modules ask for
        // the same random numbers.
        random::local().seed(1, 2);
        setCompareSampleRate(48000.f);
        {
                ComparePatch patch(scenario.model_slug, log);
                patch.m_presets_directory = files.presets_directory;
                if(!files.save_directory.empty())
                {
                        patch.m_save_prefix = files.save_directory + "/" + scenario.name;
                }
                if(!files.load_directory.empty())
                {
                        patch.m_load_prefix = files.load_directory + "/" + scenario.name;
                }
                scenario.run(patch);
                // Every scenario ends by saving and reopening the patch, to check that everything
                // it set up is saved, and loads into other builds.
                patch.reloadPatch();
                patch.runClocks(20);
                patch.note("end");
        }
        std::fclose(log);
}

static void runScenarios(const CompareScenario *scenarios, size_t count, const std::string &directory,
                         const char *filter, const ComparePatchFiles &files)
{
        for(size_t i = 0; i < count; ++i)
        {
                if(std::strstr(scenarios[i].name, filter))
                {
                        runScenario(scenarios[i], directory, files);
                }
        }
}

int runCompareScenarios(int argc, char **argv)
{
        const char *filter = "";
        ComparePatchFiles files;
        bool invalid_patches = false;
        bool usage = argc < 2;
        for(int i = 2; i < argc && !usage; ++i)
        {
                const std::string arg = argv[i];
                if(arg == "--invalid-patches")
                {
                        invalid_patches = true;
                }
                else if(arg == "--presets" && i + 1 < argc)
                {
                        files.presets_directory = argv[++i];
                }
                else if((arg == "--save-patches" || arg == "--load-patches") && i + 1 < argc)
                {
                        (arg == "--save-patches" ? files.save_directory : files.load_directory) = argv[++i];
                }
                else if(arg.compare(0, 2, "--") != 0)
                {
                        filter = argv[i];
                }
                else
                {
                        usage = true;
                }
        }
        if(usage)
        {
                std::fprintf(stderr,
                             "Usage: %s <output directory> [scenario name filter] [--presets <directory>] "
                             "[--save-patches <directory>] [--load-patches <directory>] [--invalid-patches]\n",
                             argv[0]);
                return 1;
        }
        const std::string directory = argv[1];

        try
        {
                registerComparePlugins();
                if(invalid_patches)
                {
                        runScenarios(kInvalidPatchScenarios, kInvalidPatchScenarioCount, directory, filter, files);
                }
                else
                {
                        runScenarios(kEugeneScenarios, kEugeneScenarioCount, directory, filter, files);
                        runScenarios(kPolygeneScenarios, kPolygeneScenarioCount, directory, filter, files);
                }
        }
        catch(Exception &e)
        {
                std::fprintf(stderr, "%s\n", e.what());
                return 1;
        }
        return 0;
}
