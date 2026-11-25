#include "PolygeneWidget.hpp"
#include "OrbitsAssets.h"
#include "OrbitsSkinned.hpp"
#include "PolygeneComponentPositions.h"
#include "PolygeneModule.hpp"
#include "PolygeneThemes.h"

struct PolygeneRhythmDisplayConfig
{
        std::array<NVGcolor, orbitsThemeIndex(OrbitsTheme::Count)> accent;
};

// Draws the display, for PolygeneRhythmDisplay to show.
struct PolygeneRhythmDisplayUnbuffered : Widget
{
        PolygeneDisplayData m_data{};
        NVGcolor m_display_accent{};

        explicit PolygeneRhythmDisplayUnbuffered(Vec size);
        void draw(const DrawArgs &args) override;
};

PolygeneRhythmDisplayUnbuffered::PolygeneRhythmDisplayUnbuffered(Vec size)
{
        box.pos = Vec(0.0, 0.0);
        box.size = size;
}

struct PolygeneRhythmDisplay : OrbitsDisplay, OrbitsSkinned
{
        RareBreeds_Orbits_Polygene *m_module = NULL;
        PolygeneRhythmDisplayConfig m_config{};
        PolygeneRhythmDisplayUnbuffered *m_ub = NULL;

        PolygeneRhythmDisplay(RareBreeds_Orbits_Polygene *module, const PolygeneRhythmDisplayConfig &config, Vec pos,
                              Vec size);
        void setAccent(NVGcolor accent);
        void step() override;
        void loadTheme(OrbitsTheme theme) override;
};

PolygeneRhythmDisplay::PolygeneRhythmDisplay(RareBreeds_Orbits_Polygene *module,
                                             const PolygeneRhythmDisplayConfig &config, Vec pos, Vec size)
        : OrbitsDisplay(pos, size), m_module(module), m_config(config)
{
        m_ub = new PolygeneRhythmDisplayUnbuffered(size);
        setContent(m_ub);
        setAccent(m_config.accent[orbitsThemeIndex(kOrbitsThemeDefault)]);
}

void PolygeneRhythmDisplay::setAccent(NVGcolor accent)
{
        m_ub->m_display_accent = accent;
        setDirty();
}

void PolygeneRhythmDisplay::loadTheme(OrbitsTheme theme)
{
#if ORBITS_THEME_SELECTION_ENABLED
        setAccent(m_config.accent[orbitsThemeIndex(theme)]);
#else
        UNUSED(theme);
#endif
}

void PolygeneRhythmDisplay::step()
{
        PolygeneDisplayData data = RareBreeds_Orbits_Polygene::getDisplayData(m_module);
        if(data != m_ub->m_data)
        {
                m_ub->m_data = data;
                setDirty();
        }

        OrbitsDisplay::step();
}

// The gap between a channel's dashes, in radians
static const float kBeatGap = 0.06f;

// The angle step `k` of a channel's ring starts at, so step 0 is at the top
static float stepAngle(const PolygeneChannelDisplayData &channel, unsigned int k)
{
        const float pi2_len = 2.0f * static_cast<float>(M_PI) / static_cast<float>(channel.length);
        return static_cast<float>(k) * pi2_len + static_cast<float>(M_PI_2);
}

// Adds a dash for each of a channel's beats to the current path
static void addBeatArcs(NVGcontext *vg, const PolygeneChannelDisplayData &channel, float radius)
{
        const float pi2_len = 2.0f * static_cast<float>(M_PI) / static_cast<float>(channel.length);
        const float len = pi2_len - kBeatGap;
        for(unsigned int k = 0; k < channel.length; ++k)
        {
                if(RareBreeds_Orbits_Polygene::Channel::isOnBeat(channel.length, channel.hits, channel.shift,
                                                                 channel.variation, k, channel.invert))
                {
                        const float a0 = stepAngle(channel, k);
                        // Start a new subpath, or nvgArc() joins the dash to the previous one
                        nvgMoveTo(vg, radius * cosf(a0), radius * sinf(a0));
                        nvgArc(vg, 0.0f, 0.0f, radius, a0, a0 + len, NVG_CW);
                }
        }
}

// Adds a dot just before a channel's current step to the current path
static void addCurrentStepDot(NVGcontext *vg, const PolygeneChannelDisplayData &channel, float radius, float dot_radius)
{
        // The engine is run in a different thread and only wraps the step position to a new length on
        // the next clock, so getDisplayData() shows a position beyond the length as 0.
        if(channel.current_step < channel.length)
        {
                const float center = stepAngle(channel, channel.current_step) - kBeatGap / 2.f;
                nvgCircle(vg, radius * cosf(center), radius * sinf(center), dot_radius);
        }
}

