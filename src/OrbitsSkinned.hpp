#pragma once

#include "OrbitsMacros.h"

#include "plugin.hpp"

#include <array>
#include <string>

enum class OrbitsTheme
{
        Dark,
        Light,
        Count
};
constexpr std::array<const char *, static_cast<size_t>(OrbitsTheme::Count)> kOrbitsThemeNames = {
        "Dark",
        "Light",
};
constexpr OrbitsTheme kOrbitsThemeDefault = OrbitsTheme::Dark;

constexpr uint8_t orbitsThemeIndex(OrbitsTheme t)
{
        return static_cast<uint8_t>(t);
}

constexpr const char *orbitsThemeName(OrbitsTheme t)
{
        return kOrbitsThemeNames[static_cast<size_t>(t)];
}

bool orbitsThemeFromName(const char *name, OrbitsTheme &outTheme);

#define ORBITS_POS(NAME) OrbitsPos(NAME##_X, NAME##_Y)
#define ORBITS_SIZE(NAME) OrbitsPos(NAME##_WIDTH, NAME##_HEIGHT)
Vec OrbitsPos(float x, float y);

// Interface for components with the ability to change skins
struct OrbitsSkinned
{
        virtual void loadTheme(OrbitsTheme theme) = 0;
        virtual ~OrbitsSkinned() = 0;
};

struct OrbitsSkinnedPanelConfig
{
        // This struct is used as a global and the C string
        // prevents exit-time-destructor warnings
        const char *panelPaths[orbitsThemeIndex(OrbitsTheme::Count)];
};

struct OrbitsSkinnedKnobConfig
{
        std::array<std::string, orbitsThemeIndex(OrbitsTheme::Count)> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedKnob : SvgKnob, OrbitsSkinned
{
        OrbitsSkinnedKnobConfig m_config;
        OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig &config);
        void loadTheme(OrbitsTheme theme) override;
};

struct OrbitsSkinnedSnappyKnob : OrbitsSkinnedKnob
{
        OrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig &config);
};

struct OrbitsSkinnedScrewConfig
{
        std::array<std::string, orbitsThemeIndex(OrbitsTheme::Count)> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedScrew : app::SvgScrew, OrbitsSkinned
{
        OrbitsSkinnedScrewConfig m_config;
        OrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig &config);
        void loadTheme(OrbitsTheme theme) override;
};

struct OrbitsSkinnedSwitchConfig
{
        std::array<std::string, orbitsThemeIndex(OrbitsTheme::Count)> onImagePaths;
        std::array<std::string, orbitsThemeIndex(OrbitsTheme::Count)> offImagePaths;
        Vec pos;
};

#if ORBITS_ROOM_LIGHTING_ENABLED
// Draws an SVG as a light: in layer 1, which isn't dimmed with the room lights, faded by the light
// brightness setting. Nothing is drawn in layer 0.
struct OrbitsLitOverlay : Widget
{
        OrbitsLitOverlay();
        void setSvg(std::shared_ptr<window::Svg> svg);
        void draw(const DrawArgs &args) override;
        void drawLayer(const DrawArgs &args, int layer) override;

      private:
        // The SVG's shapes overlap, so it's drawn into a framebuffer and faded as one image.
        FramebufferWidget *m_fb = NULL;
        SvgWidget *m_svg = NULL;
};
#endif

// A switch whose "on" state is a light, like a button with an LED in it: it always shows its unlit
// "off" image, which dims with the room lights, and while it's on the "on" image is drawn on top as
// a light.
struct OrbitsSkinnedSwitch : app::SvgSwitch, OrbitsSkinned
{
        OrbitsSkinnedSwitchConfig m_config;
#if ORBITS_ROOM_LIGHTING_ENABLED
        // Draws the "on" image
        OrbitsLitOverlay *m_lit = NULL;
#endif
        OrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig &config);
        void loadTheme(OrbitsTheme theme) override;
        void onChange(const ChangeEvent &e) override;
};

struct OrbitsSkinnedButton : OrbitsSkinnedSwitch
{
        OrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig &config);
};

struct OrbitsSkinnedPortConfig
{
        std::array<std::string, orbitsThemeIndex(OrbitsTheme::Count)> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedPort : app::SvgPort, OrbitsSkinned
{
        OrbitsSkinnedPortConfig m_config;
        OrbitsSkinnedPort(const OrbitsSkinnedPortConfig &config);
        void loadTheme(OrbitsTheme theme) override;
};

// Configs for a control with an image per theme, at `pos`.
OrbitsSkinnedKnobConfig orbitsKnobConfig(const char *dark, const char *light, Vec pos);
OrbitsSkinnedSwitchConfig orbitsSwitchConfig(const char *dark_on, const char *light_on, const char *dark_off,
                                             const char *light_off, Vec pos);
OrbitsSkinnedPortConfig orbitsPortConfig(const char *dark, const char *light, Vec pos);

// Moves a widget so it's centred on `pos`
inline void orbitsCentreOn(widget::Widget *w, Vec pos)
{
        w->box.pos = pos.minus(w->box.size.div(2));
}

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig &config);

// Creates a skinned knob, switch or button, TParamWidget, controlling `paramId` of `module`.
template <typename TParamWidget, typename TConfig>
TParamWidget *createOrbitsSkinnedParam(const TConfig &config, engine::Module *module, int paramId)
{
        TParamWidget *o = new TParamWidget(config);
        orbitsCentreOn(o, config.pos);
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedPort(const OrbitsSkinnedPortConfig &config, engine::Module *module, int portId);
OrbitsSkinnedPort *createOrbitsSkinnedInput(const OrbitsSkinnedPortConfig &config, engine::Module *module, int inputId);
OrbitsSkinnedPort *createOrbitsSkinnedOutput(const OrbitsSkinnedPortConfig &config, engine::Module *module,
                                             int outputId);
