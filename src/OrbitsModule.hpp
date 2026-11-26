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

        uint8_t getMode() const;
        void setMode(uint8_t mode);
        std::vector<std::string> getOptions() const;
        json_t *dataToJson() const;
        void dataFromJson(json_t *root);
};

struct EOCGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_previous_beat_was_last = false;
        void update(const EOCMode &mode, bool is_first, bool is_last);
        bool process(float delta);
};

struct BeatMode
{
        uint8_t m_mode = 0;

        uint8_t getMode() const;
        void setMode(uint8_t mode);
        std::vector<std::string> getOptions() const;
        json_t *dataToJson() const;
        void dataFromJson(json_t *root);
};

struct BeatGenerator
{
        dsp::PulseGenerator m_generator;
        bool m_state = false;
        void update(bool is_on);
        bool process(const BeatMode &mode, float delta);
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

// What's played with a knob at `value`, including any CV.
unsigned int orbitsLength(float value);
unsigned int orbitsHitCount(float value, unsigned int length);
unsigned int orbitsShift(float value, unsigned int length);
unsigned int orbitsVariation(float value, unsigned int count);

// CV in each knob's units, for the knobs' displays.
struct OrbitsKnobInputs
{
        virtual float readLengthCV() = 0;
        virtual float readHitsCV() = 0;
        virtual float readShiftCV() = 0;
        virtual unsigned int readLength() = 0;
        virtual unsigned int readHits(unsigned int length) = 0;
        virtual ~OrbitsKnobInputs()
        {
        }
};

// A knob shown, and typed, as the whole number it plays.
struct OrbitsPlayedQuantity : ParamQuantity
{
        OrbitsKnobInputs *m_inputs = NULL;

        virtual unsigned int readPlayed(float value) = 0;
        virtual float readKnobValue(unsigned int played) = 0;
        virtual std::string formatPlayed(unsigned int played);

        std::string getDisplayValueString() override;
        void setDisplayValueString(std::string s) override;
};

struct OrbitsLengthQuantity : OrbitsPlayedQuantity
{
        unsigned int readPlayed(float value) override;
        float readKnobValue(unsigned int played) override;
};

struct OrbitsShiftQuantity : OrbitsPlayedQuantity
{
        unsigned int readPlayed(float value) override;
        float readKnobValue(unsigned int played) override;
};

// e.g. "5 / 16"
struct OrbitsHitsQuantity : OrbitsPlayedQuantity
{
        unsigned int readPlayed(float value) override;
        float readKnobValue(unsigned int played) override;
        std::string formatPlayed(unsigned int played) override;
};
