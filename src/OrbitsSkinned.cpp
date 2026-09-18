#include "OrbitsSkinned.hpp"

Vec OrbitsPos(float x, float y)
{
        return mm2px(Vec(x, y));
}

OrbitsSkinnedKnob::OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig& config) : m_config(config)
{
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

OrbitsSkinnedSwitch::OrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig& config) : m_config(config)
{
        std::string offImagePath = asset::plugin(pluginInstance, m_config.offImagePaths[ORBITS_THEME_DEFAULT]);
        std::string onImagePath = asset::plugin(pluginInstance, m_config.onImagePaths[ORBITS_THEME_DEFAULT]);

        addFrame(APP->window->loadSvg(offImagePath));
        addFrame(APP->window->loadSvg(onImagePath));
        shadow->opacity = 0.0;
}

void OrbitsSkinnedSwitch::loadTheme(int theme)
{
        std::string offImagePath = asset::plugin(pluginInstance, m_config.offImagePaths[theme]);
        std::string onImagePath = asset::plugin(pluginInstance, m_config.onImagePaths[theme]);

#if METAMODULE == 0
        // TODO: MOOOO: Metamodule does not like this, seems to corrupt the image path that it's using
        // whether you directly assign to frame[0] or clear then re-add
        frames.clear();
        addFrame(APP->window->loadSvg(offImagePath));
        addFrame(APP->window->loadSvg(onImagePath));
#endif

        event::Change change;
        onChange(change);
        onChange(change);
}

OrbitsSkinnedButton::OrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig& config)
        : OrbitsSkinnedSwitch(config)
{
        momentary = true;
}

OrbitsSkinnedPort::OrbitsSkinnedPort(const OrbitsSkinnedPortConfig& config) : m_config(config)
{
        loadTheme(ORBITS_THEME_DEFAULT);
        shadow->opacity = 0.0f;
}

void OrbitsSkinnedPort::loadTheme(int theme)
{
        std::string imagePath = asset::plugin(pluginInstance, m_config.imagePaths[theme]);
        setSvg(APP->window->loadSvg(imagePath));
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

OrbitsSkinnedSwitch *createOrbitsSkinnedSwitch(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId)
{
        OrbitsSkinnedSwitch *o = new OrbitsSkinnedSwitch(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
        return o;
}

OrbitsSkinnedButton *createOrbitsSkinnedButton(const OrbitsSkinnedSwitchConfig& config, engine::Module *module, int paramId)
{
        OrbitsSkinnedButton *o = new OrbitsSkinnedButton(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        o->app::ParamWidget::module = module;
        o->app::ParamWidget::paramId = paramId;
        o->initParamQuantity();
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedPort(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                           int portId)
{
        OrbitsSkinnedPort *o = new OrbitsSkinnedPort(config);
        o->box.pos = config.pos.minus(o->box.size.div(2));
        o->module = module;
        o->portId = portId;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedInput(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                            int inputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, module, inputId);
        o->type = engine::Port::INPUT;
        return o;
}

OrbitsSkinnedPort *createOrbitsSkinnedOutput(const OrbitsSkinnedPortConfig& config, engine::Module *module,
                                             int outputId)
{
        OrbitsSkinnedPort *o = createOrbitsSkinnedPort(config, module, outputId);
        o->type = engine::Port::OUTPUT;
        return o;
}
