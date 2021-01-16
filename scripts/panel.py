import fileinput
import re

FIND_LAYER = 0
FIND_COMPONENT = 1
FIND_TYPE = 2
FIND_CX = 3
FIND_CY = 4
FIND_LABEL = 5
state = FIND_LAYER

TYP_PARAM = 0
TYP_INPUT = 1
TYP_OUTPUT = 2
TYP_UNKNOWN = 3

for line in fileinput.input():
    if state == FIND_LAYER:
        if 'inkscape:label="components"' in line:
            state = FIND_COMPONENT
    elif state == FIND_COMPONENT:
        if 'circle' in line or 'ellipse' in line:
            state = FIND_TYPE
    elif state == FIND_TYPE:
        if "style=" in line:
            fill = re.search(r"fill:#([0-9a-f]+);", line).group(1)
            if fill == "ff0000":
                typ = TYP_PARAM
            elif fill == "00ff00":
                typ = TYP_INPUT
            elif fill == "0000ff":
                typ = TYP_OUTPUT
            else:
                typ = TYP_UNKNOWN
        state = FIND_CX
    elif state == FIND_CX:
        if 'cx=' in line:
            cx = float(line.split('"')[1])
            state = FIND_CY
    elif state == FIND_CY:
        if 'cy=' in line:
            cy = float(line.split('"')[1])
            state = FIND_LABEL
    elif state == FIND_LABEL:
        if 'inkscape:label=' in line:
            label = line.split('"')[1]
            var = "_".join([label, "pos"])
            decl = f"static const Vec {var} = mm2px(Vec({cx:.2f}, {cy:.2f}));"
            print(decl)
            state = FIND_COMPONENT
