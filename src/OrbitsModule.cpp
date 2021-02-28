#include "OrbitsModule.hpp"

static const EOCModeOptions eoc_mode_options;

bool EOCModeOptions::process(int mode, bool is_first, bool is_last, bool prev_was_last) const
{
        return options[mode]->process(is_first, is_last, prev_was_last);
}

std::vector<std::string> EOCModeOptions::getOptions(void) const
{
        std::vector<std::string> opts;
        for(auto option : options)
        {
                opts.push_back(option->desc);
        }
        return opts;
}

EOCModeOptions::~EOCModeOptions()
{
        for(auto i : options)
        {
                delete i;
        }
}

size_t EOCModeOptions::size() const
{
        return options.size();
}

int EOCMode::getMode(void)
{
        return m_mode;
}

void EOCMode::setMode(int mode)
{
        m_mode = math::clamp(mode, 0, eoc_mode_options.size());
}

std::vector<std::string> EOCMode::getOptions(void)
{
        return eoc_mode_options.getOptions();
}

json_t *EOCMode::dataToJson(void)
{
        return json_integer(m_mode);
}

void EOCMode::dataFromJson(json_t *root)
{
        if(root)
        {
                setMode(json_integer_value(root));
        }
}

void EOCGenerator::update(EOCMode &mode, bool is_first, bool is_last)
{
        if(eoc_mode_options.process(mode.getMode(), is_first, is_last, m_previous_beat_was_last))
        {
                m_generator.trigger(1e-3f);
        }
        m_previous_beat_was_last = is_last;
}

bool EOCGenerator::process(float delta)
{
        return m_generator.process(delta);
}

static const BeatModeOptions beat_mode_options;

std::vector<std::string> BeatModeOptions::getOptions(void) const
{
        std::vector<std::string> opts;
        for(auto option : options)
        {
                opts.push_back(option->desc);
        }
        return opts;
}

BeatModeOptions::~BeatModeOptions()
{
        for(auto i : options)
        {
                delete i;
        }
}

size_t BeatModeOptions::size() const
{
        return options.size();
}

bool BeatModeOptions::process(int mode, bool pulse, bool state) const
{
        return options[mode]->process(pulse, state);
}

int BeatMode::getMode(void)
{
        return m_mode;
}

void BeatMode::setMode(int mode)
{
        m_mode = math::clamp(mode, 0, beat_mode_options.size());
}

std::vector<std::string> BeatMode::getOptions(void)
{
        return beat_mode_options.getOptions();
}

json_t *BeatMode::dataToJson(void)
{
        return json_integer(m_mode);
}

void BeatMode::dataFromJson(json_t *root)
{
        if(root)
        {
                setMode(json_integer_value(root));
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
        return beat_mode_options.process(mode.getMode(), pulse, m_state);

        if(mode.getMode() == 0)
        {
                return m_generator.process(delta);
        }
        else if(mode.getMode() == 1)
        {
                if(m_generator.process(delta))
                {
                        return false;
                }
                return m_state;
        }
        else
        {
                return m_state;
        }
}