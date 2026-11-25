#include "PolygeneModule.hpp"
#include "OrbitsRhythm.hpp"
#include "PolygeneWidget.hpp"

#include <algorithm>

// Undo/redo for one channel randomized with the Random button.
struct RandomizeChannelAction : rack::history::ModuleAction
{
        uint8_t channel = 0;
        PolygeneChannelState new_state, old_state;

        void apply(const PolygeneChannelState &state)
        {
                RareBreeds_Orbits_Polygene *polygene =
                        static_cast<RareBreeds_Orbits_Polygene *>(APP->engine->getModule(moduleId));
                if(polygene && channel < PORT_MAX_CHANNELS)
                {
                        polygene->m_channels[channel].m_state = state;
                        polygene->syncParamsToActiveChannel();
                }
        }

        void undo() override
        {
                apply(old_state);
        }

        void redo() override
        {
                apply(new_state);
        }

        RandomizeChannelAction()
        {
                name = "randomize channel";
        }
};

void RareBreeds_Orbits_Polygene::Channel::init(RareBreeds_Orbits_Polygene *module, uint8_t channel)
{
        m_current_step = 0;
        m_module = module;
        m_channel = channel;
        m_state.length = m_module->getParam(LENGTH_KNOB_PARAM).getValue();
        m_state.hits = m_module->getParam(HITS_KNOB_PARAM).getValue();
        m_state.shift = m_module->getParam(SHIFT_KNOB_PARAM).getValue();
        m_state.variation = m_module->getParam(VARIATION_KNOB_PARAM).getValue();
        m_state.reverse = false;
        m_state.invert = false;
}

bool RareBreeds_Orbits_Polygene::Channel::readReverse(void)
{
        if(m_module->getInput(REVERSE_CV_INPUT).isConnected())
        {
                m_reverse_trigger.process(m_module->getInput(REVERSE_CV_INPUT).getPolyVoltage(m_channel));
                return m_reverse_trigger.isHigh();
        }
        else
        {
                return m_state.reverse;
        }
}

bool RareBreeds_Orbits_Polygene::Channel::readInvert(void)
{
        if(m_module->getInput(INVERT_CV_INPUT).isConnected())
        {
                m_invert_trigger.process(m_module->getInput(INVERT_CV_INPUT).getPolyVoltage(m_channel));
                return m_invert_trigger.isHigh();
        }
        else
        {
                return m_state.invert;
        }
}

