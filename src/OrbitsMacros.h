#pragma once

#define UNUSED(x) (void)(x)

// Metamodule does not like how I change the frames for switches, seems to corrupt the image path.
// This happens whether I directly assign to frame[0] or clear the vector and re-add the frames.
#ifdef METAMODULE
#define ORBITS_THEME_SELECTION_ENABLED 0
#else
#define ORBITS_THEME_SELECTION_ENABLED 1
#endif

// MetaModule's FramebufferWidget doesn't draw anything, and MetaModule redraws displays every frame
// anyway, so displays are drawn directly there instead of being cached in a framebuffer.
#ifdef METAMODULE
#define ORBITS_FRAMEBUFFER_CACHING_ENABLED 0
#else
#define ORBITS_FRAMEBUFFER_CACHING_ENABLED 1
#endif

// Rack's room lighting: buttons' "on" images are drawn as lights, which don't dim with the room
// lights, and there's a brightness setting for Orbits' lights. MetaModule has no room lighting and
// draws switches itself, so it doesn't need either.
#ifdef METAMODULE
#define ORBITS_ROOM_LIGHTING_ENABLED 0
#else
#define ORBITS_ROOM_LIGHTING_ENABLED 1
#endif

// Rack fades a bypassed module in ModuleWidget::draw(), which doesn't cover what's drawn in layer 1,
// so Orbits' lit buttons and displays fade themselves. MetaModule fades everything on a bypassed
// module itself.
#ifdef METAMODULE
#define ORBITS_BYPASS_FADE_ENABLED 0
#else
#define ORBITS_BYPASS_FADE_ENABLED 1
#endif
