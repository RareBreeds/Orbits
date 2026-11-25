// Times drawing the modules' widgets in a Rack window with Rack's OpenGL renderer, as Rack draws
// them each frame.

#include "EugeneModule.hpp"
#include "PolygeneModule.hpp"
#include "RackTestHelpers.hpp"

#include <GL/glew.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>

// Defined in BenchMain.cpp, which can use Rack's internal functions.
void resetFramebufferBudget();

// Several instances of a module are drawn each frame so their cost stands out from the cost of the
// frame itself (clearing it and waiting for the GPU). They're all drawn at the same position: the
// GPU still draws every one.
static const int kInstances = 8;
static const int kFramesPerRun = 120;
// The median of the runs is reported, as drawing times vary more than the engine's and an
// occasional run is much faster than the rest.
static const int kRuns = 7;
static const double kWarmUpNs = 200e6;

struct FrameTimes
{
        // Until nvgEndFrame() returns: building paths, tessellating, text and sending it to the GPU.
        double cpu_ns;
        // Until the GPU has finished drawing too.
        double total_ns;
};

struct DrawResult
{
        double median_cpu_us;
        double median_total_us;
        double best_total_us;
};

struct DrawInstance
{
        Module *module;
        app::ModuleWidget *widget;
};

// Draws `frames` frames of `instances`. If `clocked`, each instance gets one clock pulse before
// each frame, which moves its display on a step. Returns the total time taken.
static FrameTimes runFrames(std::vector<DrawInstance> &instances, int clock_input, bool clocked, int frames)
{
        const Module::ProcessArgs args = testProcessArgs();
        GLFWwindow *win = APP->window->win;
        NVGcontext *vg = APP->window->vg;
        int fb_width, fb_height, win_width, win_height;
        glfwGetFramebufferSize(win, &fb_width, &fb_height);
        glfwGetWindowSize(win, &win_width, &win_height);
        const float pixel_ratio = static_cast<float>(fb_width) / static_cast<float>(win_width);

        FrameTimes times = {0.0, 0.0};
        for(int frame = 0; frame < frames; ++frame)
        {
                // The engine's work isn't timed.
                if(clocked)
                {
                        for(size_t i = 0; i < instances.size(); ++i)
                        {
                                Input &clock = instances[i].module->getInput(clock_input);
                                for(int c = 0; c < clock.getChannels(); ++c)
                                {
                                        clock.setVoltage(10.f, static_cast<uint8_t>(c));
                                }
                                instances[i].module->process(args);
                                for(int c = 0; c < clock.getChannels(); ++c)
                                {
                                        clock.setVoltage(0.f, static_cast<uint8_t>(c));
                                }
                                instances[i].module->process(args);
                        }
                }

                const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
                glViewport(0, 0, fb_width, fb_height);
                glClearColor(0.f, 0.f, 0.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                nvgBeginFrame(vg, static_cast<float>(win_width), static_cast<float>(win_height), pixel_ratio);
                for(size_t i = 0; i < instances.size(); ++i)
                {
                        app::ModuleWidget *widget = instances[i].widget;
                        widget->step();

                        // Rack only re-renders framebuffers while the frame has time left, which this
                        // loop doesn't track, so let every instance's framebuffers re-render.
                        resetFramebufferBudget();

                        widget::Widget::DrawArgs draw_args;
                        draw_args.vg = vg;
                        draw_args.clipBox = math::Rect(math::Vec(0.f, 0.f), widget->box.size);
                        widget->draw(draw_args);
                        widget->drawLayer(draw_args, 1);
                }
                nvgEndFrame(vg);
                const std::chrono::steady_clock::time_point cpu_end = std::chrono::steady_clock::now();
                glFinish();
                const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

                times.cpu_ns += std::chrono::duration<double, std::nano>(cpu_end - start).count();
                times.total_ns += std::chrono::duration<double, std::nano>(end - start).count();
        }
        return times;
}

// Times drawing `kInstances` instances of `model` per frame, or empty frames if `model` is NULL.
// `channels` is the number of clock channels; with 0 the clock isn't connected.
static DrawResult timeDrawing(Model *model, int clock_input, int channels, bool clocked)
{
        std::vector<DrawInstance> instances;
        for(int i = 0; model && i < kInstances; ++i)
        {
                DrawInstance instance;
                instance.module = model->createModule();
                // The widget deletes its module, which it expects to find in the engine.
                APP->engine->addModule(instance.module);
                if(channels > 0)
                {
                        connectInput(instance.module->getInput(clock_input), channels);
                }
                instance.widget = model->createModuleWidget(instance.module);
                instances.push_back(instance);
        }

        // Untimed frames first, so the timed ones start with everything loaded, warm caches and the
        // CPU and GPU at full speed.
        double warm_up_ns = 0.0;
        while(warm_up_ns < kWarmUpNs)
        {
                warm_up_ns += runFrames(instances, clock_input, clocked, kFramesPerRun).total_ns;
        }

        std::vector<double> cpu_us, total_us;
        for(int run = 0; run < kRuns; ++run)
        {
                const FrameTimes times = runFrames(instances, clock_input, clocked, kFramesPerRun);
                cpu_us.push_back(times.cpu_ns / 1000.0 / kFramesPerRun);
                total_us.push_back(times.total_ns / 1000.0 / kFramesPerRun);
        }
        std::sort(cpu_us.begin(), cpu_us.end());
        std::sort(total_us.begin(), total_us.end());

        for(size_t i = 0; i < instances.size(); ++i)
        {
                delete instances[i].widget;
        }

        DrawResult result = {cpu_us[cpu_us.size() / 2], total_us[total_us.size() / 2], total_us.front()};
        return result;
}

// Prints the time per module instance, without the time of an empty frame.
static void printDrawing(const char *name, const DrawResult &result, const DrawResult &empty)
{
        std::printf("%-50s %8.1f %8.1f %8.1f\n", name, (result.median_cpu_us - empty.median_cpu_us) / kInstances,
                    (result.median_total_us - empty.median_total_us) / kInstances,
                    (result.best_total_us - empty.best_total_us) / kInstances);
}

void runDrawBenchmarks()
{
        const int eugene_clock = RareBreeds_Orbits_Eugene::CLOCK_INPUT;
        const int polygene_clock = RareBreeds_Orbits_Polygene::CLOCK_INPUT;

        const DrawResult empty = timeDrawing(NULL, 0, 0, false);
        // The CPU time is until nvgEndFrame() returns, the total includes the GPU.
        std::printf("\n%-50s %8s %8s %8s\n", "drawing, us per module per frame", "cpu", "median", "best");
        std::printf("%-50s %8.1f %8.1f %8.1f\n", "Empty frame (not per module)", empty.median_cpu_us,
                    empty.median_total_us, empty.best_total_us);

        printDrawing("Eugene, display unchanged", timeDrawing(modelRareBreeds_Orbits_Eugene, eugene_clock, 1, false),
                     empty);
        printDrawing("Eugene, display changing", timeDrawing(modelRareBreeds_Orbits_Eugene, eugene_clock, 1, true),
                     empty);
        printDrawing("Polygene, 16 channels, display unchanged",
                     timeDrawing(modelRareBreeds_Orbits_Polygene, polygene_clock, PORT_MAX_CHANNELS, false), empty);
        printDrawing("Polygene, 1 channel, display changing",
                     timeDrawing(modelRareBreeds_Orbits_Polygene, polygene_clock, 1, true), empty);
        printDrawing("Polygene, 16 channels, display changing",
                     timeDrawing(modelRareBreeds_Orbits_Polygene, polygene_clock, PORT_MAX_CHANNELS, true), empty);
}