bool RareBreeds_Orbits_Polygene::Channel::isOnBeat(unsigned int length, unsigned int hits, unsigned int shift,
                                                   unsigned int variation, unsigned int beat, bool invert)
{
        return orbitsNearEvenBeat(length, hits, variation, shift, beat) != invert;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readLength()
{
        auto cv = m_module->getParameterizedVoltage(LENGTH_CV_INPUT, m_channel) / 5.f;
        auto f_length = m_state.length + cv * (kOrbitsRhythmMaxLength - 1);
        return clampRounded(f_length, 1, kOrbitsRhythmMaxLength);
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readHits(unsigned int length)
{
        auto cv = m_module->getParameterizedVoltage(HITS_CV_INPUT, m_channel) / 5.f;
        auto f_hits = m_state.hits + cv;
        return clampRounded(f_hits * static_cast<float>(length), 0, length);
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readShift(unsigned int length)
{
        auto cv = m_module->getParameterizedVoltage(SHIFT_CV_INPUT, m_channel) / 5.f;
        auto f_shift = m_state.shift + cv * (kOrbitsRhythmMaxLength - 1);
        return clampRounded(f_shift, 0, kOrbitsRhythmMaxLength - 1) % length;
}

unsigned int RareBreeds_Orbits_Polygene::Channel::readVariation(unsigned int length, unsigned int hits)
{
        auto cv = m_module->getParameterizedVoltage(VARIATION_CV_INPUT, m_channel) / 5.f;
        auto f_variation = m_state.variation + cv;
        auto count = orbitsNearEvenRhythmCount(length, hits);
        return clampRounded(f_variation * (static_cast<float>(count - 1)), 0, count - 1);
}

void RareBreeds_Orbits_Polygene::Channel::process(const ProcessArgs &args)
{
        // A rising clock edge means first play the current beat
        // then advance to the next step
        if(m_clock_trigger.process(m_module->getInput(CLOCK_INPUT).getPolyVoltage(m_channel)))
        {
                // Play the current beat
                auto length = readLength();
                auto hits = readHits(length);
                auto shift = readShift(length);
                auto invert = m_invert.load(std::memory_order_relaxed);
                auto variation = readVariation(length, hits);
                auto reverse = m_reverse.load(std::memory_order_relaxed);

                // Avoid stepping out of bounds
                m_current_step = readStep(length);

                if(reverse)
                {
                        if(m_current_step == 0)
                        {
                                m_current_step = length - 1;
                        }
                        else
                        {
                                --m_current_step;
                        }
                }

                unsigned int first = reverse ? length - 1 : 0;
                unsigned int last = reverse ? 0 : length - 1;
                m_eoc_generator.update(m_module->m_eoc, m_current_step == first, m_current_step == last);

                m_beat_generator.update(isOnBeat(length, hits, shift, variation, m_current_step, invert));

                if(!reverse)
                {
                        if(m_current_step == length - 1)
                        {
                                m_current_step = 0;
                        }
                        else
                        {
                                ++m_current_step;
                        }
                }
        }

        auto out = m_beat_generator.process(m_module->m_beat, args.sampleTime) ? 10.f : 0.f;
        m_module->getOutput(BEAT_OUTPUT).setVoltage(out, m_channel);

        auto eoc_out = m_eoc_generator.process(args.sampleTime) ? 10.f : 0.f;
        m_module->getOutput(EOC_OUTPUT).setVoltage(eoc_out, m_channel);
}

json_t *RareBreeds_Orbits_Polygene::Channel::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "length", json_real(m_state.length));
                json_object_set_new(root, "hits", json_real(m_state.hits));
                json_object_set_new(root, "shift", json_real(m_state.shift));
                json_object_set_new(root, "variation", json_real(m_state.variation));
                json_object_set_new(root, "reverse", json_boolean(m_state.reverse));
                json_object_set_new(root, "invert", json_boolean(m_state.invert));
        }
        return root;
}

void RareBreeds_Orbits_Polygene::Channel::dataFromJson(json_t *root)
{
        if(root)
        {
                json_load_real(root, "length", &m_state.length);
                json_load_real(root, "hits", &m_state.hits);
                json_load_real(root, "shift", &m_state.shift);
                json_load_real(root, "variation", &m_state.variation);
                json_load_bool(root, "reverse", &m_state.reverse);
                json_load_bool(root, "invert", &m_state.invert);
        }
}

void RareBreeds_Orbits_Polygene::Channel::onRandomizeWithHistory(unsigned int randomization_mask)
{
        PolygeneRandomizeRecord record;
        record.channel = m_channel;
        record.old_state = m_state;
        onRandomize(randomization_mask);
        record.new_state = m_state;
        m_module->m_randomize_undo.post(record);
}

void RareBreeds_Orbits_Polygene::Channel::onRandomize(unsigned int randomization_mask)
{
        if(randomization_mask & (1 << RANDOMIZE_LENGTH))
        {
                m_state.length = random::uniform() * kOrbitsRhythmMaxLength;
        }

        if(randomization_mask & (1 << RANDOMIZE_HITS))
        {
                m_state.hits = random::uniform();
        }

        if(randomization_mask & (1 << RANDOMIZE_SHIFT))
        {
                m_state.shift = random::uniform() * (kOrbitsRhythmMaxLength - 1);
        }

        if(randomization_mask & (1 << RANDOMIZE_VARIATION))
        {
                m_state.variation = random::uniform();
        }

        if(randomization_mask & (1 << RANDOMIZE_REVERSE))
        {
                m_state.reverse = (random::uniform() < 0.5f);
        }

        if(randomization_mask & (1 << RANDOMIZE_INVERT))
        {
                m_state.invert = (random::uniform() < 0.5f);
        }
}

