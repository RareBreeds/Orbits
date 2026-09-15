#pragma once

#include "OrbitsConfig.hpp"
#include "plugin.hpp"

// TODO: Better place for this
#define ORBITS_THEME_COUNT (2)
#define ORBITS_THEME_DEFAULT (0)

// Interface for components with the ability to change skins
struct OrbitsSkinned
{
        OrbitsConfig *m_config;
        std::string m_component;
        virtual void loadTheme(int theme) = 0;
        virtual ~OrbitsSkinned() = 0;
};

// TODO: Same as ScrewConfig
struct OrbitsSkinnedKnobConfig
{
        std::array<std::string, ORBITS_THEME_COUNT> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedKnob : SvgKnob, OrbitsSkinned
{
        OrbitsSkinnedKnobConfig m_config;
        OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config);
        void loadTheme(int theme) override;
};

struct OrbitsSkinnedSnappyKnob : OrbitsSkinnedKnob
{
        OrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig& config);
};

struct OrbitsSkinnedScrewConfig
{
        std::array<std::string, ORBITS_THEME_COUNT> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedScrew : app::SvgScrew, OrbitsSkinned
{
        OrbitsSkinnedScrewConfig m_config;
        OrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig& config);
        void loadTheme(int theme) override;
};

struct OrbitsSkinnedSwitchConfig
{
        std::array<std::string, ORBITS_THEME_COUNT> onImagePaths;
        std::array<std::string, ORBITS_THEME_COUNT> offImagePaths;
        Vec pos;
};

struct OrbitsSkinnedSwitch : app::SvgSwitch, OrbitsSkinned
{
        OrbitsSkinnedSwitchConfig m_config;
        OrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig& config);
        void loadTheme(int theme) override;
};

struct OrbitsSkinnedButton : OrbitsSkinnedSwitch
{
        OrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig& config);
};

struct OrbitsSkinnedPort : app::SvgPort, OrbitsSkinned
{
        OrbitsSkinnedPort(OrbitsConfig *config, std::string component);
        void loadTheme(int theme) override;
};

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig& config);
OrbitsSkinnedSnappyKnob *createOrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedKnob *createOrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedSwitch *createOrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedButton *createOrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId);

OrbitsSkinnedPort *createOrbitsSkinnedPort(OrbitsConfig *config, std::string component, engine::Module *module,
                                           int portId);
OrbitsSkinnedPort *createOrbitsSkinnedInput(OrbitsConfig *config, std::string component, engine::Module *module,
                                            int inputId);
OrbitsSkinnedPort *createOrbitsSkinnedOutput(OrbitsConfig *config, std::string component, engine::Module *module,
                                             int outputId);


template <class TParamWidget>
TParamWidget *createOrbitsSkinnedParam(OrbitsConfig *config, std::string component, engine::Module *module, int paramId)
{
        TParamWidget *o = new TParamWidget(config, component);
        o->box.pos = config->getPos(component).minus(o->box.size.div(2));
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
        return o;
}
