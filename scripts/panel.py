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

with open(output_file, "w") as out:
    out.write(f"""\
#pragma once

""")

    for element in components.iter():
        label = element.get(INKSCAPE + "label")

        if not label:
            continue

        if element.tag == SVG + "circle" or \
           element.tag == SVG + "ellipse":
            x = float_value(element.get("cx"))
            y = float_value(element.get("cy"))
            out.write(
                f"        #define {module.upper()}_{enum_name(label)}_POS_X ({x})\n"
                f"        #define {module.upper()}_{enum_name(label)}_POS_Y ({y})\n"
            )

        elif element.tag == SVG + "rect":
            x = float_value(element.get("x"))
            y = float_value(element.get("y"))
            width = float_value(element.get("width"))
            height = float_value(element.get("height"))            
            out.write(
                f"        #define {module.upper()}_{enum_name(label)}_POS_X ({x})\n"
                f"        #define {module.upper()}_{enum_name(label)}_POS_Y ({y})\n"
                f"        #define {module.upper()}_{enum_name(label)}_POS_WIDTH ({width})\n"
                f"        #define {module.upper()}_{enum_name(label)}_POS_HEIGHT ({height})\n"                
            )