PolygeneDisplayData RareBreeds_Orbits_Polygene::getDisplayData(void)
{
        PolygeneDisplayData data;
        data.active_channel_id = m_active_channel_id;
        data.active_channels = m_active_channels;
        for(uint8_t c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                auto ch = &data.channels[c];
                ch->length = m_channels[c].readLength();
                ch->hits = m_channels[c].readHits(ch->length);
                ch->shift = m_channels[c].readShift(ch->length);
                ch->variation = m_channels[c].readVariation(ch->length, ch->hits);
                ch->invert = m_channels[c].m_invert.load(std::memory_order_relaxed);
                ch->current_step = m_channels[c].readStep(ch->length);
        }
        return data;
}

PolygeneDisplayData RareBreeds_Orbits_Polygene::getDisplayData(RareBreeds_Orbits_Polygene *module)
{
        if(module)
        {
                return module->getDisplayData();
        }
        else
        {
                // Example rhythms shown in the module browser, repeated if there are more
                // channels than examples.
                static const PolygeneChannelDisplayData examples[] = {
                        {32, 6, 0, 0, 0, 4},   {16, 9, 3, 0, 0, 0},    {7, 3, 6, 0, 0, 5},    {9, 7, 2, 0, 0, 3},
                        {17, 4, 12, 0, 0, 14}, {30, 20, 15, 0, 0, 20}, {19, 4, 14, 0, 0, 10}, {4, 2, 3, 0, 0, 2},
                        {7, 1, 0, 0, 0, 6},    {8, 5, 3, 0, 0, 1},     {16, 10, 4, 0, 0, 4},  {19, 16, 2, 0, 0, 18},
                        {24, 20, 8, 0, 0, 14}, {28, 14, 6, 0, 0, 20},  {21, 12, 4, 0, 0, 4},  {9, 3, 3, 0, 0, 7},
                };
                const int num_examples = static_cast<int>(sizeof(examples) / sizeof(examples[0]));

                PolygeneDisplayData data;
                data.active_channel_id = static_cast<uint8_t>(std::min(3, PORT_MAX_CHANNELS - 1));
                data.active_channels = PORT_MAX_CHANNELS;
                for(int c = 0; c < PORT_MAX_CHANNELS; ++c)
                {
                        data.channels[c] = examples[c % num_examples];
                }

                return data;
        }
}

RareBreeds_Orbits_Polygene::RareBreeds_Orbits_Polygene()
{
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(CHANNEL_KNOB_PARAM, 0.f, static_cast<float>(PORT_MAX_CHANNELS - 1), 0.f, "Channel", "", 0.f, 1.f,
                    1.f);
        configParam(LENGTH_KNOB_PARAM, 1.f, kOrbitsRhythmMaxLength, kOrbitsRhythmMaxLength, "Length");
        configParam(HITS_KNOB_PARAM, 0.f, 1.f, 0.5f, "Hits", "%", 0.f, 100.f);
        configParam(SHIFT_KNOB_PARAM, 0.f, kOrbitsRhythmMaxLength - 1, 0.f, "Shift");
        configParam(VARIATION_KNOB_PARAM, 0.f, 1.f, 0.0f, "Variation", "%", 0.f, 100.f);
        configSwitch(REVERSE_KNOB_PARAM, 0.f, 1.f, 0.f, "Reverse", {"Off", "On"});
        configSwitch(INVERT_KNOB_PARAM, 0.f, 1.f, 0.f, "Invert", {"Off", "On"});
        configButton(RANDOM_KNOB_PARAM, "Randomize channel");
        configButton(SYNC_KNOB_PARAM, "Sync");

        configInput(CLOCK_INPUT, "Clock");
        configInput(SYNC_INPUT, "Sync");
        configInput(LENGTH_CV_INPUT, "Length CV");
        configInput(HITS_CV_INPUT, "Hits CV");
        configInput(SHIFT_CV_INPUT, "Shift CV");
        configInput(VARIATION_CV_INPUT, "Variation CV");
        configInput(REVERSE_CV_INPUT, "Reverse CV");
        configInput(INVERT_CV_INPUT, "Invert CV");
        configInput(RANDOM_CV_INPUT, "Random CV");

        configOutput(BEAT_OUTPUT, "Beat");
        configOutput(EOC_OUTPUT, "End of cycle");

        configBypass(CLOCK_INPUT, BEAT_OUTPUT);

        for(int i = 0; i < NUM_INPUTS; ++i)
        {
                m_input_mode[i] = INPUT_MODE_MONOPHONIC_COPIES_TO_ALL;
        }

        reset();
}

