## [Unreleased]
### Added
 - Support for the 4ms MetaModule
 - The INV, REV, RND and SYNC buttons behave as lights, so they stay lit when Rack's room lights are dimmed
 - "Light boost" plugin-wide setting in the context menu, for how much brighter than the room the lights and displays are
 - "Follow Rack" theme, the default for new modules, which uses the Light or Dark theme to match Rack's "Use dark panels if available" setting. Modules in existing patches keep their theme
 - Tooltip descriptions for most inputs, outputs and knobs, including what Length, Hits, Shift and Variation do
 - A new manual, this time based on the Rack guidelines

### Changed
 - Polygene's RND input no longer adds to the undo history each time it randomizes a channel
 - The displays follow Rack's room brightness, 30% brighter by default, instead of always being fully bright
 - New modules use the Light theme unless "Use dark panels if available" is on in Rack's View menu, instead of always starting Dark
 - Eugene's Invert input is named "Invert CV", like its other CV inputs and Polygene's
 - Length, Hits, Shift and Variation show the value being played, including CV, e.g. Hits "5 / 16". Typing a number sets the value played
 - Eugene's CV knobs show percentages
 - Hits, Variation and Eugene's CV knobs change straight away when turned or typed into, instead of gliding to the new value
 - Bypassing Eugene or Polygene silences their outputs, instead of passing the clock input through to the beat output
 - The displays fade when a module is bypassed, like the rest of the panel, instead of staying fully bright

### Fixed
 - Length, Hits and Shift CV well below 0V could jump to the maximum value instead of the minimum
 - Hits could sometimes stay at 0 after randomizing
 - Undoing a Polygene channel randomization could restore the wrong channel
 - Polygene's mono input behaviour option for the RND input wasn't saved with the patch
 - Possible crashes when loading patches or presets with invalid settings
 - Small memory leak when loading several presets in quick succession

## [2.0.4] - 2023-12-21
### Added
 - Ability to filter what is randomized
 - RND input
 - Display current channel number in Polygene
 - Snappy knobs (where max parameter value is known up front)
 - Options for how CV inputs behave with mono connections

### Fixed
 - EOC behaviour when running in reverse

## [2.0.3] - 2023-07-29
### Fixed
 - [#4](/../../issues/4) - Support for UTF-8 paths on Windows

## [2.0.2] - 2023-07-09
### Fixed
 - [#3](/../../issues/3) - Support for using Eugene and Polygene with 8FaceMk2

## [2.0.1] - 2021-12-28
### Added
 - Factory presets for Eugene and Polygene

### Fixed
 - Saved themes weren't loaded correctly after update to v2.0.0

## [2.0.0] - 2021-12-04
### Added
 - 3 Beat mode options to Eugene and Polygene
   - Pulse - 1ms pulse on an on beat
   - Gate - Output held for the duration of the beat and retriggered
   - Hold - Output is held high if the beat is on and low if a beat is off
 - 2 Sync modes added to Polygene
   - Individual channels - Each channel synced independently
   - All channels - Any trigger on any channel resets all channels

## [1.2.0] - 2021-02-27
### Added
 - Polygene - a new polyphonic rhythm generator module
 - End of cycle output for Polygene and Eugene
   - 3 behaviour options in the right click menu (On Repeat, First, Last)

### Changed
 - Eugene's Euclidean rhythms are now based on the Polygene ones, old patches may need the shift knob rotating to keep the same rhythm

### Fixed
 - The Eugene sync input only triggered if the clock and sync was rising at the same time, now a rising edge on sync with reset the rhythm regardless of where it occurs in relation to the clock
 - Reversed rhythms were playing the first beat of the rhythm first rather than the last beat of the rhythm

## [1.1.1] - 2021-01-23
### Added
 - Light and dark modes

### Changed
 - Panel design updates
 - Logo and graphics licencing

## [1.1.0] - 2020-12-23
 - Initial Release

[2.0.4]: https://github.com/RareBreeds/Orbits/compare/v2.0.3...v2.0.4
[2.0.3]: https://github.com/RareBreeds/Orbits/compare/v2.0.2...v2.0.3
[2.0.2]: https://github.com/RareBreeds/Orbits/compare/v2.0.1...v2.0.2
[2.0.1]: https://github.com/RareBreeds/Orbits/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/RareBreeds/Orbits/compare/v1.2.0...v2.0.0
[1.2.0]: https://github.com/RareBreeds/Orbits/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/RareBreeds/Orbits/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/RareBreeds/Orbits/releases/tag/v1.1.0
