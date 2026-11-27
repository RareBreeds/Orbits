#include "OrbitsSkinned.hpp"
#include "OrbitsMacros.h"
#include "OrbitsWidget.hpp"

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

OrbitsSkinnedKnobConfig orbitsKnobConfig(const char *dark, const char *light, Vec pos)
{
        OrbitsSkinnedKnobConfig config;
        config.imagePaths = {dark, light};
        config.pos = pos;
        return config;
}

OrbitsSkinnedSwitchConfig orbitsSwitchConfig(const char *dark_on, const char *light_on, const char *dark_off,
                                             const char *light_off, Vec pos)
{
        OrbitsSkinnedSwitchConfig config;
        config.onImagePaths = {dark_on, light_on};
        config.offImagePaths = {dark_off, light_off};
        config.pos = pos;
        return config;
}

OrbitsSkinnedPortConfig orbitsPortConfig(const char *dark, const char *light, Vec pos)
{
        OrbitsSkinnedPortConfig config;
        config.imagePaths = {dark, light};
        config.pos = pos;
        return config;
}

OrbitsSkinnedKnob::OrbitsSkinnedKnob(const OrbitsSkinnedKnobConfig &config) : m_config(config)
{
        minAngle = -0.83f * static_cast<float>(M_PI);
        maxAngle = 0.83f * static_cast<float>(M_PI);
        // Rack smooths knob changes by default, which is for parameters that affect audio. These
        // are only read on clock edges and rounded to whole steps.
        smooth = false;
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

#if ORBITS_ROOM_LIGHTING_ENABLED
        m_lit = new OrbitsLitOverlay;
        m_lit->setSvg(frames[1]);
        m_lit->setVisible(false);
        addChild(m_lit);
#endif
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

#if ORBITS_ROOM_LIGHTING_ENABLED
        m_lit->setSvg(frames[1]);
#endif

        onChange(event::Change());
#else
        UNUSED(theme);
#endif
}

void OrbitsSkinnedSwitch::onChange(const ChangeEvent &e)
{
        SvgSwitch::onChange(e);
#if ORBITS_ROOM_LIGHTING_ENABLED
        // SvgSwitch shows the "on" frame when the value rounds to 1 or more. Show the unlit "off" frame
        // instead, and the "on" frame as a light on top of it.
        engine::ParamQuantity *pq = getParamQuantity();
        m_lit->setVisible(pq && std::round(pq->getValue() - pq->getMinValue()) >= 1.f);
        sw->setSvg(frames[0]);
        fb->setDirty();
#endif
}

#if ORBITS_ROOM_LIGHTING_ENABLED
OrbitsLitOverlay::OrbitsLitOverlay()
{
        m_fb = new FramebufferWidget;
        addChild(m_fb);
        m_svg = new SvgWidget;
        m_fb->addChild(m_svg);
}

void OrbitsLitOverlay::setSvg(std::shared_ptr<window::Svg> svg)
{
        m_svg->setSvg(svg);
        box.size = m_svg->box.size;
        m_fb->box.size = m_svg->box.size;
        m_fb->setDirty();
}

void OrbitsLitOverlay::draw(const DrawArgs & /*args*/)
{
        // Only drawn in layer 1
}

void OrbitsLitOverlay::drawLayer(const DrawArgs &args, int layer)
{
        const float brightness = orbitsLightBrightness();
        if(layer == 1 && brightness > 0.f)
        {
                nvgGlobalTint(args.vg, nvgRGBAf(1.f, 1.f, 1.f, brightness * orbitsBypassAlpha(this)));
                drawChild(m_fb, args);
        }
}
#endif

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
