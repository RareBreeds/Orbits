#pragma once

#include "plugin.hpp"
#include <vector>

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

class EOCModeOptions
{
      public:
        bool process(int mode, bool is_first, bool is_last, bool prev_was_last) const;
        std::vector<std::string> getOptions(void) const;
        ~EOCModeOptions();
        size_t size() const;

      private:
        std::vector<EOCModeOption *> options{new EOCModeOptionOnRepeat, new EOCModeOptionFirst, new EOCModeOptionLast};
};

struct EOCMode
{
        int m_mode = 0;

        int getMode(void);
        void setMode(int mode);
        std::vector<std::string> getOptions(void);
        json_t *dataToJson(void);
        void dataFromJson(json_t *root);
};

struct EOCGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_previous_beat_was_last = false;
        void update(Orbits::EOCMode &mode, bool is_first, bool is_last);
        bool process(float delta);
};
} // namespace Orbits
