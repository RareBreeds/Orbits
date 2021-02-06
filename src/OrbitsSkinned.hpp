#pragma once

#include "OrbitsConfig.hpp"
#include "plugin.hpp"

// Interface for components with the ability to change skins
struct OrbitsSkinned
{
        OrbitsConfig *m_config;
        std::string m_component;
        virtual void loadTheme(int theme);
};

struct OrbitsSkinnedKnob : RoundKnob, OrbitsSkinned
{
        OrbitsSkinnedKnob(OrbitsConfig *config, std::string component);
        void loadTheme(int theme) override;
};

struct OrbitsNonRandomizedSkinnedKnob : OrbitsSkinnedKnob
{
        OrbitsNonRandomizedSkinnedKnob(OrbitsConfig *config, std::string component);
        void randomize() override;
};

struct OrbitsSkinnedScrew : app::SvgScrew, OrbitsSkinned
{
        OrbitsSkinnedScrew(OrbitsConfig *config, std::string component);
        void loadTheme(int theme) override;
};

struct OrbitsSkinnedSwitch : app::SvgSwitch, OrbitsSkinned
{
        OrbitsSkinnedSwitch(OrbitsConfig *config, std::string component);
        void loadTheme(int theme) override;
};

struct OrbitsSkinnedPort : app::SvgPort, OrbitsSkinned
{
        OrbitsSkinnedPort(OrbitsConfig *config, std::string component);
        void loadTheme(int theme) override;
};

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(OrbitsConfig *config, std::string component, math::Vec pos);
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
        if(module)
        {
                o->paramQuantity = module->paramQuantities[paramId];
        }
        return o;
}
