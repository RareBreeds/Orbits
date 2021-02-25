#pragma once

#include <vector>
#include "plugin.hpp"

namespace Orbits
{
struct EOCModeOption
{
        const char *desc;
        EOCModeOption(const char *_desc) : desc{_desc}
        {
        }

        virtual bool process(bool is_first, bool is_last, bool prev_was_last) const = 0;

        virtual ~EOCModeOption()
        {
        }
};

struct EOCModeOptionOnRepeat : EOCModeOption
{
        EOCModeOptionOnRepeat() : EOCModeOption("On Repeat")
        {
        }

        virtual ~EOCModeOptionOnRepeat() override
        {
        }

        bool process(bool is_first, bool is_last, bool prev_was_last) const override
        {
                return is_first && prev_was_last;
        }
};

struct EOCModeOptionFirst : EOCModeOption
{
        EOCModeOptionFirst() : EOCModeOption("First")
        {
        }

        virtual ~EOCModeOptionFirst() override
        {
        }

        bool process(bool is_first, bool is_last, bool prev_was_last) const override
        {
                return is_first;
        }
};

struct EOCModeOptionLast : EOCModeOption
{
        EOCModeOptionLast() : EOCModeOption("Last")
        {
        }

        bool process(bool is_first, bool is_last, bool prev_was_last) const override
        {
                return is_last;
        }
};

// TODO: put in cpp file
class EOCModeOptions
{
      public:
        bool process(int mode, bool is_first, bool is_last, bool prev_was_last) const
        {
                return options[mode]->process(is_first, is_last, prev_was_last);
        }
        std::vector<std::string> getOptions(void) const
        {
                std::vector<std::string> opts;
                for(auto option : options)
                {
                        opts.push_back(option->desc);
                }
                return opts;
        }
        ~EOCModeOptions()
        {
                for(auto i : options)
                {
                        delete i;
                }
        }

        size_t size() const
        {
            return options.size();
        }

      private:
        std::vector<EOCModeOption *> options{new EOCModeOptionOnRepeat, new EOCModeOptionFirst, new EOCModeOptionLast};
};

static const EOCModeOptions eoc_mode_options;

struct EOCMode
{
        int m_mode = 0;

        int getMode(void)
        {
                return m_mode;
        }

        void setMode(int mode)
        {
                m_mode = math::clamp(mode, 0, eoc_mode_options.size());
        }

        std::vector<std::string> getOptions(void)
        {
                return eoc_mode_options.getOptions();
        }

        json_t *dataToJson(void)
        {
                return json_integer(m_mode);
        }

        void dataFromJson(json_t *root)
        {
                if(root)
                {
                        setMode(json_integer_value(root));
                }
        }
};

struct EOCGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_previous_beat_was_last = false;
        void update(int mode, bool is_first, bool is_last)
        {
                if(eoc_mode_options.process(mode, is_first, is_last, m_previous_beat_was_last))
                {
                        m_generator.trigger(1e-3f);
                }
                m_previous_beat_was_last = is_last;
        }

        bool process(float delta)
        {
                return m_generator.process(delta);
        }
};
} // namespace Orbits