void RareBreeds_Orbits_Polygene::reset()
{
        m_previous_channel_id = PORT_MAX_CHANNELS;
        m_active_channel_id = 0;
        m_active_channel = &m_channels[m_active_channel_id];

        for(uint8_t i = 0; i < PORT_MAX_CHANNELS; ++i)
        {
                m_channels[i].init(this, i);
        }
}

void RareBreeds_Orbits_Polygene::syncParamsToActiveChannel()
{
        getParamQuantity(LENGTH_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.length);
        getParamQuantity(HITS_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.hits);
        getParamQuantity(SHIFT_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.shift);
        getParamQuantity(VARIATION_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.variation);
        getParamQuantity(REVERSE_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.reverse);
        getParamQuantity(INVERT_KNOB_PARAM)->setImmediateValue(m_active_channel->m_state.invert);
}

InputMode RareBreeds_Orbits_Polygene::getInputMode(int input_id)
{
        return m_input_mode[input_id];
}

float RareBreeds_Orbits_Polygene::getParameterizedVoltage(int input_id, uint8_t channel)
{
        Input &input = getInput(input_id);
        switch(getInputMode(input_id))
        {
        case INPUT_MODE_MONOPHONIC_COPIES_TO_ALL:
                return input.getNormalPolyVoltage(0.0f, channel);
        case INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST:
        default:
                return input.getNormalVoltage(0.0f, channel);
        }
}

void RareBreeds_Orbits_Polygene::process(const ProcessArgs &process_args)
{
        m_active_channels = static_cast<uint8_t>(getInput(CLOCK_INPUT).getChannels());
        getOutput(BEAT_OUTPUT).setChannels(m_active_channels);
        getOutput(EOC_OUTPUT).setChannels(m_active_channels);

        m_active_channel_id =
                static_cast<uint8_t>(clampRounded(getParam(CHANNEL_KNOB_PARAM).getValue(), 0, PORT_MAX_CHANNELS - 1));
        m_active_channel = &m_channels[m_active_channel_id];

        // Update the knob positions when the channel changes
        if(m_previous_channel_id != m_active_channel_id)
        {
                syncParamsToActiveChannel();
                m_previous_channel_id = m_active_channel_id;
        }

        m_active_channel->m_state.length = getParam(LENGTH_KNOB_PARAM).getValue();
        m_active_channel->m_state.hits = getParam(HITS_KNOB_PARAM).getValue();
        m_active_channel->m_state.shift = getParam(SHIFT_KNOB_PARAM).getValue();
        m_active_channel->m_state.variation = getParam(VARIATION_KNOB_PARAM).getValue();

        m_reverse_trigger.process(getParam(REVERSE_KNOB_PARAM).getValue() > 0.5f);
        m_active_channel->m_state.reverse = m_reverse_trigger.state;

        m_invert_trigger.process(getParam(INVERT_KNOB_PARAM).getValue() > 0.5f);
        m_active_channel->m_state.invert = m_invert_trigger.state;

        bool rnd = getParam(RANDOM_KNOB_PARAM).getValue() > 0.5f;
        if(m_random_trigger.process(rnd, process_args.sampleTime))
        {
                m_active_channel->onRandomizeWithHistory(m_randomization_mask);
                syncParamsToActiveChannel();
        }

        for(uint8_t i = 0; i < PORT_MAX_CHANNELS; ++i)
        {
                if(m_channels[i].m_random_trigger.process(getParameterizedVoltage(RANDOM_CV_INPUT, i)))
                {
                        // CV randomization is not a user edit, so it is not added to the undo history.
                        m_channels[i].onRandomize(m_randomization_mask);
                        syncParamsToActiveChannel();
                }
        }

        for(uint8_t i = 0; i < PORT_MAX_CHANNELS; ++i)
        {
                if(m_channels[i].m_sync_trigger.process(getParameterizedVoltage(SYNC_INPUT, i)))
                {
                        m_channels[i].m_current_step = 0;
                }
        }

        if(m_sync_trigger.process(getParam(SYNC_KNOB_PARAM).getValue() > 0.5f))
        {
                for(auto &chan : m_channels)
                {
                        chan.m_current_step = 0;
                }
        }

        for(auto &chan : m_channels)
        {
                chan.m_reverse.store(chan.readReverse(), std::memory_order_relaxed);
                chan.m_invert.store(chan.readInvert(), std::memory_order_relaxed);
        }

        uint8_t clock_channels = static_cast<uint8_t>(getInput(CLOCK_INPUT).getChannels());
        for(uint8_t i = 0; i < clock_channels; ++i)
        {
                m_channels[i].process(process_args);
        }
}

