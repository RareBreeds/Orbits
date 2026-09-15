#include "OrbitsSkinned.hpp"

OrbitsSkinnedKnob::OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config)
{
        m_config = config;
        minAngle = -0.83f * (float)M_PI;
        maxAngle = 0.83f * (float)M_PI;
        loadTheme(ORBITS_THEME_DEFAULT);
}

void OrbitsSkinnedKnob::loadTheme(int theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[theme]);
        setSvg(APP->window->loadSvg(imagePath));
        fb->dirty = true;
}

OrbitsSkinnedSnappyKnob::OrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig& config) : OrbitsSkinnedKnob(config)
{
        snap = true;
}

OrbitsSkinnedScrew::OrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig& config) : m_config(config)
{
        loadTheme(ORBITS_THEME_DEFAULT);
}

void OrbitsSkinnedScrew::loadTheme(int theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[theme]);
        setSvg(APP->window->loadSvg(imagePath));
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
        shadow->opacity = 0.0f;
}

void OrbitsSkinnedPort::loadTheme(int theme)
{
        setSvg(APP->window->loadSvg(m_config->getSvg(m_component, theme)));
        // fb->dirty = true; // Already set by setSvg for SvgPorts
}

OrbitsSkinned::~OrbitsSkinned()
{
}

OrbitsSkinnedScrew *createOrbitsSkinnedScrew(const OrbitsSkinnedScrewConfig& config)
{
        OrbitsSkinnedScrew *o = new OrbitsSkinnedScrew(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        return o;
}

OrbitsSkinnedSnappyKnob *createOrbitsSkinnedSnappyKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId)
{
        OrbitsSkinnedSnappyKnob *o = new OrbitsSkinnedSnappyKnob(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
        return o;
}

OrbitsSkinnedKnob *createOrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config, engine::Module *module, int paramId)
{
        OrbitsSkinnedKnob *o = new OrbitsSkinnedKnob(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
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
