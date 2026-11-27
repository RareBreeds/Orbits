# Orbits
Euclidean rhythm generators for [VCV Rack](https://vcvrack.com/).

<a href='https://ko-fi.com/E1E63S8XF'><img height='36' style='border:0px;height:36px;' src='https://cdn.ko-fi.com/cdn/kofi5.png?v=2' border='0' alt='Buy Me a Coffee at ko-fi.com' /></a>

## Modules
- [Eugene](manual/RareBreeds_Orbits_Eugene.md) - Euclidean Rhythm Generator
- [Polygene](manual/RareBreeds_Orbits_Polygene.md) - Polyphonic Rhythm Generator

## Manual
The user manual is in [`manual/`](manual/plugin.md), in [VCV Rack's manual format](https://vcvrack.com/manual/PluginGuide#User-manuals). It's packaged with the plugin and shown on the [VCV Library](https://library.vcvrack.com/RareBreeds_Orbits).

See the [changelog](CHANGELOG.md) for what's changed in each version.

## Building

    git clone https://github.com/RareBreeds/Orbits.git
    cd Orbits
    RACK_DIR=<path to the Rack SDK> make

`RACK_DIR` can be left out when Orbits is cloned into the `plugins/` directory of a Rack source tree. `make install` installs the plugin into your Rack user folder, and `make dist` builds a `.vcvplugin` package in `dist/`. Add `WERROR=1` to treat warnings as errors, as CI does. See [Building Rack plugins](https://vcvrack.com/manual/Building#building-rack-plugins) for more on building VCV Rack plugins in general.

The same sources also build for the [4ms MetaModule](https://4mscompany.com/metamodule). That build lives in [Orbits-MetaModule](https://github.com/RareBreeds/Orbits-MetaModule), which includes this repository as a submodule.

## Testing

    make -C tests

builds and runs the unit tests that don't need VCV Rack, after checking the panel and component SVGs with `xmllint`. The other targets build against Rack, so give them `RACK_DIR` as an absolute path to a Rack SDK, or a Rack source directory with libRack built:

- `make -C tests rack RACK_DIR=...`: tests that use Rack, such as patch loading and the modules' outputs.
- `make -C tests widgets RACK_DIR=...`: opens a Rack window and checks both modules' widgets in each theme: every param, input and output has a widget, everything is drawn inside the panel, and each widget draws something. Needs a display.
- `make -C tests screenshots RACK_DIR=...`: saves PNG screenshots of both modules in each theme, bypassed or not and with every button off or on, to `tests/build/screenshots/` (or `SCREENSHOT_DIR=...`). Needs a display.
- `make -C tests compare RACK_DIR=...`: runs scenarios in Rack's engine and compares every output change, sample by sample, with the last release, and loads patches saved by that release. Expected differences are marked in the scenarios. It then runs everything again with sanitizers, along with randomly generated invalid patch data. Logs are in `tests/build/compare/`.
- `make -C tests all RACK_DIR=...`: runs the unit tests, `rack`, `widgets` and `compare` in turn, stopping at the first failure.
- `make -C tests bench RACK_DIR=...`: engine and drawing benchmarks.

The unit, `rack` and `widgets` tests use AddressSanitizer and UndefinedBehaviorSanitizer. The tests use [doctest](https://github.com/doctest/doctest), which is included in `tests/doctest/`; the only other dependency is `xmllint`.

## License
The source code is licensed under the GPLv3, and the graphics under CC BY-NC-ND 4.0. See [LICENSE.md](LICENSE.md) for details, including the licenses of the font and doctest.
