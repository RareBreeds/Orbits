#pragma once

#include "plugin.hpp"

#include <string>
#include <vector>

// Arguments for running a module one sample at a time at 48 kHz.
inline Module::ProcessArgs testProcessArgs()
{
        Module::ProcessArgs args;
        args.sampleRate = 48000.f;
        args.sampleTime = 1.f / args.sampleRate;
        args.frame = 0;
        return args;
}

// Connects an input as a cable with `channels` channels would. Rack's setChannels() leaves a
// disconnected input disconnected, because only cables connect inputs.
inline void connectInput(Input &input, int channels)
{
        input.channels = static_cast<uint8_t>(channels);
}

// What two outputs did as each clock pulse arrived, per polyphonic channel. Each string has one
// character per clock pulse: 'x' if the output was high, '.' if it was low.
struct ClockedOutputs
{
        std::vector<std::string> beat;
        std::vector<std::string> eoc;
};

// Sends `clocks` clock pulses to `clock_input` on `channels` channels, recording `beat_output` and
// `eoc_output` as each pulse arrives. Between pulses the clock is low for long enough that the
// modules' 1 ms output pulses end.
inline ClockedOutputs sendClocks(Module &module, int clock_input, int beat_output, int eoc_output, int channels,
                                 unsigned int clocks)
{
        Module::ProcessArgs args = testProcessArgs();
        Input &clock = module.getInput(clock_input);
        connectInput(clock, channels);

        // Rack's triggers start high so a clock that's already high when a patch loads doesn't
        // fire. Start with the clock low, as a real clock would be, so the first pulse is seen.
        for(int sample = 0; sample < 10; ++sample)
        {
                module.process(args);
        }

        ClockedOutputs outputs;
        outputs.beat.resize(static_cast<size_t>(channels));
        outputs.eoc.resize(static_cast<size_t>(channels));

        for(unsigned int pulse = 0; pulse < clocks; ++pulse)
        {
                for(int c = 0; c < channels; ++c)
                {
                        clock.setVoltage(10.f, static_cast<uint8_t>(c));
                }
                module.process(args);
                for(int c = 0; c < channels; ++c)
                {
                        const uint8_t channel = static_cast<uint8_t>(c);
                        outputs.beat[channel] += module.getOutput(beat_output).getVoltage(channel) > 5.f ? 'x' : '.';
                        outputs.eoc[channel] += module.getOutput(eoc_output).getVoltage(channel) > 5.f ? 'x' : '.';
                        clock.setVoltage(0.f, channel);
                }

                // 100 samples at 48 kHz is about 2 ms.
                for(int sample = 0; sample < 100; ++sample)
                {
                        module.process(args);
                }
        }
        return outputs;
}

// Clears Rack's undo history. Defined in RackTestMain.cpp, see there for why.
void clearTestUndoHistory();

// Whether a json_t has been freed can't be checked, but its reference count can. Patch loading
// tests hold one reference of their own, so a count above 1 afterwards means a leak.
inline size_t referenceCount(const json_t *json)
{
        return json->refcount;
}
