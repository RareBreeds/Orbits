#!/usr/bin/python3

import math
import itertools

def _necklacesOfLengthAndDensity(n, d, k=2):
    '''
    http://combos.org/necklace
    '''
    def printd(p):
        nex = (d // p) * a[p] + a[d % p]
        if nex < n:
            return
        m = 1
        if nex == n and d % p:
            m = b[d % p] + 1
            p = d
        elif nex == n and d % p == 0:
            m = b[p]
        end = n

        b[d] = m
        while b[d] < k:
            i = 1
            y = []
            for j in range(1, end + 1):
                if a[i] == j:
                    y.append(b[i])
                    i += 1
                else:
                    y.append(0)
            yield y
            b[d] += 1
            p = d

    def gen(t, p):
        if t >= d - 1:
            yield from printd(p)
        else:
            tail = n - (d - t) + 1
            m = a[t-p+1] + a[p]
            if m <= tail:
                a[t+1] = m
                b[t+1] = b[t-p+1]
                yield from gen(t+1, p)
                for i in range(b[t+1] + 1, k):
                    b[t+1] = i
                    yield from gen(t+1,t+1)
                tail = m-1
            for j in range(tail, a[t], -1):
                a[t+1] = j
                for i in range(1, k):
                    b[t+1] = i
                    yield from gen(t+1, t+1)

    a = [0] * (n + 1)
    if d == 0:
        yield a[1:]
    elif d == 1:
        a[-1] = 1
        yield a[1:]
    else:
        b = [0] * (n + 1)
        a[d] = n
        for j in range(n - d + 1, (n - 1) // d, -1):
            a[1] = j
            for i in range(1, 2):
                b[1] = i
                yield from gen(1, 1)

def _necklacesOfLength(n, k=2):
    '''
    http://combos.org/necklace
    '''
    def gen(t, p):
        if t > n:
            if n % p == 0:
                yield a[1:]
        else:
            a[t] = a[t - p]
            yield from gen(t+1, p)
            for j in range(a[t-p] + 1, k):
                a[t] = j
                yield from gen(t+1, t)
    a = [0]*(n + 1)
    yield from gen(1, 1)

def _lyndonWords(n):
    '''
    Generate nonempty Lyndon words of length <= n over a 2-symbol alphabet.
    The words are generated in lexicographic order, using an algorithm from
    J.-P. Duval, Theor. Comput. Sci. 1988, doi:10.1016/0304-3975(88)90113-2.
    As shown by Berstel and Pocchiola, it takes constant average time
    per generated word.
    '''
    w = [-1] # set up for first increment
    while w:
        w[-1] += 1 # increment the last non-z symbol
        yield w
        m = len(w)
        while len(w) < n: # repeat word to fill exactly n syms
            w.append(w[-m])
        while w and w[-1]: # delete trailing z's
            w.pop()

class Rhythm:
    @classmethod
    def lyndonWordsOfLength(cls, length):
        for l in _lyndonWords(length):
            if len(l) == length:
                yield Rhythm(reversed(l))

    @classmethod
    def lyndonWordsOfLengthAndDensity(cls, length, density):
        for l in _lyndonWords(length):
            if len(l) == length and l.count(1) == density:
                yield Rhythm(reversed(l))

    @classmethod
    def necklacesOfLength(cls, length):
        for n in _necklacesOfLength(length):
            yield Rhythm(reversed(n))

    @classmethod
    def necklacesOfLengthAndDensity(cls, length, density):
        for n in _necklacesOfLengthAndDensity(length, density):
            yield Rhythm(reversed(n))

    @classmethod
    def allOfLength(cls, length):
        for r in itertools.product([False, True], repeat=length):
            yield Rhythm(reversed(r))

    @classmethod
    def allOfLengthAndDensity(cls, length, density):
        for r in itertools.product([False, True], repeat=length):
            if sum(r) == density:
                yield Rhythm(reversed(r))

    @classmethod
    def maximallyEven(cls, length, density):
        '''
        http://cgm.cs.mcgill.ca/~godfried/publications/Percussive-Notes-Web.pdf

        Looks to me like the maximally even rhythm is always the euclidean rhythm, although usually with a different rotation.
        If you plot all euclidean rhythms for each density for a particular length you can see that the maximally even rhythm
        creates a more symetrical pattern.

        The euclidean / maximally even rhythm is also always the last necklace in an enumerated necklace list.
        '''
        rounded_beats = [d if m * 2 < length else d + 1 for d, m in Rhythm.optimalEvenBeats(length, density)]
        rhythm = Rhythm.fromOnsetList(length, rounded_beats)

        assert rhythm.density == density, "Bad density"
        assert rhythm.isRotationOf(Rhythm.euclidean(length, density)), "I thought these would always be equal"

        return rhythm

    @classmethod
    def euclidean(cls, length, density):
        '''
        http://cgm.cs.mcgill.ca/~godfried/publications/banff.pdf
        '''
        if density < 0 or density > length:
            raise ValueError("density must be >= 0 and <= length")

        if density == 0:
            pattern = [False] * length
        elif density == length:
            pattern = [True] * length
        else:
            left_blocks = density
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

        rhythm = Rhythm(pattern)
        assert len(rhythm) == length, "Bad pattern length"
        assert rhythm.density == density, "Bad pattern density"
        return rhythm

    @classmethod
    def fromOnsetList(cls, length, onset_list):
        return Rhythm(True if b in onset_list else False for b in range(length))

    def __init__(self, rhythm):
        '''
        Accepts the following rhythm initialisation formats:
        "10110100"
        "x.xx.x.."
        "X.XX.X.."
        [1, 0, 1, 1, 0, 1, 0, 0]
        [True, False, True, True, False, True, False, False]
        '''
        self._rhythm = Rhythm.canonicalise(rhythm)

    @property
    def length(self):
        return len(self._rhythm)

    @property
    def density(self):
        return self.onsets

    @property
    def onsets(self):
        return self._rhythm.count(True)

    @property
    def onset_list(self):
        '''
        Produces a list of the position of each onset in the rhythm.
        The length of the rhythm is lost in this representation.
        '''
        return [i for i,b in enumerate(self) if b]

    @property
    def off_beat_onsets(self):
        '''
        https://research.cs.queensu.ca/home/daver/Pubs/MyPDF/MeasureSycopa.pdf

        Sum the onset beats that are off-beat and divide by the maximum possible number of off-beats.
        Beats are off-beat if they are a generator for the cyclic-group of order n where n is the length of the rhythm.
        Generators are integers that are less than n and relatively prime to n (gcd(n, g) == 1).
        (see https://sites.millersville.edu/bikenaga/abstract-algebra-1/cyclic-groups/cyclic-groups.pdf)

        Unlike a number of other measures, the off-beatness of a rhythm does depend on the start beat so varies with rotation.
        '''
        return sum(1 for i,b in enumerate(self) if b and i in Rhythm.offBeats(len(self)))

    @property
    def on_beat_onsets(self):
        '''
        Beats that are not off-beat are on-beat.
        '''
        return self.onsets - self.off_beat_onsets

    @property
    def off_beatness(self):
        try:
            return self.off_beat_onsets / self.onsets
        except ZeroDivisionError:
            return 0.0

    @property
    def on_beatness(self):
        return 1.0 - self.off_beatness

    @property
    def is_odd(self):
        '''
        Not related to the rhythm's evenness, see:
        http://cgm.cs.mcgill.ca/~godfried/publications/Percussive-Notes-Web.pdf
        '''

        # Odd length rhythms are always rhythmically odd as the opposite on the rhythm circle never lines up with a beat
        # https://research.cs.queensu.ca/home/daver/Pubs/MyPDF/MeasureSycopa.pdf
        if len(self) % 2:
            return True

        return not any(a == b == True for a, b in zip(self[:len(self)//2], self[len(self)//2:]))

    @property
    def evenness(self):
        '''
        https://www.researchgate.net/publication/249881698_Vector_Products_and_Intervallic_Weighting
        '''
        w = [2 * math.sin(k * math.pi / len(self)) for k in range(1, len(self) // 2 + 1)]
        assert len(self.inter_onset_vector) == len(w), "Bad lengths"
        return sum(a * b for a, b in zip(w, self.inter_onset_vector))

    @property
    def inter_onset_vector(self):
        '''
        https://en.wikipedia.org/wiki/Interval_vector
        A rhythm will have the same inter-onset vector as its reversal.
        '''
        iv = [0] * (self.length // 2)
        for i,b1 in enumerate(self):
            if b1:
                for j,b2 in enumerate(self[i+1:], i+1):
                    if b2:
                        diff = j - i
                        if diff > len(self) // 2:
                            iv[len(self) - diff - 1] += 1
                        else:
                            iv[diff-1] += 1
        return iv

    @property
    def adjacent_inter_onset_vector(self):
        vec = []
        first_on = -1
        prev = -1
        for i,c in enumerate(self):
            if c:
                if first_on == -1:
                    first_on = i
                else:
                    vec.append(i - prev)
                prev = i
        if first_on != -1:
            vec.append((len(self) - prev) + first_on)

        assert len(vec) == self.density, "Incorrectly generated adjacent inter-onset vector"
        assert self.density == 0 or sum(vec) == len(self), "Incorrectly generated adjacent inter-onset vector"

        return vec

    @property
    def period(self):
        if len(self) == 0:
            return 0

        period = 1
        for i,r in enumerate(self):
            if self[i % period] != r:
                period = i + 1

        if len(self) % period:
            period = len(self)

        return period

    @property
    def is_euclidean(self):
        return self in Rhythm.euclidean(len(self), self.density)

    @property
    def is_maximally_even(self):
        return self in Rhythm.maximallyEven(len(self), self.density)

    def isRotationOf(self, rhythm):
        return len(self) == len(rhythm) and self in rhythm

    def svgCircle(self):
        rhythm_size = 500
        border_scale = 0.05
        background_colour = "none"
        stroke_colour = "black"
        max_beat_radius = 0.03
        stroke_width = 0.005

        beat_radius = min(0.95 / len(self), max_beat_radius)
        border = rhythm_size * border_scale
        svg_size = rhythm_size + 2 * border

        def angle(beat):
            return 360 * beat / len(self) - 180

        svg = '<?xml version="1.0"?>'
        svg += f'<svg width="{svg_size}" height="{svg_size}" version="1.1" xmlns="http://www.w3.org/2000/svg" style="background-color:{background_colour}">'
        svg += f'<g transform="translate({border} {border}) scale({rhythm_size} {rhythm_size}) translate(.5 .5)">'

        # Ring to join the beats
        svg += '<mask id="m">'
        svg += f'<rect x="-1" y="-1" width="2" height="2" fill="white"/>'
        circle = f'<circle r="{beat_radius}" cy="0.5" stroke="black" stroke-width="{stroke_width}" fill="black" '
        svg += ''.join(circle + f'transform="rotate({angle(i)})"/>' for i,_ in enumerate(self))
        svg += '</mask>'
        svg += f'<circle r="0.5" fill="none" mask="url(#m)" stroke="{stroke_colour}" stroke-width="{stroke_width}"/>'

        # Beats
        circle = f'<circle r="{beat_radius}" cy="0.5" stroke="{stroke_colour}" stroke-width="{stroke_width}" '
        svg += ''.join(circle + f'fill="{stroke_colour if r else "none"}" transform="rotate({angle(i)})"/>' for i,r in enumerate(self))

        svg += '</g>'
        svg += '</svg>'

        return svg

    def svgBoxes(self):
        box_size = 20
        border_scale = 0.05
        background_colour = "none"
        stroke_colour = "black"
        stroke_width = 0.05
        beat_radius = 0.3

        border = box_size * border_scale
        svg_height = svg_width = box_size * len(self) + 2 * border

        svg = '<?xml version="1.0"?>'
        svg += f'<svg width="{svg_width}" height="{svg_height}" version="1.1" xmlns="http://www.w3.org/2000/svg" style="background-color:{background_colour}">'
        svg += f'<g transform="translate({border} {border}) scale({box_size} {box_size})">'
        svg += ''.join(f'<rect x="{i}" width="1.0" height="1.0" fill="none" stroke-width="{stroke_width}" stroke="{stroke_colour}"/>' for i,r in enumerate(self))
        svg += ''.join(f'<circle r="{beat_radius}" cx="{i + 0.5}" cy="0.5" fill="{stroke_colour}" stroke-width="{stroke_width}" stroke="{stroke_colour}"/>' for i,r in enumerate(self) if r)
        svg += '</g>'
        svg += '</svg>'

        return svg

    def __mul__(self, other):
        return Rhythm(self._rhythm * other)

    def __rmul__(self, other):
        return Rhythm(other * self._rhythm)

    def __str__(self):
        return self.asStr(self._rhythm)

    def __repr__(self):
        return str(self)

    def __len__(self):
        return self.length

    def __iter__(self):
        return iter(self._rhythm)

    def __getitem__(self, index):
        return self._rhythm[index]

    def __int__(self):
        return self.asInt(self._rhythm)

    def __contains__(self, key):
        return str(key) in str(self * 2)

    def __eq__(self, other):
        return self._rhythm == other._rhythm

    @staticmethod
    def canonicalise(rhythm):
        return [b in {True, 'x', 'X', '1'} for b in rhythm]

    @staticmethod
    def asStr(rhythm):
        return ''.join('x' if b else '.' for b in rhythm)

    @staticmethod
    def asInt(rhythm):
        if not rhythm:
            raise ValueError("Can't convert 0 length rhythm to an integer.")
        return int("".join("1" if b else "0" for b in reversed(rhythm)), 2)

    @staticmethod
    def offBeats(length):
        # The off beat beats in a rhythm are defined here as the generators of
        # cyclic-group of order 'length'.
        # https://research.cs.queensu.ca/home/daver/Pubs/MyPDF/MeasureSycopa.pdf
        return set(i for i in range(length) if math.gcd(length, i) == 1)

    @staticmethod
    def optimalEvenBeats(length, density):
        return [(n // density, n % density) for n in range(0, density * length, length)]

if __name__ == "__main__":
    #print(Rhythm(Rhythm.maximallyEven(17, 6)).svgCircle())
    #print(Rhythm(Rhythm.maximallyEven(65, 38)).svgBoxes())
    #for length in range(1, 20):
    #    for density in range(length + 1):
    #        print(Rhythm.maximallyEven(length, density))
