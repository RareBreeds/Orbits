#include "EugeneWidget.hpp"
#include "EugeneComponentPositions.h"
#include "EugeneModule.hpp"
#include "EugeneThemes.h"
#include "OrbitsSkinned.hpp"

// Draws the display, for EugeneRhythmDisplay to show.
struct EugeneRhythmDisplayUnbuffered : Widget
{
        EugeneDisplayData m_data{};
        EugeneRhythmDisplayUnbuffered(Vec size);
        void draw(const DrawArgs &args) override;
};

EugeneRhythmDisplayUnbuffered::EugeneRhythmDisplayUnbuffered(Vec size)
{
        box.pos = Vec(0.0, 0.0);
        box.size = size;
}

void EugeneRhythmDisplayUnbuffered::draw(const DrawArgs &args)
{
        const auto foreground_color = color::WHITE;

        nvgSave(args.vg);

        // Everything drawn stays inside the box, so there's no scissor.
        // On MetaModule a scissor makes every path slower to draw.
        const Rect b = Rect(Vec(0, 0), box.size);

        // Everything drawn after here is in the foreground colour
        nvgStrokeColor(args.vg, foreground_color);
        nvgFillColor(args.vg, foreground_color);

        // Translate so (0, 0) is the center of the screen
        nvgTranslate(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Draw length text center bottom and hits text center top
        nvgBeginPath(args.vg);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgFontSize(args.vg, 20);
        std::shared_ptr<Font> font =
                APP->window->loadFont(asset::plugin(pluginInstance, ORBITS_ASSET_FONTS_SHARETECHMONO_REGULAR_TTF));
        nvgFontFaceId(args.vg, font->handle);
        nvgText(args.vg, 0.f, -7.f, std::to_string(m_data.hits).c_str(), NULL);
        nvgText(args.vg, 0.f, 7.f, std::to_string(m_data.length).c_str(), NULL);
        // nvgFill(args.vg);

        // Scale to [-1, 1]
        nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Flip x and y so we start at the top and positive angle increments go clockwise
        nvgScale(args.vg, -1.f, -1.f);

        // nvgBeginPath(args.vg);
        if(m_data.reverse)
        {
                nvgMoveTo(args.vg, 0.23f, -0.2f);
                nvgLineTo(args.vg, 0.3f, 0.f);
                nvgLineTo(args.vg, 0.23f, 0.2f);
        }
        else
        {
                nvgMoveTo(args.vg, -0.23f, 0.2f);
                nvgLineTo(args.vg, -0.3f, -0.f);
                nvgLineTo(args.vg, -0.23f, -0.2f);
        }
        nvgFill(args.vg);

        // set the on beat radius so 8 can fit on the screen
        const auto on_radius = 1.f / 8.f;

        // set outline radius so it's bigger than the on_radius
        const auto outline_radius = on_radius * 1.3f;

        // set the off beat radius so it's almost a dot
        const auto off_radius = on_radius / 10.f;

        // Placement of the inner ring of circles when there's too many of them for one ring
        const auto inner_ring_scale = 0.75f;

        // Width of the line when drawing circles
        const auto circle_stroke_width = 0.03f;
        nvgStrokeWidth(args.vg, circle_stroke_width);

        // Add a border of half a circle so we don't draw over the edge
        nvgScale(args.vg, 1.f - outline_radius, 1.f - outline_radius);

        // Everything is in the foreground colour, so the steps' circles are drawn as one path, then
        // the current beat marker and the shift pointer, rather than a path for every step. Points
        // are transformed as they're added to a path, so each step can still be rotated into place.
        const float mpif = static_cast<float>(M_PI);
        const float lenf = static_cast<float>(m_data.length);
        const unsigned int length = m_data.length;
        auto step_y = [length, inner_ring_scale](unsigned int k)
        { return (length > 16 && k % 2) ? inner_ring_scale : 1.f; };
        auto rotate_to_step = [&args, mpif, lenf](unsigned int k)
        { nvgRotate(args.vg, 2.f * static_cast<float>(k) * mpif / lenf); };

        nvgBeginPath(args.vg);
        for(unsigned int k = 0; k < m_data.length; ++k)
        {
                float radius = off_radius;
                if(orbitsEuclideanBeat(m_data.length, m_data.hits, m_data.shift, k, m_data.invert))
                {
                        radius = on_radius;
                }

                nvgSave(args.vg);
                rotate_to_step(k);
                nvgCircle(args.vg, 0.f, step_y(k), radius);
                nvgRestore(args.vg);
        }
        if(m_data.invert)
        {
                nvgStroke(args.vg);
        }
        else
        {
                nvgFill(args.vg);
        }

        // Current beat marker
        // Highlight the beat that has just played
        // For forward moving rhythms this is the previous index
        // For reversed rhythms this is index
        nvgBeginPath(args.vg);
        for(unsigned int k = 0; k < m_data.length; ++k)
        {
                if((m_data.reverse && m_data.current_step == k) ||
                   (!m_data.reverse &&
                    ((k == m_data.length - 1) ? (0 == m_data.current_step) : (k + 1 == m_data.current_step))))
                {
                        nvgSave(args.vg);
                        rotate_to_step(k);
                        nvgCircle(args.vg, 0.f, step_y(k), outline_radius);
                        nvgRestore(args.vg);
                }
        }
        nvgStroke(args.vg);

        // shift pointer
        if(m_data.shift < m_data.length)
        {
                nvgSave(args.vg);
                rotate_to_step(m_data.shift);
                nvgBeginPath(args.vg);
                nvgMoveTo(args.vg, 0.f, 0.5f);
                nvgLineTo(args.vg, -0.05f, 0.4f);
                nvgLineTo(args.vg, 0.05f, 0.4f);
                nvgClosePath(args.vg);
                nvgFill(args.vg);
                nvgRestore(args.vg);
        }

        nvgRestore(args.vg);
}

struct EugeneRhythmDisplay : OrbitsDisplay
{
        EugeneRhythmDisplayUnbuffered *m_ub = NULL;
        RareBreeds_Orbits_Eugene *m_module = NULL;

        EugeneRhythmDisplay(RareBreeds_Orbits_Eugene *module, Vec pos, Vec size);
        void step() override;
};

EugeneRhythmDisplay::EugeneRhythmDisplay(RareBreeds_Orbits_Eugene *module, Vec pos, Vec size)
        : OrbitsDisplay(pos, size), m_module(module)
{
        m_ub = new EugeneRhythmDisplayUnbuffered(size);
        setContent(m_ub);
}

void EugeneRhythmDisplay::step()
{
        EugeneDisplayData data = RareBreeds_Orbits_Eugene::getDisplayData(m_module);
        if(data != m_ub->m_data)
        {
                m_ub->m_data = data;
                setDirty();
        }

        OrbitsDisplay::step();
}

#define EUGENE_POS(NAME) ORBITS_POS(EUGENE_##NAME)
#define EUGENE_SIZE(NAME) ORBITS_SIZE(EUGENE_##NAME)
#define EUGENE_KNOB(NAME) orbitsKnobConfig(EUGENE_DARK_##NAME, EUGENE_LIGHT_##NAME, EUGENE_POS(NAME))
#define EUGENE_SWITCH(NAME)                                                                                            \
        orbitsSwitchConfig(EUGENE_DARK_##NAME##_ON, EUGENE_LIGHT_##NAME##_ON, EUGENE_DARK_##NAME##_OFF,                \
                           EUGENE_LIGHT_##NAME##_OFF, EUGENE_POS(NAME))
#define EUGENE_PORT(NAME) orbitsPortConfig(EUGENE_DARK_##NAME, EUGENE_LIGHT_##NAME, EUGENE_POS(NAME))

static const OrbitsSkinnedPanelConfig g_config = {{EUGENE_DARK_PANEL, EUGENE_LIGHT_PANEL}};
RareBreeds_Orbits_EugeneWidget::RareBreeds_Orbits_EugeneWidget(RareBreeds_Orbits_Eugene *eugene)
        : OrbitsWidget(g_config)
{
        using Eugene = RareBreeds_Orbits_Eugene;

        setModule(eugene);

        // Module may be NULL if this is the module selection screen
        if(eugene)
        {
                attachWidgetData(&eugene->m_widget_data);
                beat_widget.m_module = &eugene->m_beat;
                eoc_widget.m_module = &eugene->m_eoc;
        }

        OrbitsSkinnedScrewConfig screw;
        screw.imagePaths = {EUGENE_DARK_SCREW, EUGENE_LIGHT_SCREW};
        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(EUGENE_KNOB(LENGTH_KNOB), eugene,
                                                                   Eugene::LENGTH_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(EUGENE_KNOB(HITS_KNOB), eugene, Eugene::HITS_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(EUGENE_KNOB(SHIFT_KNOB), eugene,
                                                                   Eugene::SHIFT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(EUGENE_KNOB(LENGTH_CV_KNOB), eugene,
                                                             Eugene::LENGTH_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(EUGENE_KNOB(HITS_CV_KNOB), eugene,
                                                             Eugene::HITS_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(EUGENE_KNOB(SHIFT_CV_KNOB), eugene,
                                                             Eugene::SHIFT_CV_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(EUGENE_SWITCH(REVERSE_SWITCH), eugene,
                                                               Eugene::REVERSE_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(EUGENE_SWITCH(INVERT_SWITCH), eugene,
                                                               Eugene::INVERT_KNOB_PARAM));
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedButton>(EUGENE_SWITCH(SYNC_BUTTON), eugene,
                                                               Eugene::SYNC_KNOB_PARAM));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(CLOCK_PORT), eugene, Eugene::CLOCK_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(SYNC_PORT), eugene, Eugene::SYNC_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(LENGTH_CV_PORT), eugene, Eugene::LENGTH_CV_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(HITS_CV_PORT), eugene, Eugene::HITS_CV_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(SHIFT_CV_PORT), eugene, Eugene::SHIFT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(REVERSE_CV_PORT), eugene, Eugene::REVERSE_CV_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(INVERT_CV_PORT), eugene, Eugene::INVERT_CV_INPUT));
        addInput(createOrbitsSkinnedInput(EUGENE_PORT(RANDOM_CV_PORT), eugene, Eugene::RANDOM_CV_INPUT));
        addOutput(createOrbitsSkinnedOutput(EUGENE_PORT(BEAT_PORT), eugene, Eugene::BEAT_OUTPUT));
        addOutput(createOrbitsSkinnedOutput(EUGENE_PORT(EOC_PORT), eugene, Eugene::EOC_OUTPUT));

        rhythm_display = new EugeneRhythmDisplay(eugene, EUGENE_POS(DISPLAY), EUGENE_SIZE(DISPLAY));
        addChild(rhythm_display);

        loadInitialTheme();
}

void RareBreeds_Orbits_EugeneWidget::appendModuleContextMenu(Menu *menu)
{
        beat_widget.appendContextMenu(menu);
        eoc_widget.appendContextMenu(menu);

        RareBreeds_Orbits_Eugene *eugene = static_cast<RareBreeds_Orbits_Eugene *>(module);
        orbitsAppendRandomizeFilterMenu(menu, &eugene->m_randomization_mask,
                                        {{"Length", RareBreeds_Orbits_Eugene::RANDOMIZE_LENGTH},
                                         {"Length CV", RareBreeds_Orbits_Eugene::RANDOMIZE_LENGTH_CV},
                                         {"Hits", RareBreeds_Orbits_Eugene::RANDOMIZE_HITS},
                                         {"Hits CV", RareBreeds_Orbits_Eugene::RANDOMIZE_HITS_CV},
                                         {"Shift", RareBreeds_Orbits_Eugene::RANDOMIZE_SHIFT},
                                         {"Shift CV", RareBreeds_Orbits_Eugene::RANDOMIZE_SHIFT_CV},
                                         {"Reverse", RareBreeds_Orbits_Eugene::RANDOMIZE_REVERSE},
                                         {"Invert", RareBreeds_Orbits_Eugene::RANDOMIZE_INVERT}});
}
