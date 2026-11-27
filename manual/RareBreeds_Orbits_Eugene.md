*Eugene* is a [Euclidean rhythm](https://en.wikipedia.org/wiki/Euclidean_rhythm) generator.
A Euclidean rhythm spreads a number of *hits* as evenly as possible over a number of *steps*, which produces many of the rhythms found in music around the world: 3 hits in 8 steps is the Cuban tresillo, and 5 in 8 the cinquillo.

![Eugene, Dark theme](images/RareBreeds_Orbits_Eugene-dark.png) ![Eugene, Light theme](images/RareBreeds_Orbits_Eugene-light.png)

Patch a clock into **CLK**. Each clock pulse plays the next step of the rhythm, and **OUT** sends a trigger on every step that's a hit.

The **LENGTH** knob sets the number of steps in the rhythm, from 1 to 32.

The **HITS** knob sets how many of those steps are hits, from none to all of them.
It's a proportion of the length, so the rhythm keeps the same density as the length changes; its tooltip shows the number of hits being played, e.g. "5 / 16".

The **SHIFT** knob rotates the rhythm clockwise by up to 31 steps, wrapping around at the length. For example, on an 8-step rhythm, a shift of 8 is the same as 0.
Rotating it changes which part of the pattern plays first.

Rhythms always start at the first step, at 12 o'clock on the display.
A trigger on the **SYNC** input, or pressing the button below it, restarts the rhythm there, so it can be kept in time with another sequencer or a reset signal. Nothing plays on the sync itself: the next clock plays the first step.
If the length is shortened past the current step, the rhythm also restarts from the first step.

**REV** plays the rhythm backwards (anticlockwise); after a restart it starts from the last step.
**INV** swaps hits and rests: the steps that were hits are silent and the rest play.

##### Display

![Eugene's display](images/eugene-display.png)

Each step is a circle around the display: hits are large circles and rests small dots.
When inverted, hits are shown as outlines instead.
The step that has just played is circled, and the triangle shows the shift: it points to where the rhythm started before it was shifted.
In the middle are the number of hits (top) and the length (bottom), with an arrow showing the direction of play.

##### CV

LENGTH, HITS and SHIFT each have a CV input and an attenuator knob above it.
The input takes ±5V, which is scaled by the attenuator (0% to 100%) and added to the knob:
at 100%, +5V adds the knob's whole range (31 steps of length or shift, or all of the steps as hits) and -5V takes it away.
For example, with the knob at its minimum and the attenuator at 100%, 0V to 5V sweeps the whole range.
The knobs' tooltips show the value being played, including CV.

The CV inputs next to REV and INV are gates that override their buttons while patched: 1V or above turns reverse or invert on, and 0V or below turns it off.

A trigger on the **RND** input randomizes the module, the same as Rack's *Randomize* (Ctrl/Cmd+R).

##### Outputs

**OUT** sends a 10V signal on each hit, shaped by **Beat Mode** in the context menu:

- **Pulse** (default): a 1 ms trigger on each hit.
- **Gate**: high from each hit until the next step, with a 1 ms gap at the start of each hit so that consecutive hits retrigger.
- **Hold**: high from each hit until the next rest, staying high through consecutive hits.

**EOC** (end of cycle) sends a 1 ms trigger once per cycle of the rhythm. Its timing is set by **EOC Mode** in the context menu:

- **On Repeat** (default): when the rhythm loops round from its last step to its first. It doesn't trigger when the rhythm first starts or after a sync trigger part way through.
- **First**: on the first step of every cycle, including after a restart.
- **Last**: on the last step of every cycle.

When reversed, the first and last steps are those in the direction of play.

##### Settings

Right-click the panel for these settings, as well as the Theme and Light boost settings described in the [Orbits manual](plugin.md).

- **Beat Mode** and **EOC Mode**: see Outputs above.
- **Randomize Filter**: chooses which controls Randomize and the RND input change: the LENGTH, HITS and SHIFT knobs, their CV attenuators, and the REV and INV buttons. All are on by default.

##### Presets

Eugene comes with factory presets of rhythms from around the world and from well-known songs, such as Bossa Nova, Samba, Tresillo and Take Five. Find them under *Preset* in the context menu.
