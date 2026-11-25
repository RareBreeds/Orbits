#include "OrbitsSkinned.hpp"
#include "OrbitsMacros.h"

Vec OrbitsPos(float x, float y)
{
        return mm2px(Vec(x, y));
}

bool orbitsThemeFromName(const char *name, OrbitsTheme &outTheme)
{
        if(!name)
        {
                return false;
        }

        for(uint8_t i = 0; i < static_cast<uint8_t>(OrbitsTheme::Count); ++i)
        {
                if(strcmp(name, kOrbitsThemeNames[i]) == 0)
                {
                        outTheme = static_cast<OrbitsTheme>(i);
                        return true;
                }
        }
        return false;
}

OrbitsSkinnedKnob::OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig &config) : m_config(config)
{
        minAngle = -0.83f * static_cast<float>(M_PI);
        maxAngle = 0.83f * static_cast<float>(M_PI);
        loadTheme(kOrbitsThemeDefault);
}

void OrbitsSkinnedKnob::loadTheme(OrbitsTheme theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[orbitsThemeIndex(theme)]);
        setSvg(APP->window->loadSvg(imagePath));
        fb->dirty = true;
}

OrbitsSkinnedSnappyKnob::OrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig &config) : OrbitsSkinnedKnob(config)
{
        snap = true;
}

OrbitsSkinnedScrew::OrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig &config) : m_config(config)
{
        loadTheme(kOrbitsThemeDefault);
}

void OrbitsSkinnedScrew::loadTheme(OrbitsTheme theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[orbitsThemeIndex(theme)]);
        setSvg(APP->window->loadSvg(imagePath));
        fb->dirty = true;
}

OrbitsSkinnedSwitch::OrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig &config) : m_config(config)
{
        std::string offImagePath =
                asset::plugin(pluginInstance, m_config.offImagePaths[orbitsThemeIndex(kOrbitsThemeDefault)]);
        std::string onImagePath =
                asset::plugin(pluginInstance, m_config.onImagePaths[orbitsThemeIndex(kOrbitsThemeDefault)]);

        addFrame(APP->window->loadSvg(offImagePath));
        addFrame(APP->window->loadSvg(onImagePath));
        shadow->opacity = 0.0;
}

void OrbitsSkinnedSwitch::loadTheme(OrbitsTheme theme)
{
#if ORBITS_THEME_SELECTION_ENABLED
        std::string offImagePath = asset::plugin(pluginInstance, m_config.offImagePaths[orbitsThemeIndex(theme)]);
        std::string onImagePath = asset::plugin(pluginInstance, m_config.onImagePaths[orbitsThemeIndex(theme)]);

        // Metamodule does not like the frames being changed, seems to corrupt the image path.
        // This happens whether I directly assign to frame[0] or clear the vector and re-add the frames.
        frames.clear();
        addFrame(APP->window->loadSvg(offImagePath));
        addFrame(APP->window->loadSvg(onImagePath));

        onChange(event::Change());
#else
        UNUSED(theme);
#endif
}

OrbitsSkinnedButton::OrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig &config) : OrbitsSkinnedSwitch(config)
{
        momentary = true;
}

OrbitsSkinnedPort::OrbitsSkinnedPort(const OrbitsSkinnedPortConfig &config) : m_config(config)
{
        loadTheme(kOrbitsThemeDefault);
        shadow->opacity = 0.0f;
}

void OrbitsSkinnedPort::loadTheme(OrbitsTheme theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[orbitsThemeIndex(theme)]);
        setSvg(APP->window->loadSvg(imagePath));
        // fb->dirty = true; // Already set by setSvg for SvgPorts
}

OrbitsSkinned::~OrbitsSkinned()
{
}

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig &config)
{
        OrbitsSkinnedScrew *o = new OrbitsSkinnedScrew(config);
        orbitsCentreOn(o, config.pos);
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedPort(const OrbitsSkinnedPortConfig &config, engine::Module *module, int portId)
{
        OrbitsSkinnedPort *o = new OrbitsSkinnedPort(config);
        orbitsCentreOn(o, config.pos);
        o->module = module;
        o->portId = portId;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedInput(const OrbitsSkinnedPortConfig &config, engine::Module *module, int inputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, module, inputId);
        o->type = engine::Port::INPUT;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedOutput(const OrbitsSkinnedPortConfig &config, engine::Module *module,
                                             int outputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, module, outputId);
        o->type = engine::Port::OUTPUT;
        return o;
}