static InputMode loadInputMode(json_t *root, const char *key)
{
        json_t *obj = json_object_get(root, key);
        if(json_is_integer(obj))
        {
                json_int_t mode = json_integer_value(obj);
                if(mode == INPUT_MODE_MONOPHONIC_COPIES_TO_ALL || mode == INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST)
                {
                        return static_cast<InputMode>(mode);
                }
        }

        return INPUT_MODE_MONOPHONIC_COPIES_TO_ALL;
}

json_t *RareBreeds_Orbits_Polygene::dataToJson()
{
        json_t *root = json_object();
        if(root)
        {
                json_object_set_new(root, "beat", m_beat.dataToJson());
                json_object_set_new(root, "eoc", m_eoc.dataToJson());

                json_object_set_new(root, "sync_cv", json_integer(m_input_mode[SYNC_INPUT]));
                json_object_set_new(root, "length_cv", json_integer(m_input_mode[LENGTH_CV_INPUT]));
                json_object_set_new(root, "hits_cv", json_integer(m_input_mode[HITS_CV_INPUT]));
                json_object_set_new(root, "shift_cv", json_integer(m_input_mode[SHIFT_CV_INPUT]));
                json_object_set_new(root, "variation_cv", json_integer(m_input_mode[VARIATION_CV_INPUT]));
                json_object_set_new(root, "random_cv", json_integer(m_input_mode[RANDOM_CV_INPUT]));

                json_object_set_new(root, "randomization_mask", json_integer(m_randomization_mask));

                json_object_set_new(root, "active_channel_id", json_integer(m_active_channel_id));

                json_t *channels = json_array();
                if(channels)
                {
                        for(auto &chan : m_channels)
                        {
                                json_t *channel_json = chan.dataToJson();
                                if(channel_json)
                                {
                                        json_array_append_new(channels, channel_json);
                                }
                        }

                        json_object_set_new(root, "channels", channels);
                }

                if(m_widget)
                {
                        json_t *w = m_widget->dataToJson();
                        if(w)
                        {
                                json_object_set_new(root, "widget", w);
                        }
                }
        }

        return root;
}

