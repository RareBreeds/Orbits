## [Unreleased]
 - No changes
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

[Unreleased]: https://github.com/RareBreeds/Orbits/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/RareBreeds/Orbits/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/RareBreeds/Orbits/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/RareBreeds/Orbits/releases/tag/v1.1.0
