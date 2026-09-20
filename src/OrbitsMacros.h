#pragma once

#define UNUSED(x) (void)(x)

// Metamodule does not like how I change the frames for switches, seems to corrupt the image path.
// This happens whether I directly assign to frame[0] or clear the vector and re-add the frames.
#if METAMODULE
#define ORBITS_THEME_SELECTION_ENABLED 0
#else
#define ORBITS_THEME_SELECTION_ENABLED 1
#endif