void RareBreeds_Orbits_Polygene::dataFromJson(json_t *root)
{
        if(root)
        {
                m_beat.dataFromJson(json_object_get(root, "beat"));
                m_eoc.dataFromJson(json_object_get(root, "eoc"));

                // v2.0.0 added "sync" to allow selection of the sync CV behavior when
                // the cable is monophonic.
                //
                // SYNC_MODE_INDIVIDUAL_CHANNELS 0 only first channel is synced [default]
                // SYNC_MODE_ALL_CHANNELS        1 all channels synced
                //
                // This is backwards to the v2.0.4 behavior. The default there is to
                // use VCV racks voltage standards, where mono cables have channel 1 copied
                // to all other channels (INPUT_MODE_MONOPHONIC_COPIES_TO_ALL). Then
                // optionally you can make it only control the first channel using
                // INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST.
                //
                // So that patches saved with v2.0.0 behave the same with newer versions we
                // use a new key name "sync_cv" and map the old value to the new one.
                json_t *obj = json_object_get(root, "sync");
                if(obj)
                {
                        uint8_t old_sync = static_cast<uint8_t>(json_integer_value(obj));
                        if(old_sync == /*SYNC_MODE_INDIVIDUAL_CHANNELS*/ 0)
                        {
                                m_input_mode[SYNC_INPUT] = INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST;
                        }
                        else
                        {
                                m_input_mode[SYNC_INPUT] = INPUT_MODE_MONOPHONIC_COPIES_TO_ALL;
                        }
                }
                else
                {
                        m_input_mode[SYNC_INPUT] = loadInputMode(root, "sync_cv");
                }

                m_input_mode[LENGTH_CV_INPUT] = loadInputMode(root, "length_cv");
                m_input_mode[HITS_CV_INPUT] = loadInputMode(root, "hits_cv");
                m_input_mode[SHIFT_CV_INPUT] = loadInputMode(root, "shift_cv");
                m_input_mode[VARIATION_CV_INPUT] = loadInputMode(root, "variation_cv");
                // Not saved by v2.0.4 and earlier, so those patches get the default.
                m_input_mode[RANDOM_CV_INPUT] = loadInputMode(root, "random_cv");

                m_randomization_mask = RANDOMIZE_ALL;
                json_load_unsigned_int(root, "randomization_mask", &m_randomization_mask);

                json_t *active_channel = json_object_get(root, "active_channel_id");
                if(json_is_integer(active_channel))
                {
                        json_int_t channel_id = json_integer_value(active_channel);
                        if(channel_id >= 0 && channel_id < PORT_MAX_CHANNELS)
                        {
                                m_active_channel_id = static_cast<uint8_t>(channel_id);
                        }
                }
                json_t *channels = json_object_get(root, "channels");
                if(channels)
                {
                        for(uint8_t i = 0; i < PORT_MAX_CHANNELS; ++i)
                        {
                                json_t *channel = json_array_get(channels, i);
                                if(channel)
                                {
                                        m_channels[i].dataFromJson(channel);
                                }
                        }
                }

                obj = json_object_get(root, "widget");
                if(obj)
                {
                        // Always defer the widget config to the widget draw method for 2 reasons
                        // 1. The widget isn't created before dataFromJson is called in Rack-2
                        //    so store the config and initialize when it's first used.
                        // 2. EightFaceMk2 calls dataFromJson from a different thread than the
                        //    main thread which may cause GL operations to fail.
                        json_decref(m_widget_config.exchange(json_incref(obj)));
                }

                syncParamsToActiveChannel();
        }
}

void RareBreeds_Orbits_Polygene::onRandomize(const RandomizeEvent & /*e*/)
{
        for(auto &chan : m_channels)
        {
                chan.onRandomize(m_randomization_mask);
        }

        // Update the parameters so they reflect the active channels randomized parameters
        syncParamsToActiveChannel();
}

void RareBreeds_Orbits_Polygene::flushUndoHistory()
{
        PolygeneRandomizeRecord record;
        while(m_randomize_undo.pop(record))
        {
                RandomizeChannelAction *action = new RandomizeChannelAction;
                action->moduleId = id;
                action->channel = record.channel;
                action->old_state = record.old_state;
                action->new_state = record.new_state;
                orbitsPushUndoAction(action);
        }
}

void RareBreeds_Orbits_Polygene::onReset(const ResetEvent &e)
{
        Module::onReset(e);
        reset();
}

RareBreeds_Orbits_Polygene::~RareBreeds_Orbits_Polygene()
{
        json_decref(m_widget_config);
}