void PolygeneRhythmDisplayUnbuffered::draw(const DrawArgs &args)
{
        const PolygeneDisplayData &data = m_data;

        const auto foreground_color = color::WHITE;
        nvgStrokeColor(args.vg, foreground_color);
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
        std::shared_ptr<Font> font =
                APP->window->loadFont(asset::plugin(pluginInstance, ORBITS_ASSET_FONTS_SHARETECHMONO_REGULAR_TTF));
        if(font)
        {
                nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
                nvgFontSize(args.vg, 18);

                nvgFontFaceId(args.vg, font->handle);

                const auto active_hits = data.channels[data.active_channel_id].hits;
                nvgText(args.vg, 0.f, -6.f, std::to_string(active_hits).c_str(), NULL);

                const auto active_length = data.channels[data.active_channel_id].length;
                nvgText(args.vg, 0.f, 6.f, std::to_string(active_length).c_str(), NULL);

                const auto active_channel = data.active_channel_id + 1;
                nvgFontSize(args.vg, 12);
                nvgText(args.vg, -b.size.x / 2.f + 23.f, b.size.y / 2.f - 9.f, std::to_string(active_channel).c_str(),
                        NULL);
        }

        // Scale to [-1, 1]
        nvgScale(args.vg, b.size.x / 2.f, b.size.y / 2.f);

        // Flip x and y so we start at the top and positive angle
        // increments go clockwise
        nvgScale(args.vg, -1.f, -1.f);

        // Inner circle radius
        const float inner_circle_radius = 0.17f;
        const float channel_width = (1.0f - inner_circle_radius) / static_cast<float>(PORT_MAX_CHANNELS);
        // Width of the line when drawing circles
        const float arc_stroke_width = channel_width / 2.0f;

        // Add a border so we don't draw over the edge
        nvgScale(args.vg, 1.0f - channel_width, 1.0f - channel_width);

        nvgStrokeWidth(args.vg, arc_stroke_width);

        // Each channel is drawn in one of three colours. Drawing every channel of a colour as one
        // path, rather than a path per beat, needs a few draw calls instead of hundreds. The rings
        // don't overlap, so the order they're drawn in doesn't matter.
        const NVGcolor colours[] = {m_display_accent, nvgRGB(0xff, 0xff, 0xff), nvgRGB(0x50, 0x50, 0x50)};
        uint8_t channel_colours[PORT_MAX_CHANNELS];
        for(uint8_t c = 0; c < PORT_MAX_CHANNELS; ++c)
        {
                if(c == data.active_channel_id)
                {
                        channel_colours[c] = 0;
                }
                else if(c < data.active_channels)
                {
                        channel_colours[c] = 1;
                }
                else
                {
                        channel_colours[c] = 2;
                }
        }

        for(uint8_t colour = 0; colour < sizeof(colours) / sizeof(colours[0]); ++colour)
        {
                nvgStrokeColor(args.vg, colours[colour]);
                nvgFillColor(args.vg, colours[colour]);

                // A dash for each beat
                nvgBeginPath(args.vg);
                for(uint8_t c = 0; c < PORT_MAX_CHANNELS; ++c)
                {
                        if(channel_colours[c] == colour)
                        {
                                addBeatArcs(args.vg, data.channels[c], 1.0f - c * channel_width);
                        }
                }
                nvgStroke(args.vg);

                // A dot before the current step
                nvgBeginPath(args.vg);
                for(uint8_t c = 0; c < PORT_MAX_CHANNELS; ++c)
                {
                        if(channel_colours[c] == colour)
                        {
                                addCurrentStepDot(args.vg, data.channels[c], 1.0f - c * channel_width,
                                                  arc_stroke_width / 2.0f);
                        }
                }
                nvgFill(args.vg);
        }

        nvgRestore(args.vg);
}

#define POLYGENE_POS(NAME) ORBITS_POS(POLYGENE_##NAME)
#define POLYGENE_SIZE(NAME) ORBITS_SIZE(POLYGENE_##NAME)

