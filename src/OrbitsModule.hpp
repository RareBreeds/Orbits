#pragma once

#include "OrbitsMacros.h"
#include "plugin.hpp"
#include <vector>

unsigned int clampRounded(float value, unsigned int min, unsigned int max);
void json_load_real(json_t *root, const char *param, float *result);
void json_load_bool(json_t *root, const char *param, bool *result);
void json_load_unsigned_int(json_t *root, const char *param, unsigned int *result);

struct EOCMode
{
        uint8_t m_mode = 0;

        uint8_t getMode(void);
        void setMode(uint8_t mode);
        std::vector<std::string> getOptions(void);
        json_t *dataToJson(void);
        void dataFromJson(json_t *root);
};

struct EOCGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_previous_beat_was_last = false;
        void update(EOCMode &mode, bool is_first, bool is_last);
        bool process(float delta);
};

struct BeatMode
{
        uint8_t m_mode = 0;

        uint8_t getMode(void);
        void setMode(uint8_t mode);
        std::vector<std::string> getOptions(void);
        json_t *dataToJson(void);
        void dataFromJson(json_t *root);
};

struct BeatGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_state = false;
        void update(bool is_on);
        bool process(BeatMode &mode, float delta);
};

struct RepeatTrigger
{
        const float m_repeat_delay_s = 0.5f;
        const float m_repeat_rate_s = 0.05f;

        dsp::BooleanTrigger m_trigger;
        dsp::PulseGenerator m_repeat_timer;

        bool process(bool state, float sampleTime)
        {
                if(m_trigger.process(state))
                {
                        m_repeat_timer.trigger(m_repeat_delay_s);
                        return true;
                }

                if(state && !m_repeat_timer.process(sampleTime))
                {
                        m_repeat_timer.trigger(m_repeat_rate_s);
                        return true;
                }

                return false;
        }
};
