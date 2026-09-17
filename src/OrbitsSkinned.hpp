#pragma once

#include "OrbitsConfig.hpp"
#include "plugin.hpp"

// TODO: MOOOO: Better place for this, OrbitsConfig?
#define ORBITS_THEME_COUNT (2)
#define ORBITS_THEME_DEFAULT (0)
#define ORBITS_POS(NAME) OrbitsPos(NAME##_POS_X, NAME##_POS_Y)
Vec OrbitsPos(float x, float y);

// Interface for components with the ability to change skins
struct OrbitsSkinned
{
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

struct OrbitsSkinnedPortConfig
{
        std::array<std::string, ORBITS_THEME_COUNT> imagePaths;
        Vec pos;
};

struct OrbitsSkinnedPort : app::SvgPort, OrbitsSkinned
{
        OrbitsSkinnedPortConfig m_config;
        OrbitsSkinnedPort(const OrbitsSkinnedPortConfig& config);
        void loadTheme(int theme) override;
};

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig& config);
OrbitsSkinnedSnappyKnob *createOrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedKnob *createOrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedSwitch *createOrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId);
OrbitsSkinnedButton *createOrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId);

OrbitsSkinnedPort *createOrbitsSkinnedPort(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                           int portId);
OrbitsSkinnedPort *createOrbitsSkinnedInput(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                            int inputId);
OrbitsSkinnedPort *createOrbitsSkinnedOutput(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                             int outputId);