static const OrbitsSkinnedPanelConfig g_config = {{POLYGENE_DARK_PANEL, POLYGENE_LIGHT_PANEL}};
RareBreeds_Orbits_PolygeneWidget::RareBreeds_Orbits_PolygeneWidget(RareBreeds_Orbits_Polygene *polygene)
        : OrbitsWidget(g_config)
{
        using Polygene = RareBreeds_Orbits_Polygene;

        setModule(polygene);

        // Module may be NULL if this is the module selection screen
        if(polygene)
        {
                attachWidgetData(&polygene->m_widget_data);
                beat_widget.m_module = &polygene->m_beat;
                eoc_widget.m_module = &polygene->m_eoc;
        }

        // clang-format off
        OrbitsSkinnedScrewConfig screw;
        screw.imagePaths = {POLYGENE_DARK_SCREW, POLYGENE_LIGHT_SCREW};
        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(RACK_GRID_WIDTH + RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));

        screw.pos = Vec(box.size.x - RACK_GRID_WIDTH - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH / 2);
        addChild(createOrbitsSkinnedScrew(screw));


        OrbitsSkinnedKnobConfig knob;
        knob.imagePaths = {POLYGENE_DARK_CHANNEL_KNOB, POLYGENE_LIGHT_CHANNEL_KNOB};
        knob.pos = POLYGENE_POS(CHANNEL_KNOB);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(knob, polygene, Polygene::CHANNEL_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_LENGTH_KNOB, POLYGENE_LIGHT_LENGTH_KNOB};
        knob.pos = POLYGENE_POS(LENGTH_KNOB);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(knob, polygene, Polygene::LENGTH_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_HITS_KNOB, POLYGENE_LIGHT_HITS_KNOB};
        knob.pos = POLYGENE_POS(HITS_KNOB);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(knob, polygene, Polygene::HITS_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_SHIFT_KNOB, POLYGENE_LIGHT_SHIFT_KNOB};
        knob.pos = POLYGENE_POS(SHIFT_KNOB);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSnappyKnob>(knob, polygene, Polygene::SHIFT_KNOB_PARAM));

        knob.imagePaths = {POLYGENE_DARK_VARIATION_KNOB, POLYGENE_LIGHT_VARIATION_KNOB};
        knob.pos = POLYGENE_POS(VARIATION_KNOB);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedKnob>(knob, polygene, Polygene::VARIATION_KNOB_PARAM));

        OrbitsSkinnedSwitchConfig swtch;
        swtch.onImagePaths = {POLYGENE_DARK_REVERSE_SWITCH_ON, POLYGENE_LIGHT_REVERSE_SWITCH_ON};
        swtch.offImagePaths = {POLYGENE_DARK_REVERSE_SWITCH_OFF, POLYGENE_LIGHT_REVERSE_SWITCH_OFF};
        swtch.pos = POLYGENE_POS(REVERSE_SWITCH);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(swtch, polygene, Polygene::REVERSE_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_INVERT_SWITCH_ON, POLYGENE_LIGHT_INVERT_SWITCH_ON};
        swtch.offImagePaths = {POLYGENE_DARK_INVERT_SWITCH_OFF, POLYGENE_LIGHT_INVERT_SWITCH_OFF};
        swtch.pos = POLYGENE_POS(INVERT_SWITCH);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedSwitch>(swtch, polygene, Polygene::INVERT_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_RANDOM_BUTTON_ON, POLYGENE_LIGHT_RANDOM_BUTTON_ON};
        swtch.offImagePaths = {POLYGENE_DARK_RANDOM_BUTTON_OFF, POLYGENE_LIGHT_RANDOM_BUTTON_OFF};
        swtch.pos = POLYGENE_POS(RANDOM_BUTTON);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedButton>(swtch, polygene, Polygene::RANDOM_KNOB_PARAM));

        swtch.onImagePaths = {POLYGENE_DARK_SYNC_BUTTON_ON, POLYGENE_LIGHT_SYNC_BUTTON_ON};
        swtch.offImagePaths = {POLYGENE_DARK_SYNC_BUTTON_OFF, POLYGENE_LIGHT_SYNC_BUTTON_OFF};
        swtch.pos = POLYGENE_POS(SYNC_BUTTON);
        addParam(createOrbitsSkinnedParam<OrbitsSkinnedButton>(swtch, polygene, Polygene::SYNC_KNOB_PARAM));

        OrbitsSkinnedPortConfig port;
        port.imagePaths = {POLYGENE_DARK_CLOCK_PORT, POLYGENE_LIGHT_CLOCK_PORT};
        port.pos = POLYGENE_POS(CLOCK_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::CLOCK_INPUT));
        port.imagePaths = {POLYGENE_DARK_SYNC_PORT, POLYGENE_LIGHT_SYNC_PORT};
        port.pos = POLYGENE_POS(SYNC_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::SYNC_INPUT));
        port.imagePaths = {POLYGENE_DARK_LENGTH_CV_PORT, POLYGENE_LIGHT_LENGTH_CV_PORT};
        port.pos = POLYGENE_POS(LENGTH_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::LENGTH_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_HITS_CV_PORT, POLYGENE_LIGHT_HITS_CV_PORT};
        port.pos = POLYGENE_POS(HITS_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::HITS_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_SHIFT_CV_PORT, POLYGENE_LIGHT_SHIFT_CV_PORT};
        port.pos = POLYGENE_POS(SHIFT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::SHIFT_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_VARIATION_CV_PORT, POLYGENE_LIGHT_VARIATION_CV_PORT};
        port.pos = POLYGENE_POS(VARIATION_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::VARIATION_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_REVERSE_CV_PORT, POLYGENE_LIGHT_REVERSE_CV_PORT};
        port.pos = POLYGENE_POS(REVERSE_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::REVERSE_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_INVERT_CV_PORT, POLYGENE_LIGHT_INVERT_CV_PORT};
        port.pos = POLYGENE_POS(INVERT_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::INVERT_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_RANDOM_CV_PORT, POLYGENE_LIGHT_RANDOM_CV_PORT};
        port.pos = POLYGENE_POS(RANDOM_CV_PORT);
        addInput(createOrbitsSkinnedInput(port, polygene, Polygene::RANDOM_CV_INPUT));
        port.imagePaths = {POLYGENE_DARK_BEAT_PORT, POLYGENE_LIGHT_BEAT_PORT};
        port.pos = POLYGENE_POS(BEAT_PORT);
        addOutput(createOrbitsSkinnedOutput(port, polygene, Polygene::BEAT_OUTPUT));
        port.imagePaths = {POLYGENE_DARK_EOC_PORT, POLYGENE_LIGHT_EOC_PORT};
        port.pos = POLYGENE_POS(EOC_PORT);
        addOutput(createOrbitsSkinnedOutput(port, polygene, Polygene::EOC_OUTPUT));
        // clang-format on

        const PolygeneRhythmDisplayConfig display_config = {
                {POLYGENE_DARK_DISPLAY_ACCENT, POLYGENE_LIGHT_DISPLAY_ACCENT}};
        addChild(new PolygeneRhythmDisplay(polygene, display_config, POLYGENE_POS(DISPLAY), POLYGENE_SIZE(DISPLAY)));

        std::string panelPath =
                asset::plugin(pluginInstance, m_config.panelPaths[orbitsThemeIndex(kOrbitsThemeDefault)]);
        setPanel(APP->window->loadSvg(panelPath));
}

