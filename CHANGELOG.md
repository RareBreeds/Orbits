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
