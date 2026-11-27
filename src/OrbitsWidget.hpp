#pragma once

#include "OrbitsModule.hpp"
#include "OrbitsSkinned.hpp"
#include "OrbitsWidgetData.hpp"

// The theme chosen in a module's menu: either follow Rack's "Use dark panels if available" setting,
// or always use one theme.
struct OrbitsThemeSetting
{
        bool m_follow_rack = true;
        // The theme to use when not following Rack
        OrbitsTheme m_theme = kOrbitsThemeDefault;

        OrbitsTheme shownTheme() const;
        void toJson(json_t *root) const;
        void fromJson(json_t *root);
};

struct OrbitsWidget : ModuleWidget
{
        OrbitsTheme m_theme = kOrbitsThemeDefault;
        OrbitsThemeSetting m_theme_setting;
        OrbitsSkinnedPanelConfig m_config;
        OrbitsWidgetData *m_widget_data = NULL;

        OrbitsWidget(const OrbitsSkinnedPanelConfig &config);
        ~OrbitsWidget();
        // Connects the widget to its module's saved widget data.
        // Call from the constructor when there's a module - which there isn't
        // in the module browser.
        void attachWidgetData(OrbitsWidgetData *data);
        void draw(const DrawArgs &args) override;
        void appendContextMenu(Menu *menu) override;
        void step() override;
        virtual void appendModuleContextMenu(Menu *menu) = 0;
        void loadInitialTheme();
        void switchTheme(OrbitsTheme theme);
        json_t *dataToJson();
        void dataFromJson(json_t *root);
};

// The tint to draw Orbits' displays with in layer 1, which isn't dimmed with the room lights:
// orbitsLightBrightness() in each colour channel.
NVGcolor orbitsLightTint();

// The alpha to draw a widget's layer 1 with.
// 1 if the module isn't bypassed, or if the platform fades the layer for us.
float orbitsBypassAlpha(Widget *widget);

#if ORBITS_ROOM_LIGHTING_ENABLED
// How bright Orbits' lights and displays are, from 0 (off) to 1: the room brightness plus the light
// boost setting, so they follow the room lights and stay that much brighter.
float orbitsLightBrightness();

// The light boost setting, from 0 (lights dim with the room like the rest of the panel) to 1
// (always fully lit). One setting for all Orbits modules, saved in Rack's settings.
float orbitsLightBoost();
void orbitsSetLightBoost(float boost);
#endif

// A display, drawn in layer 1 so it doesn't dim when the room lights are dimmed. The content widget
// draws the display in its draw(). Where framebuffer caching is enabled the content is cached, and
// only redrawn after setDirty(), so call that whenever what the display shows changes.
struct OrbitsDisplay : TransparentWidget
{
        OrbitsDisplay(Vec pos, Vec size);
        // Sets the widget that draws the display. Call once; the display takes ownership of it.
        void setContent(Widget *content);
        void setDirty();
        void draw(const DrawArgs &args) override;
        void drawLayer(const DrawArgs &args, int layer) override;

      private:
        Widget *m_content = NULL;
#if ORBITS_FRAMEBUFFER_CACHING_ENABLED
        // A FramebufferWidget only caches what its children draw in draw(), not drawLayer(), so the
        // content draws in draw() and the framebuffer is shown in layer 1.
        FramebufferWidget *m_fb = NULL;
#endif
};

struct EOCWidget
{
        EOCMode *m_module = NULL;
        void appendContextMenu(Menu *menu);
};

// One option in the Randomize Filter menu: its name, and its bit in the module's randomization mask.
struct OrbitsRandomizeFilterOption
{
        const char *name;
        unsigned int bit;
};

// Adds the "Randomize Filter" submenu, with a check item for each option that toggles its bit in
// `*mask`. Bits that are set are randomized.
void orbitsAppendRandomizeFilterMenu(Menu *menu, unsigned int *mask,
                                     const std::vector<OrbitsRandomizeFilterOption> &options);

struct BeatWidget
{
        BeatMode *m_module = NULL;
        void appendContextMenu(Menu *menu);
};