static void ToggleInputMode(InputMode *mode)
{
        switch(*mode)
        {
        case INPUT_MODE_MONOPHONIC_COPIES_TO_ALL:
                *mode = INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST;
                break;
        case INPUT_MODE_MONOPHONIC_COPIES_TO_FIRST:
        default:
                *mode = INPUT_MODE_MONOPHONIC_COPIES_TO_ALL;
                break;
        }
}

void RareBreeds_Orbits_PolygeneWidget::appendModuleContextMenu(Menu *menu)
{
        beat_widget.appendContextMenu(menu);
        eoc_widget.appendContextMenu(menu);

        RareBreeds_Orbits_Polygene *polygene = static_cast<RareBreeds_Orbits_Polygene *>(module);

        orbitsAppendRandomizeFilterMenu(menu, &polygene->m_randomization_mask,
                                        {{"Length", RareBreeds_Orbits_Polygene::RANDOMIZE_LENGTH},
                                         {"Hits", RareBreeds_Orbits_Polygene::RANDOMIZE_HITS},
                                         {"Shift", RareBreeds_Orbits_Polygene::RANDOMIZE_SHIFT},
                                         {"Variation", RareBreeds_Orbits_Polygene::RANDOMIZE_VARIATION},
                                         {"Reverse", RareBreeds_Orbits_Polygene::RANDOMIZE_REVERSE},
                                         {"Invert", RareBreeds_Orbits_Polygene::RANDOMIZE_INVERT}});

        menu->addChild(createSubmenuItem(
                "Mono Input Behavior", "",
                [=](Menu *submenu)
                {
                        submenu->addChild(createMenuLabel("Make mono cables only control the first channel"));

                        InputMode *input_mode_array = polygene->m_input_mode;
                        std::vector<std::pair<std::string, size_t>> items = {
                                std::make_pair("Sync", RareBreeds_Orbits_Polygene::SYNC_INPUT),
                                std::make_pair("Length", RareBreeds_Orbits_Polygene::LENGTH_CV_INPUT),
                                std::make_pair("Hits", RareBreeds_Orbits_Polygene::HITS_CV_INPUT),
                                std::make_pair("Shift", RareBreeds_Orbits_Polygene::SHIFT_CV_INPUT),
                                std::make_pair("Variation", RareBreeds_Orbits_Polygene::VARIATION_CV_INPUT),
                                std::make_pair("Random", RareBreeds_Orbits_Polygene::RANDOM_CV_INPUT)

                        };

                        for(auto i : items)
                        {
                                submenu->addChild(createCheckMenuItem(
                                        i.first, "", [=]() { return input_mode_array[i.second]; },
                                        [=]() { ToggleInputMode(&input_mode_array[i.second]); }));
                        }
                }));
}
