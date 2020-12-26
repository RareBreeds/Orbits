def euclideanRhythm(length, hits):
    if hits < 0 or hits > length:
        raise ValueError("hits must be >= 0 and <= length")

    if hits == 0:
        pattern = [False] * length
    elif hits == length:
        pattern = [True] * length
    else:
        left_blocks = hits
        left_pattern = [True]

        right_blocks = length - left_blocks
        right_pattern = [False]

        # The loop can be terminated when the remainder consists of only 1 sequence.
        # Note that here we proceed one step further until right_blocks is 0, this results
        # in the same rhythm as terminating at a remainder of 1 but rotated.
        while right_blocks > 0:
            if left_blocks > right_blocks:
                remainder_blocks = left_blocks - right_blocks
                remainder_pattern = list(left_pattern)
                left_blocks = right_blocks
            else:
                remainder_blocks = right_blocks - left_blocks
                remainder_pattern = list(right_pattern)

            left_pattern.extend(right_pattern)
            right_blocks = remainder_blocks
            right_pattern = remainder_pattern

        pattern = left_pattern * left_blocks + right_pattern * right_blocks

    assert len(pattern) == length, "Bad pattern length"
    assert sum(pattern) == hits, "Bad number of hits"
    return pattern

def _asHex(rhythm):
    return hex(int(''.join('1' if p else '0' for p in reversed(rhythm)), 2))

def _asStr(euclid):
    return ''.join('x' if b else '.' for b in euclid)

def euclideanRhythmStr(length, hits):
    '''
    These tests are for rhythms terminating then the remainder is 0.

    The paper on Euclidean Rhythms terminates at 1 so these results are a rotation of the
    ones in the paper.
    http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf

    >>> euclideanRhythmStr(13, 5)
    'x..x.x..x..x.'

    >>> euclideanRhythmStr(8, 3)
    'x..x.x..'

    >>> euclideanRhythmStr(8, 5)
    'x.xx.x.x'

    >>> euclideanRhythmStr(2, 1)
    'x.'

    >>> euclideanRhythmStr(3, 1)
    'x..'

    >>> euclideanRhythmStr(4, 1)
    'x...'

    >>> euclideanRhythmStr(12, 4)
    'x..x..x..x..'

    >>> euclideanRhythmStr(3, 2)
    'x.x'

    >>> euclideanRhythmStr(5, 2)
    'x..x.'

    >>> euclideanRhythmStr(4, 3)
    'x.xx'

    >>> euclideanRhythmStr(5, 3)
    'x.xx.'

    >>> euclideanRhythmStr(7, 3)
    'x..x.x.'

    >>> euclideanRhythmStr(8, 3)
    'x..x.x..'

    >>> euclideanRhythmStr(16, 9)
    'x.xx.x.x.x.xx.x.'

    >>> euclideanRhythmStr(24, 13)
    'x.xx.x.x.x.x.x.xx.x.x.x.'

    >>> euclideanRhythmStr(1, 0)
    '.'

    >>> euclideanRhythmStr(1, 1)
    'x'

    >>> euclideanRhythmStr(8, 0)
    '........'

    >>> euclideanRhythmStr(8, 8)
    'xxxxxxxx'
    '''
    return _asStr(euclideanRhythm(length, hits))

def svg(length=32):
    size = 768
    svg = '<?xml version="1.0"?>\n'
    svg += f'<svg width="{size:.0f}" height="{size:.0f}" version="1.1" xmlns="http://www.w3.org/2000/svg">\n'
    svg += f'<g transform="scale({size/(length + 1)})">\n'
    for h in range(length + 1):
        for i, on in enumerate(euclideanRhythm(length, h)):
            if on:
                svg += f'<rect x="{i}" y="{h}" width="1" height="1" rx="0.3" fill="black"/>\n'
            else:
                svg += f'<circle cx="{i + 0.5}" cy="{h + 0.5}" r="0.1" fill="black"/>\n'
    svg += '</g>\n'
    svg += '</svg>\n'
    print(svg)

if __name__ == "__main__":
    import doctest
    doctest.testmod()
    svg()
