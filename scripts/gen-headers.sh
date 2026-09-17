python3 scripts/panel.py eugene res/eugene-dark-panel.svg src/EugeneComponentPositions.h
clang-format -i src/EugeneComponentPositions.h
python3 scripts/panel.py polygene res/polygene-dark-panel.svg src/PolygeneComponentPositions.h
clang-format -i src/PolygeneComponentPositions.h
python3 scripts/assets.py --output src/OrbitsAssets.hpp res
clang-format -i src/OrbitsAssets.hpp
