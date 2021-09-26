#include "OrbitsSkinned.hpp"

OrbitsSkinnedKnob::OrbitsSkinnedKnob(OrbitsConfig *config, std::string component)
{
        m_config = config;
        m_component = component;
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        loadTheme(m_config->getDefaultThemeId());
}

void OrbitsSkinnedKnob::loadTheme(int theme)
{
        setSvg(APP->window->loadSvg(m_config->getSvg(m_component, theme)));
        fb->dirty = true;
}

OrbitsSkinnedScrew::OrbitsSkinnedScrew(OrbitsConfig *config, std::string component)
{
        m_config = config;
        m_component = component;
        loadTheme(m_config->getDefaultThemeId());
}

void OrbitsSkinnedScrew::loadTheme(int theme)
{
        setSvg(APP->window->loadSvg(m_config->getSvg(m_component, theme)));
        fb->dirty = true;
}

OrbitsSkinnedSwitch::OrbitsSkinnedSwitch(OrbitsConfig *config, std::string component)
{
        m_config = config;
        m_component = component;
        addFrame(APP->window->loadSvg(m_config->getSvg(m_component + "_off")));
        addFrame(APP->window->loadSvg(m_config->getSvg(m_component + "_on")));
        shadow->opacity = 0.0;
}

void OrbitsSkinnedSwitch::loadTheme(int theme)
{
        frames[0] = APP->window->loadSvg(m_config->getSvg(m_component + "_off", theme));
        frames[1] = APP->window->loadSvg(m_config->getSvg(m_component + "_on", theme));

        event::Change change;
        onChange(change);
        onChange(change);
}

OrbitsSkinnedButton::OrbitsSkinnedButton(OrbitsConfig *config, std::string component)
        : OrbitsSkinnedSwitch(config, component)
{
        momentary = true;
}

OrbitsSkinnedPort::OrbitsSkinnedPort(OrbitsConfig *config, std::string component)
{
        m_config = config;
        m_component = component;
        loadTheme(m_config->getDefaultThemeId());
        shadow->opacity = 0.07;
}

void OrbitsSkinnedPort::loadTheme(int theme)
{
        setSvg(APP->window->loadSvg(m_config->getSvg(m_component, theme)));
        // fb->dirty = true; // Already set by setSvg for SvgPorts
}

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(OrbitsConfig *config, std::string component, math::Vec pos)
{
        OrbitsSkinnedScrew *o = new OrbitsSkinnedScrew(config, component);
        o->box.pos = pos.minus(o->box.size.div(2));
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedPort(OrbitsConfig *config, std::string component, engine::Module *module,
                                           int portId)
{
        OrbitsSkinnedPort *o = new OrbitsSkinnedPort(config, component);
        o->box.pos = config->getPos(component).minus(o->box.size.div(2));
        o->module = module;
        o->portId = portId;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedInput(OrbitsConfig *config, std::string component, engine::Module *module,
                                            int inputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, component, module, inputId);
        o->type = engine::Port::INPUT;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedOutput(OrbitsConfig *config, std::string component, engine::Module *module,
                                             int outputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, component, module, outputId);
        o->type = engine::Port::OUTPUT;
        return o;
}