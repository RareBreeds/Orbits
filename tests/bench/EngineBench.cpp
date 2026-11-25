// Times the modules' process(), which Rack calls once per sample on the engine thread.

#include "EugeneModule.hpp"
#include "PolygeneModule.hpp"
#include "RackTestHelpers.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

typedef RareBreeds_Orbits_Eugene Eugene;
typedef RareBreeds_Orbits_Polygene Polygene;

// 10 seconds of audio at 48 kHz per timed run. The best and median of the runs are reported: the
// best is the least disturbed by the rest of the system, the median shows how noisy that was.
static const int kSamplesPerRun = 480000;
static const int kRuns = 7;
static const double kWarmUpNs = 200e6;

// Clock periods in samples. 6000 samples is 8 Hz at 48 kHz, 16th notes at 120 BPM. A clock every
// 4 samples makes the work done on each clock edge, such as finding the beat, dominate.
static const int kNormalClockPeriod = 6000;
static const int kFastClockPeriod = 4;

// Connects an input with `channels` channels, all at `voltage`.
static void connectInputAt(Module &module, int input_id, int channels, float voltage)
{
        Input &input = module.getInput(input_id);
        connectInput(input, channels);
        for(int c = 0; c < channels; ++c)
        {
                input.setVoltage(voltage, static_cast<uint8_t>(c));
        }
}

// Runs `samples` samples with a square wave clock of `clock_period` samples on `clock_input`, or
// with no clock if `clock_period` is 0. Returns the time taken in nanoseconds.
static double runSamples(Module &module, int clock_input, int clock_period, int samples)
{
        const Module::ProcessArgs args = testProcessArgs();
        Input &clock = module.getInput(clock_input);
        const int channels = clock.getChannels();
        // The clock only changes twice per period, so setting it costs almost nothing per sample.
        const int half_period = clock_period > 0 ? clock_period / 2 : samples;

        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        bool high = false;
        for(int done = 0; done < samples; done += half_period)
        {
                if(clock_period > 0)
                {
                        high = !high;
                        for(int c = 0; c < channels; ++c)
                        {
                                clock.setVoltage(high ? 10.f : 0.f, static_cast<uint8_t>(c));
                        }
                }
                const int count = std::min(half_period, samples - done);
                for(int sample = 0; sample < count; ++sample)
                {
                        module.process(args);
                }
        }
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::nano>(end - start).count();
}

// Times `module` and prints the best and median nanoseconds per sample.
static void benchmark(const char *name, Module &module, int clock_input, int clock_period)
{
        // Untimed runs first, so the timed runs start with warm caches and the CPU at full speed,
        // which takes a while after it's been idle.
        double warm_up_ns = 0.0;
        while(warm_up_ns < kWarmUpNs)
        {
                warm_up_ns += runSamples(module, clock_input, clock_period, kSamplesPerRun);
        }

        std::vector<double> ns_per_sample;
        for(int run = 0; run < kRuns; ++run)
        {
                ns_per_sample.push_back(runSamples(module, clock_input, clock_period, kSamplesPerRun) / kSamplesPerRun);
        }
        std::sort(ns_per_sample.begin(), ns_per_sample.end());
        std::printf("%-50s %8.1f %8.1f\n", name, ns_per_sample.front(), ns_per_sample[ns_per_sample.size() / 2]);
}

// Connects every CV input of `module` with `channels` channels. Inputs that take triggers or gates
// are held low so they don't fire; the rest get a CV that moves the rhythm away from the knobs.
static void connectAllInputs(Module &module, const int *trigger_inputs, size_t trigger_count, const int *cv_inputs,
                             size_t cv_count, int channels)
{
        for(size_t i = 0; i < trigger_count; ++i)
        {
                connectInputAt(module, trigger_inputs[i], channels, 0.f);
        }
        for(size_t i = 0; i < cv_count; ++i)
        {
                connectInputAt(module, cv_inputs[i], channels, 1.f);
        }
}

static void benchmarkEugene(const char *name, bool all_inputs, int clock_period)
{
        Eugene eugene;
        if(clock_period > 0)
        {
                connectInputAt(eugene, Eugene::CLOCK_INPUT, 1, 0.f);
        }
        if(all_inputs)
        {
                static const int kTriggers[] = {Eugene::SYNC_INPUT, Eugene::REVERSE_CV_INPUT, Eugene::INVERT_CV_INPUT,
                                                Eugene::RANDOM_CV_INPUT};
                static const int kCvs[] = {Eugene::LENGTH_CV_INPUT, Eugene::HITS_CV_INPUT, Eugene::SHIFT_CV_INPUT};
                connectAllInputs(eugene, kTriggers, sizeof(kTriggers) / sizeof(kTriggers[0]), kCvs,
                                 sizeof(kCvs) / sizeof(kCvs[0]), 1);
        }
        benchmark(name, eugene, Eugene::CLOCK_INPUT, clock_period);
}

static void benchmarkPolygene(const char *name, int channels, bool all_inputs, int clock_period)
{
        Polygene polygene;
        if(clock_period > 0)
        {
                connectInputAt(polygene, Polygene::CLOCK_INPUT, channels, 0.f);
        }
        if(all_inputs)
        {
                static const int kTriggers[] = {Polygene::SYNC_INPUT, Polygene::REVERSE_CV_INPUT,
                                                Polygene::INVERT_CV_INPUT, Polygene::RANDOM_CV_INPUT};
                static const int kCvs[] = {Polygene::LENGTH_CV_INPUT, Polygene::HITS_CV_INPUT, Polygene::SHIFT_CV_INPUT,
                                           Polygene::VARIATION_CV_INPUT};
                connectAllInputs(polygene, kTriggers, sizeof(kTriggers) / sizeof(kTriggers[0]), kCvs,
                                 sizeof(kCvs) / sizeof(kCvs[0]), channels);
        }
        benchmark(name, polygene, Polygene::CLOCK_INPUT, clock_period);
}

void runEngineBenchmarks()
{
        std::printf("%-50s %8s %8s\n", "process(), ns per sample", "best", "median");

        benchmarkEugene("Eugene, nothing connected", false, 0);
        benchmarkEugene("Eugene, clocked", false, kNormalClockPeriod);
        benchmarkEugene("Eugene, clocked, all inputs", true, kNormalClockPeriod);
        benchmarkEugene("Eugene, fast clock, all inputs", true, kFastClockPeriod);

        benchmarkPolygene("Polygene, nothing connected", 0, false, 0);
        benchmarkPolygene("Polygene, 1 channel clocked", 1, false, kNormalClockPeriod);
        benchmarkPolygene("Polygene, 16 channels clocked", PORT_MAX_CHANNELS, false, kNormalClockPeriod);
        benchmarkPolygene("Polygene, 16 channels clocked, all inputs", PORT_MAX_CHANNELS, true, kNormalClockPeriod);
        benchmarkPolygene("Polygene, 16 channels fast clock, all inputs", PORT_MAX_CHANNELS, true, kFastClockPeriod);
}
