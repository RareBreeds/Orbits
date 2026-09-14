import re
import sys
import xml.etree.ElementTree as ET

SVG = "{http://www.w3.org/2000/svg}"
INKSCAPE = "{http://www.inkscape.org/namespaces/inkscape}"

def enum_name(label):
    """Convert an Inkscape label into a C enum identifier."""
    name = re.sub(r"[^a-zA-Z0-9]+", "_", label)
    name = re.sub(r"_+", "_", name)
    return name.strip("_").upper()


def float_value(value):
    """Convert an SVG number to a C float literal."""
    if value is None:
        return "0.0f"

    return f"{float(value):g}f"

module = sys.argv[1]
svg_file = sys.argv[2]
output_file = sys.argv[3]

tree = ET.parse(svg_file)
root = tree.getroot()

# Find the layer labelled "components".
components = next(
    (
        element
        for element in root.iter(SVG + "g")
        if element.get(INKSCAPE + "label") == "components"
    ),
    None,
)

if components is None:
    raise RuntimeError('Could not find group labelled "components"')

elements = []

for element in components.iter():
    label = element.get(INKSCAPE + "label")

    if not label:
        continue

    if element.tag == SVG + "circle":
        geometry = (
            float_value(element.get("cx")),
            float_value(element.get("cy")),
            "0.0f",
            "0.0f",
        )
        element_type = "circle"

    elif element.tag == SVG + "ellipse":
        geometry = (
            float_value(element.get("cx")),
            float_value(element.get("cy")),
            "0.0f",
            "0.0f",
        )
        element_type = "ellipse"

    elif element.tag == SVG + "rect":
        geometry = (
            float_value(element.get("x")),
            float_value(element.get("y")),
            float_value(element.get("width")),
            float_value(element.get("height")),
        )
        element_type = "rect"

    else:
        continue

    elements.append((label, element_type, geometry))


with open(output_file, "w") as out:
    out.write(f"""\
#pragma once

#include "OrbitsComponent.h"

typedef enum {module.capitalize()}Component
{{
""")

    for label, _, _ in elements:
        out.write(f"        {module.upper()}_COMPONENT_{enum_name(label)},\n")

    out.write(f"""\
    {module.upper()}_COMPONENT_COUNT
}} {module.capitalize()}Component;

static const OrbitsComponentGeometry g_{module.lower()}ComponentGeometry[{module.upper()}_COMPONENT_COUNT] = {{
""")

    for label, element_type, geometry in elements:
        x, y, width, height = geometry

        out.write(
            f"        [{module.upper()}_COMPONENT_{enum_name(label)}] = "
            f"{{ {x}, {y}, {width}, {height} }},"
            f"  /* {element_type} */\n"
        )

    out.write(f"""\
}};
""")
