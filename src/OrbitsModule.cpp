#include "OrbitsModule.hpp"

struct EOCModeOption
{
        const char *name;
        bool (*process)(bool is_first, bool is_last, bool prev_was_last);
};

static bool eocModeOnRepeat(bool is_first, bool /*is_last*/, bool prev_was_last)
{
        return is_first && prev_was_last;
}

static bool eocModeFirst(bool is_first, bool /*is_last*/, bool /*prev_was_last*/)
{
        return is_first;
}

static bool eocModeLast(bool /*is_first*/, bool is_last, bool /*prev_was_last*/)
{
        return is_last;
}

static constexpr EOCModeOption eoc_mode_options[] =
{
        {"On Repeat", eocModeOnRepeat},
        {"First", eocModeFirst},
        {"Last", eocModeLast},
};

static constexpr size_t eoc_mode_count = sizeof(eoc_mode_options) / sizeof(eoc_mode_options[0]);

uint8_t EOCMode::getMode(void)
{
        return m_mode;
}

void EOCMode::setMode(uint8_t mode)
{
        m_mode = static_cast<uint8_t>(math::clamp(mode, 0, eoc_mode_count));
}

std::vector<std::string> EOCMode::getOptions(void)
{
        std::vector<std::string> names;
        names.reserve(eoc_mode_count);

        for (const EOCModeOption &option : eoc_mode_options)
        {
                names.emplace_back(option.name);
        }

        return names;
}

json_t *EOCMode::dataToJson(void)
{
        return json_integer(m_mode);
}

void EOCMode::dataFromJson(json_t *root)
{
        if(root)
        {
                uint8_t mode = static_cast<uint8_t>(json_integer_value(root));
                setMode(mode);
        }
}

void EOCGenerator::update(EOCMode &mode, bool is_first, bool is_last)
{
        if(eoc_mode_options[mode.getMode()].process(is_first, is_last, m_previous_beat_was_last))
        {
                m_generator.trigger(1e-3f);
        }
        m_previous_beat_was_last = is_last;
}

bool EOCGenerator::process(float delta)
{
        return m_generator.process(delta);
}

struct BeatModeOption
{
        const char *name;
        bool (*process)(bool pulse, bool state);
};

static bool beatModePulse(bool pulse, bool /*state*/)
{
        return pulse;
}

static bool beatModeGate(bool pulse, bool state)
{
        if(pulse)
        {
                return false;
        }
        return state;
}

static bool beatModeHold(bool /*pulse*/, bool state)
{
        return state;
}

static constexpr BeatModeOption beat_mode_options[] =
{
        {"Pulse", beatModePulse},
        {"Gate", beatModeGate},
        {"Hold", beatModeHold},
};

static constexpr size_t beat_mode_count = sizeof(beat_mode_options) / sizeof(beat_mode_options[0]);

uint8_t BeatMode::getMode(void)
{
        return m_mode;
}

void BeatMode::setMode(uint8_t mode)
{
        m_mode = static_cast<uint8_t>(math::clamp(mode, 0, beat_mode_count));
}

std::vector<std::string> BeatMode::getOptions(void)
{
        std::vector<std::string> names;
        names.reserve(eoc_mode_count);

        for (const BeatModeOption &option : beat_mode_options)
        {
                names.emplace_back(option.name);
        }

        return names;
}

json_t *BeatMode::dataToJson(void)
{
        return json_integer(m_mode);
}

void BeatMode::dataFromJson(json_t *root)
{
        if(root)
        {
                uint8_t mode = static_cast<uint8_t>(json_integer_value(root));
                setMode(mode);
        }
}

void BeatGenerator::update(bool is_on)
{
        m_state = is_on;
        if(is_on)
        {
                m_generator.trigger();
        }
}

bool BeatGenerator::process(BeatMode &mode, float delta)
{
        bool pulse = m_generator.process(delta);
        return beat_mode_options[mode.getMode()].process(pulse, m_state);
}
