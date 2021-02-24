#pragma once

#include "plugin.hpp"

namespace Orbits
{
struct EOCModule
{
        int m_mode = 0;

        int getMode(void)
        {
                return m_mode;
        }

        void setMode(int mode)
        {
                m_mode = mode;
        }

        json_t *dataToJson(void)
        {
                return json_integer(m_mode);
        }

        void dataFromJson(json_t *root)
        {
                if(root)
                {
                        m_mode = json_integer_value(root);
                }
        }
};

struct EOCGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_previous_beat_was_last = false;
        void update(int mode, bool is_first, bool is_last)
        {
                if(mode == 0) // on repeat
                {
                        if(is_first && m_previous_beat_was_last)
                        {
                                m_generator.trigger(1e-3f);
                        }
                }
                else if(mode == 1) // first beat
                {
                        if(is_first)
                        {
                                m_generator.trigger(1e-3f);
                        }
                }
                else // last beat
                {
                        if(is_last)
                        {
                                m_generator.trigger(1e-3f);
                        }
                }

                m_previous_beat_was_last = is_last;
        }

        bool process(float delta)
        {
                return m_generator.process(delta);
        }
};
} // namespace Orbits
