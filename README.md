# Orbits
VCV Rack Plugin

<a href='https://ko-fi.com/E1E63S8XF' target='_blank'><img height='36' style='border:0px;height:36px;' src='https://cdn.ko-fi.com/cdn/kofi5.png?v=2' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a>

## Modules
  * [Eugene](https://rarebreeds.github.io/Orbits/#eugene) - Euclidean Rhythm Generator
  * [Polygene](https://rarebreeds.github.io/Orbits/#polygene) - Polyphonic Rhythm Generator

## Building

    git clone https://github.com/RareBreeds/Orbits.git && cd Orbits && make

See https://vcvrack.com/manual/Building#building-rack-plugins for how to build VCV Rack plugins in general.

## Testing

    make -C tests
    make -C tests rack RACK_DIR=$HOME/Rack-SDK

The first builds and runs the unit tests that don't need VCV Rack. The second builds and runs the tests that use Rack, such as patch loading and the modules' outputs, against a Rack SDK or a Rack source directory with libRack built; give RACK_DIR as an absolute path. Both use AddressSanitizer and UndefinedBehaviorSanitizer, and [doctest](https://github.com/doctest/doctest), which is included in `tests/doctest/`.

## Manual
https://rarebreeds.github.io/Orbits/
