# openzone-games/tools/make_cards.py
"""Draw the playing cards of OpenZone Games: two decks, their atlases, the sheets for the models.

Every picture of `classic` is drawn here, from polygons, circles and curves: no
image is read, nothing is traced from another deck. The suits are shapes of this script; only
the rank letters come from a font. The `stalker` deck is drawn the same way, and then twenty
of its cards carry a picture read from art/cards/stalker/src: stalkers and mutants cut from
the files of the S.T.A.L.K.E.R. games, which are GSC Game World's and not ours (NOTICE says
on what terms they are here; SOURCES.md in that folder says where each one comes from). The
same run always gives the same pictures: all chance is seeded from the style and the
picture's name.

    python -I tools/make_cards.py                # every style, everything
    python -I tools/make_cards.py --style stalker   # one style
    python -I tools/make_cards.py --no-paa       # stop at the PNG files and the .imageset

Needs Python 3.9+ and Pillow 10.1+ (written against 3.13 and 12.3); the .paa files need
DayZ Tools (ImageToPAA). The pictures are committed: should another Pillow or FreeType
move a pixel, the diff shows it.

Two styles: `classic`, a clean deck, and `stalker`, a worn one whose every card is worn its
own way and whose aces, kings, queens, jacks and tens carry pictures. Who is on which card
is the table PICTURES below: a line a card. (`zone`, the worn deck of the first phases, left
the mod on 2026-11-24; its palette is the ground the stalkers' deck is drawn on.) For each
style the run writes

    art/cards/<style>/<name>.png       57 pictures, 200 x 280, RGBA, round corners: the sources
    art/cards/<style>/deck_sheet.png   512 x 512, opaque, for the 3D deck: the back on the left
                                       half, a blank face over an edge band on the right
    art/cards/<style>/_sheet.png       the 57 on grey, to look at
    OpenZone_Games/gui/imagesets/ozg_cards_<style>.imageset   and ozg_cards_<style>_ca.paa
    OpenZone_Games/data/cards/ozg_deck_<style>_co.paa
    OpenZone_Games/data/cards/<style>/ozg_<style>_<face>_co.paa and ozg_<style>_back_co.paa
                                       256 x 512, DXT1: one card's face, and the back, for the
                                       3D card (models/src/card.py), which wears one on each side.
                                       The style is in the file's NAME, not only in its folder:
                                       the game took two textures of one file name in two
                                       folders for one texture (card_file below)
    build/cards/                       the PNG files the .paa are made from (not committed)

The 57 names: `c_<rank><suit>` with rank one of 2 3 4 5 6 7 8 9 T J Q K A and suit one of
s h d c, then `back`, then `suit_s`, `suit_h`, `suit_d`, `suit_c`. They are the image names
of the imageset too, so a widget takes a card as "set:ozg_cards_classic image:c_6s". The
set is listed in config.cpp (CfgMods > defs > imageSets > files[]) or it draws nothing.

The atlas is 2048 x 2048, ten cells a row, the pictures in the order of the names above.
Every cell starts on a multiple of four, so no DXT block lies across a card's edge, nor
does a texel of the next two mip levels; four transparent pixels part the columns and
fifty-six the rows. The pixels of a gap carry the colour of the card beside them under an
alpha of zero: filtering then mixes in that colour, not black.

Font: Oswald, the variable font drawn at weight 700 (art/fonts/oswald/Oswald[wght].ttf, a
copy of the one in openzone-research/source/fonts/oswald). Copyright 2016 The Oswald Project
Authors; SIL Open Font License 1.1, the text is beside the file (OFL.txt).
"""
from __future__ import annotations

import argparse
import math
import os
import random
import re
import subprocess
import sys
import zlib
from dataclasses import dataclass, replace
from functools import lru_cache
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parents[1]
MOD = "OpenZone_Games"
ART = ROOT / "art" / "cards"
SETS = ROOT / MOD / "gui" / "imagesets"
DECKS = ROOT / MOD / "data" / "cards"
BUILD = ROOT / "build" / "cards"

FONT_FILE = "Oswald[wght].ttf"
FONT_WEIGHT = 700
FONT_DIRS = (ROOT / "art" / "fonts" / "oswald",
             ROOT.parent / "openzone-research" / "source" / "fonts" / "oswald")

W, H = 200, 280            # a card, px
S = 4                      # everything is drawn S times larger and reduced
CORNER = 13.0              # the radius of a card's corner
EDGE = 1.5                 # the line round a card
CX, CY = W / 2, H / 2

RANKS = "23456789TJQKA"
SUITS = "shdc"
NAMES = [f"c_{r}{s}" for s in SUITS for r in RANKS] + ["back"] + [f"suit_{s}" for s in SUITS]

ATLAS = 2048
COLS, ROWS = 10, 6
CELL_X0, CELL_Y0 = 4, 8            # the first cell
PITCH_X, PITCH_Y = 204, 336        # from one cell to the next
BLEED = 2                          # how far a card's colour runs into the gap, alpha 0

LANCZOS = Image.Resampling.LANCZOS
BICUBIC = Image.Resampling.BICUBIC
BOX = Image.Resampling.BOX
NEAREST = Image.Resampling.NEAREST


# ----------------------------------------------------------------------------- styles


@dataclass(frozen=True)
class Style:
    name: str
    worn: bool
    paper: tuple[int, int, int]        # the stock
    edge: tuple[int, int, int]         # the line round a card
    black: tuple[int, int, int]        # spades and clubs
    red: tuple[int, int, int]          # hearts and diamonds
    gold: tuple[int, int, int]         # the trim of the court cards
    tints: dict[str, tuple[int, int, int]]     # the ground of a court card's panel, by rank
    back: tuple[int, int, int]         # the ground of the back
    back_deep: tuple[int, int, int]    # the field in its middle
    line: tuple[int, int, int]         # the lines of the back
    back_light: tuple[int, int, int] = (0, 0, 0)    # the filled cells of the back's pattern
    mark: tuple[int, int, int] = (0, 0, 0)      # the ring of the mark on the back
    accent: tuple[int, int, int] = (0, 0, 0)    # its dot and wave
    gap: tuple[int, int, int] = (0, 0, 0)       # between two cards, seen from the side of a stack
    shade: tuple[int, int, int] = (0, 0, 0)     # the wear: stock gone darker,
    light: tuple[int, int, int] = (0, 0, 0)     # stock scuffed pale,
    grime: tuple[int, int, int] = (0, 0, 0)     # dirt,
    stain: tuple[int, int, int] = (0, 0, 0)     # and what was spilt
    uneven: bool = False               # a worn deck whose every card draws its own lot of wear

    def ink(self, suit: str) -> tuple[int, int, int]:
        return self.red if suit in "hd" else self.black


CLASSIC = Style(
    name="classic", worn=False,
    paper=(250, 246, 232), edge=(126, 116, 98),
    black=(24, 22, 24), red=(200, 26, 40), gold=(224, 172, 52),
    tints={"J": (214, 228, 216), "Q": (244, 216, 214), "K": (246, 230, 176)},
    back=(104, 16, 36), back_deep=(84, 10, 28), line=(250, 244, 230),
    mark=(250, 244, 230), accent=(79, 181, 232),     # the mark as the radios carry it: a pale ring, 4FB5E8
    gap=(170, 162, 146),
)

ZONE = Style(
    name="zone", worn=True,
    paper=(220, 203, 154), edge=(98, 84, 56),
    black=(46, 42, 36), red=(152, 52, 38), gold=(178, 138, 60),
    tints={"J": (184, 190, 148), "Q": (214, 176, 150), "K": (216, 190, 122)},
    back=(88, 94, 54), back_deep=(62, 68, 40), line=(192, 180, 126), back_light=(104, 110, 64),
    mark=(226, 216, 172), accent=(140, 178, 190),
    gap=(104, 90, 62),
    shade=(152, 124, 74), light=(238, 226, 190), grime=(72, 58, 36), stain=(126, 90, 42),
)

# The stalker deck: the stock and the inks of `zone`, a red back, a fourth tint for the tens,
# which carry a picture as the court cards do.
STALKER = replace(
    ZONE, name="stalker", uneven=True,
    tints=dict(ZONE.tints, T=(186, 196, 198)),
    back=(146, 50, 38), back_deep=(118, 38, 30), line=(226, 212, 176), back_light=(166, 68, 52),
    mark=(232, 220, 184), accent=(232, 220, 184),
)

# The decks the mod ships: the classic one and the stalkers'. `zone`, the worn deck of the
# first phases, is not one of them since 2026-11-24 (the owner's word: two decks); its
# palette stays, as the ground the stalkers' deck is drawn on.
STYLES = {s.name: s for s in (CLASSIC, STALKER)}


# ----------------------------------------------------------------------------- geometry


def cubic(p0, p1, p2, p3, n=22):
    out = []
    for i in range(n + 1):
        t = i / n
        u = 1 - t
        a, b, c, d = u * u * u, 3 * u * u * t, 3 * u * t * t, t * t * t
        out.append((a * p0[0] + b * p1[0] + c * p2[0] + d * p3[0],
                    a * p0[1] + b * p1[1] + c * p2[1] + d * p3[1]))
    return out


def quad(p0, p1, p2, n=14):
    out = []
    for i in range(n + 1):
        t = i / n
        u = 1 - t
        out.append((u * u * p0[0] + 2 * u * t * p1[0] + t * t * p2[0],
                    u * u * p0[1] + 2 * u * t * p1[1] + t * t * p2[1]))
    return out


def arc(cx, cy, r, a0, a1, n=30):
    """Points of a circle from angle a0 to a1, degrees, measured from three o'clock, y down."""
    out = []
    for i in range(n + 1):
        a = math.radians(a0 + (a1 - a0) * i / n)
        out.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return out


def mirrored(half):
    """A left half, from its top on the axis to its bottom on the axis, closed by its mirror image."""
    return half + [(-x, y) for x, y in reversed(half)]


def ngon(cx, cy, r, n, turn=-90.0):
    return [(cx + r * math.cos(math.radians(turn + 360 * i / n)),
             cy + r * math.sin(math.radians(turn + 360 * i / n))) for i in range(n)]


# ----------------------------------------------------------------------------- the suits
# Each suit is a list of polygons in a box one unit high, centred on (0, 0), y down.


def _stem(top, neck=0.035, foot=0.19, base=0.5):
    """The foot of a spade or a club: narrow under the leaves, flaring to a flat base."""
    return mirrored(quad((-neck, top), (-neck - 0.015, base - 0.10), (-foot, base)))


def _heart():
    r, cx, cy, tip = 0.27, 0.23, -0.19, 0.46
    dip = -math.degrees(math.acos(cx / r))           # where the two lobes meet on the axis
    lobe = arc(-cx, cy, r, dip, -200)                # over the top and down the left side
    x, y = lobe[-1]
    side = cubic((x, y), (x + 0.342 * 0.20, y + 0.940 * 0.20), (-0.60 * 0.22, tip - 0.80 * 0.22), (0, tip))
    return [mirrored(lobe + side[1:])]


def _spade():
    r, cx, cy = 0.245, 0.235, 0.105
    x, y = arc(-cx, cy, r, 200, 200, 1)[0]           # where the shoulder runs into the lobe
    shoulder = cubic((0, -0.5), (-0.0675, -0.2386), (x + 0.342 * 0.26, y - 0.940 * 0.26), (x, y))
    lobe = arc(-cx, cy, r, 200, 20)
    return [mirrored(shoulder + lobe[1:]), _stem(0.10)]


def _club():
    r = 0.23
    leaves = [ngon(0, -0.25, r, 48), ngon(-0.255, 0.10, r, 48), ngon(0.255, 0.10, r, 48)]
    middle = [(0, -0.25), (-0.255, 0.10), (0.255, 0.10)]       # between the three leaves
    return leaves + [middle, _stem(0.0)]


def _diamond():
    w, h, k = 0.385, 0.5, 0.86                       # k < 1 draws the sides in a little

    def side(a, b):
        return quad(a, ((a[0] + b[0]) / 2 * k, (a[1] + b[1]) / 2 * k), b)[:-1]

    top, right, bottom, left = (0, -h), (w, 0), (0, h), (-w, 0)
    return [side(top, right) + side(right, bottom) + side(bottom, left) + side(left, top)]


SHAPES = {"s": _spade(), "h": _heart(), "d": _diamond(), "c": _club()}


# ----------------------------------------------------------------------------- drawing


def px(v):
    """A card coordinate on the S canvas, to the nearest pixel.

    ImageDraw would cut the fraction off instead, which moves every shape a little up and
    to the left.
    """
    return round(v * S)


def rect(x0, y0, x1, y1):
    """A box of card coordinates as ImageDraw wants it: its last pixel counted in."""
    return px(x0), px(y0), px(x1) - 1, px(y1) - 1


class Layer:
    """A transparent sheet the size of a card, S times larger; coordinates are card pixels."""

    def __init__(self):
        self.im = Image.new("RGBA", (W * S, H * S), (0, 0, 0, 0))
        self.d = ImageDraw.Draw(self.im)

    @staticmethod
    def _pts(pts):
        return [(px(x), px(y)) for x, y in pts]

    def poly(self, pts, fill):
        self.d.polygon(self._pts(pts), fill=fill)

    def line(self, pts, fill, width, closed=False):
        p = self._pts(pts)
        if closed:
            p = p + p[:2]
        self.d.line(p, fill=fill, width=max(1, px(width)), joint="curve")

    def disc(self, cx, cy, r, fill):
        self.d.ellipse(rect(cx - r, cy - r, cx + r, cy + r), fill=fill)

    def circle(self, cx, cy, r, fill, width):
        """A ring whose outer radius is r."""
        self.d.ellipse(rect(cx - r, cy - r, cx + r, cy + r), outline=fill, width=max(1, px(width)))

    def rrect(self, x0, y0, x1, y1, radius, fill=None, outline=None, width=0.0):
        self.d.rounded_rectangle(rect(x0, y0, x1, y1), radius=radius * S,
                                 fill=fill, outline=outline, width=max(1, px(width)) if outline else 0)

    def stamp(self, mask, x, y, fill):
        """A mask of the S canvas (a glyph) laid down in one colour, its corner at card (x, y)."""
        tile = Image.new("RGBA", mask.size, fill)
        tile.putalpha(mask)
        self.im.alpha_composite(tile, (px(x), px(y)))

    def add(self, other: "Layer"):
        self.im.alpha_composite(other.im)

    def add_turned(self, other: "Layer"):
        """The other sheet upside down: what the top of a card says, its bottom says turned."""
        self.im.alpha_composite(other.im.transpose(Image.Transpose.ROTATE_180))

    def clip(self, mask):
        self.im.putalpha(ImageChops.multiply(self.im.getchannel("A"), mask))


def window(x0, y0, x1, y1, radius):
    """A mask of the S canvas: white inside the rounded box."""
    m = Image.new("L", (W * S, H * S), 0)
    ImageDraw.Draw(m).rounded_rectangle(rect(x0, y0, x1, y1), radius=radius * S, fill=255)
    return m


def ramp(img, lo, hi, top=255):
    """lo..hi stretched to 0..top and clamped: a soft threshold."""
    span = max(1, hi - lo)
    return img.point([max(0, min(top, round((v - lo) * top / span))) for v in range(256)])


def flat(alpha):
    """An alpha after the reduction, its two ends pressed flat.

    Lanczos leaves a faint ripple on both sides of an edge: a few levels of alpha where
    nothing is, a few short of full where everything is. Clear is made clear again and
    solid solid; what lies between is left as it is.
    """
    return alpha.point([0 if v < 5 else 255 if v > 250 else v for v in range(256)])


def suit_at(layer, suit, cx, cy, size, fill, flip=False):
    """A suit `size` card pixels high, centred on (cx, cy); `flip` turns it upside down.

    The turned suit is the upright one's half turn to the pixel: the same whole offsets
    from the centre with their signs changed, and one pixel back, because ImageDraw
    counts a pixel in at both ends of a span. A number card then is its own half turn
    wherever custom wants it to be.
    """
    ox, oy, k = (px(cx) - 1, px(cy) - 1, -1) if flip else (px(cx), px(cy), 1)
    for pts in SHAPES[suit]:
        layer.d.polygon([(ox + k * px(x * size), oy + k * px(y * size)) for x, y in pts], fill=fill)


def band(layer, cx, cy, r, a0, a1, width, fill):
    """An arc `width` thick with round ends; r is its outer radius, angles as in arc()."""
    layer.poly(arc(cx, cy, r, a0, a1, 48) + arc(cx, cy, r - width, a1, a0, 48), fill)
    for a in (a0, a1):
        x, y = arc(cx, cy, r - width / 2, a, a, 1)[0]
        layer.disc(x, y, width / 2, fill)


def oz_mark(layer, cx, cy, r, ring, accent):
    """The mark of the series: an open ring, a dot at its centre, a wave out through the gap.

    The proportions are those of the mark the radios carry (openzone-core,
    source/icons/make_icons.py, oz_mark). (cx, cy) is the centre of the ring.
    """
    w = r * 0.30
    band(layer, cx, cy, r, -12, 282, w, ring)
    layer.disc(cx, cy, r * 0.34, accent)
    band(layer, cx, cy, r * 1.62, 292, 338, w * 0.85, accent)


# ----------------------------------------------------------------------------- letters


def font_path() -> Path:
    for folder in FONT_DIRS:
        if (folder / FONT_FILE).is_file():
            return folder / FONT_FILE
    raise SystemExit("error: the font is missing: " + " or ".join(str(d / FONT_FILE) for d in FONT_DIRS))


def _font(size: float):
    font = ImageFont.truetype(str(font_path()), size)
    font.set_variation_by_axes([FONT_WEIGHT])
    return font


@lru_cache(maxsize=None)
def cap_ratio() -> float:
    """The height of a capital in ems."""
    return -_font(1000).getbbox("H", anchor="ls")[1] / 1000


@lru_cache(maxsize=None)
def font_at(cap: float):
    """The font at the size whose capitals stand `cap` card pixels high on the S canvas."""
    return _font(cap * S / cap_ratio())


def glyphs(text, cap, track=0.0, squeeze=1.0):
    """`text` as a mask of the S canvas, cut to its ink, and how far under its top the baseline is.

    `track` moves each next letter by that part of the capital height; `squeeze` narrows the lot.
    """
    font = font_at(cap)
    unit = int(cap * S)
    sheet = Image.new("L", (unit * (len(text) + 2), unit * 3), 0)
    d = ImageDraw.Draw(sheet)
    x, base = float(unit), unit * 2
    for ch in text:
        d.text((x, base), ch, font=font, fill=255, anchor="ls")
        x += font.getlength(ch) + track * unit
    box = sheet.getbbox()
    mask = sheet.crop(box)
    if squeeze != 1.0:
        mask = mask.resize((max(1, round(mask.width * squeeze)), mask.height), LANCZOS)
    return mask, base - box[1]


def _phi(z):
    return 0.5 * (1 + math.erf(z / math.sqrt(2)))


def contour(mask, inset, width, soft=2.5):
    """A line inside a glyph: `inset` card pixels in from its outline, `width` thick.

    The glyph blurred by `soft` card pixels rises from its outline inwards as the normal
    distribution does, so two levels of that blur are two distances from the outline; the
    line is what lies between them. A stroke too thin for the blur to reach the first level
    in its middle gets no line.
    """
    pad = int(soft * S * 4)
    wide = Image.new("L", (mask.width + 2 * pad, mask.height + 2 * pad), 0)
    wide.paste(mask, (pad, pad))
    blur = wide.filter(ImageFilter.GaussianBlur(soft * S)).crop((pad, pad, pad + mask.width, pad + mask.height))
    near, far = 255 * _phi(inset / soft), 255 * _phi((inset + width) / soft)
    return ImageChops.multiply(ramp(blur, round(near) - 3, round(near) + 3),
                               ImageChops.invert(ramp(blur, round(far) - 2, round(far) + 2)))


# ----------------------------------------------------------------------------- a face

IDX_X, IDX_TOP, IDX_CAP = 27.0, 12.0, 42.0       # the index: its axis, its top, its capital height
IDX_SUIT_Y, IDX_SUIT = 77.0, 30.0                # the suit under the rank: its centre, its height

PIP, PIP_MANY = 33.0, 31.0                       # a pip; a pip of the nine and the ten, where they crowd
COL_X = {"L": 70.0, "C": 100.0, "R": 130.0}
ROW_FIRST, ROW_LAST = 52.0, 228.0

PANEL = (52.0, 24.0, 148.0, 256.0)               # the frame of a court card
COURT_CAP, COURT_TOP = 60.0, 52.5                # its letter: capital height, top
TOP_Y = 41.0                                     # the middle of what sits over the letter


def stock(style, square=False):
    """The card itself on the S canvas, RGB: the stock inside the line round it.

    The edge colour fills the canvas, the corners included: the picture's alpha cuts them
    off later, and what is left under it is a colour that may be filtered in.
    """
    im = Image.new("RGB", (W * S, H * S), style.edge)
    d = ImageDraw.Draw(im)
    inside = rect(EDGE, EDGE, W - EDGE, H - EDGE)
    if square:
        d.rectangle(inside, fill=style.paper)
    else:
        d.rounded_rectangle(inside, radius=(CORNER - EDGE) * S, fill=style.paper)
    return im


@lru_cache(maxsize=None)
def card_alpha():
    m = Image.new("L", (W * S, H * S), 0)
    ImageDraw.Draw(m).rounded_rectangle(rect(0, 0, W, H), radius=CORNER * S, fill=255)
    return flat(m.resize((W, H), LANCZOS))


def index(style, rank, suit):
    """The index of the top left corner: the rank over a small suit."""
    lay = Layer()
    ink = style.ink(suit)
    if rank == "T":
        mask, base = glyphs("10", IDX_CAP, track=-0.03, squeeze=0.88)
    else:
        mask, base = glyphs(rank, IDX_CAP)
    lay.stamp(mask, IDX_X - mask.width / S / 2, IDX_TOP + IDX_CAP - base / S, ink)
    suit_at(lay, suit, IDX_X, IDX_SUIT_Y, IDX_SUIT, ink)
    return lay


def pip_spots(n):
    """Where the pips of a number card sit: the column, and how far down from the first row
    to the last. The arrangement is the customary one; a pip below the middle is turned."""
    def both(t):
        return [("L", t), ("R", t)]

    return {
        2: [("C", 0), ("C", 1)],
        3: [("C", 0), ("C", 1 / 2), ("C", 1)],
        4: both(0) + both(1),
        5: both(0) + both(1) + [("C", 1 / 2)],
        6: both(0) + both(1 / 2) + both(1),
        7: both(0) + both(1 / 2) + both(1) + [("C", 1 / 4)],
        8: both(0) + both(1 / 2) + both(1) + [("C", 1 / 4), ("C", 3 / 4)],
        9: both(0) + both(1 / 3) + both(2 / 3) + both(1) + [("C", 1 / 2)],
        10: both(0) + both(1 / 3) + both(2 / 3) + both(1) + [("C", 1 / 6), ("C", 5 / 6)],
    }[n]


def ace(ink, style, suit):
    colour = style.ink(suit)
    if suit != "s":
        suit_at(ink, suit, CX, CY, 96, colour)
        return
    # The ace of spades is the big one and, by custom, the maker's: a line inside the leaf
    # and the mark of the series in its middle.
    size, cy = 128.0, CY - 3
    suit_at(ink, "s", CX, cy, size, colour)
    leaf = [(CX + x * size * 0.80, cy + (y + 0.035) * size * 0.80) for x, y in SHAPES["s"][0]]
    ink.line(leaf, style.paper, 1.3, closed=True)
    r = 12.0
    oz_mark(ink, CX - 0.25 * r, cy + 0.07 * size + 0.25 * r, r, style.paper, style.paper)


# --- the court cards: a framed panel, the letter twice (the lower one turned), and for each
# rank its own ground, its own sign over the letter and its own piece in the middle.


def ground_lattice(lay, colour):           # king
    c, s = math.cos(math.radians(60)), math.sin(math.radians(60))
    for n in range(-26, 27):
        x = CX + n * 11.0
        for sign in (1, -1):
            lay.line([(x - sign * c * 220, CY - s * 220), (x + sign * c * 220, CY + s * 220)], colour + (44,), 0.7)


def ground_dots(lay, colour):              # queen
    for j in range(-22, 23):
        for i in range(-8, 9):
            lay.disc(CX + (i + (0.5 if j % 2 else 0.0)) * 8.0, CY + j * 6.0, 1.0, colour + (60,))


def ground_stripes(lay, colour):           # jack
    for n in range(-40, 41):
        x = CX + n * 9.0
        lay.line([(x - 220, CY + 220), (x + 220, CY - 220)], colour + (36,), 1.3)


def top_crown(lay, style, colour):         # king: a crown of five points
    y = TOP_Y + 2
    spikes = [(-19, 3), (-19, -4), (-13, 0.5), (-9.5, -6), (-5, 0), (0, -8), (5, 0), (9.5, -6),
              (13, 0.5), (19, -4), (19, 3)]
    body = [(CX + px, y + py) for px, py in spikes]
    lay.poly(body, style.gold)
    lay.line(body, colour, 1.1, closed=True)
    lay.rrect(CX - 20, y + 2.6, CX + 20, y + 8.4, 1.2, fill=colour)
    lay.rrect(CX - 18.9, y + 3.7, CX + 18.9, y + 7.3, 0.6, fill=style.gold)
    for px, py, r in ((-19, -4.6, 1.8), (-9.5, -6.8, 2.0), (0, -9.2, 2.4), (9.5, -6.8, 2.0), (19, -4.6, 1.8)):
        lay.disc(CX + px, y + py, r + 0.9, colour)
        lay.disc(CX + px, y + py, r - 0.1, style.gold)
    for px in (-12, -6, 0, 6, 12):
        lay.disc(CX + px, y + 5.5, 1.0, colour)


def top_tiara(lay, style, colour):         # queen: a bow set with pearls
    oy, r = TOP_Y + 23.0, 24.0             # the centre of the bow's circle lies under it
    band(lay, CX, oy, r + 1.0, 270 - 52, 270 + 52, 5.4, colour)
    band(lay, CX, oy, r, 270 - 52, 270 + 52, 3.4, style.gold)
    for a, pr in ((-42, 1.8), (-21, 2.3), (0, 3.0), (21, 2.3), (42, 1.8)):
        x, y = arc(CX, oy, r + pr + 1.2, 270 + a, 270 + a, 1)[0]
        lay.disc(x, y, pr + 0.9, colour)
        lay.disc(x, y, pr - 0.1, style.gold)


def top_chevrons(lay, style, colour):      # jack: two chevrons, a soldier's
    for y in (TOP_Y - 5.5, TOP_Y + 2.0):
        v = [(CX - 15, y + 3.5), (CX, y - 3.5), (CX + 15, y + 3.5),
             (CX + 15, y + 7.7), (CX, y + 0.7), (CX - 15, y + 7.7)]
        lay.poly(v, style.gold)
        lay.line(v, colour, 1.1, closed=True)


def middle_king(ink, style, suit, colour):         # a studded belt and a ring
    x0, x1 = PANEL[0] + 4.4, PANEL[2] - 4.4
    ink.poly([(x0, CY - 5.6), (x1, CY - 5.6), (x1, CY + 5.6), (x0, CY + 5.6)], colour)
    ink.poly([(x0, CY - 4.5), (x1, CY - 4.5), (x1, CY + 4.5), (x0, CY + 4.5)], style.gold)
    for i in range(-5, 6):
        if abs(i) >= 3:
            ink.disc(CX + i * 7.6, CY, 1.5, colour)
    ink.disc(CX, CY, 19.5, colour)
    ink.disc(CX, CY, 18.4, style.gold)
    ink.disc(CX, CY, 14.8, colour)
    ink.disc(CX, CY, 13.7, style.paper)
    suit_at(ink, suit, CX, CY + 0.5, 17.5, colour)


def _leaf(ink, x, y, angle, length, width, fill, outline):
    c, s = math.cos(math.radians(angle)), math.sin(math.radians(angle))

    def turn(px, py):
        return (x + px * c - py * s, y + px * s + py * c)

    pts = ([turn(*p) for p in quad((0, 0), (length * 0.45, -width), (length, 0), 8)]
           + [turn(*p) for p in quad((length, 0), (length * 0.45, width), (0, 0), 8)])
    ink.poly(pts, fill)
    ink.line(pts, outline, 0.8, closed=True)


def middle_queen(ink, style, suit, colour):        # two sprays and a ring of pearls
    for k in (1, -1):                              # the right spray, then the same turned
        stem = cubic((CX + k * 15, CY + k * 2), (CX + k * 26, CY + k * 11), (CX + k * 36, CY + k * 6),
                     (CX + k * 40.5, CY - k * 5), 24)
        ink.line(stem, colour, 1.3)
        for i, side in ((7, -1), (12, 1), (17, -1), (21, 1)):
            (ax, ay), (bx, by) = stem[i - 1], stem[i + 1]
            heading = math.degrees(math.atan2(by - ay, bx - ax))
            _leaf(ink, stem[i][0], stem[i][1], heading + side * 48, 8.5, 3.0, style.gold, colour)
        ink.disc(stem[-1][0], stem[-1][1], 2.7, colour)
        ink.disc(stem[-1][0], stem[-1][1], 1.7, style.gold)
    ink.disc(CX, CY, 14.4, colour)
    ink.disc(CX, CY, 13.3, style.paper)
    for x, y in ngon(CX, CY, 16.6, 14):
        ink.disc(x, y, 2.9, colour)
        ink.disc(x, y, 1.9, style.gold)
    suit_at(ink, suit, CX, CY + 0.5, 17.0, colour)


def middle_jack(ink, style, suit, colour):         # two crossed staves and a six-sided plate
    x0, x1 = PANEL[0] + 10.5, PANEL[2] - 10.5
    for k in (1, -1):
        a, b = (x0, CY + k * 19.5), (x1, CY - k * 19.5)
        ink.line([a, b], colour, 4.8)
        ink.line([a, b], style.gold, 2.6)
        for x, y in (a, b):
            ink.disc(x, y, 3.9, colour)
            ink.disc(x, y, 2.8, style.gold)
    ink.poly(ngon(CX, CY, 20.0, 6), colour)
    ink.poly(ngon(CX, CY, 18.7, 6), style.gold)
    ink.poly(ngon(CX, CY, 15.4, 6), colour)
    ink.poly(ngon(CX, CY, 14.2, 6), style.paper)
    suit_at(ink, suit, CX, CY + 0.5, 16.5, colour)


COURT = {
    "K": (ground_lattice, top_crown, middle_king),
    "Q": (ground_dots, top_tiara, middle_queen),
    "J": (ground_stripes, top_chevrons, middle_jack),
}


def court(ink, style, rank, suit):
    ground, top, middle = COURT[rank]
    colour = style.ink(suit)
    x0, y0, x1, y1 = PANEL
    ink.rrect(x0, y0, x1, y1, 5, fill=style.tints[rank])
    pattern = Layer()
    ground(pattern, colour)
    pattern.clip(window(x0 + 4.4, y0 + 4.4, x1 - 4.4, y1 - 4.4, 2.5))
    ink.add(pattern)
    ink.rrect(x0, y0, x1, y1, 5, outline=colour, width=2.0)
    ink.rrect(x0 + 3.8, y0 + 3.8, x1 - 3.8, y1 - 3.8, 2.8, outline=colour, width=0.8)

    half = Layer()                                 # the upper half; the lower is this one turned
    top(half, style, colour)
    for side in (-1, 1):                           # the suit again, small, either side of it
        suit_at(half, suit, CX + side * 33.5, TOP_Y + 3.0, 12.5, colour)
    mask, base = glyphs(rank, COURT_CAP)
    x, y = CX - mask.width / S / 2, COURT_TOP + COURT_CAP - base / S
    half.stamp(mask, x, y, colour)
    half.stamp(contour(mask, 2.6, 1.2), x, y, style.gold)
    ink.add(half)
    ink.add_turned(half)
    middle(ink, style, suit, colour)


# --- the picture cards of a deck that has them. A picture is read from art/cards/<style>/src:
# a photograph, which fills its frame, or a figure cut out of its ground, which stands on a
# panel tinted by rank. An ace carries one picture, between its two indices; any other card
# carries it twice, the lower one turned, as a court card carries its figure, and the name
# between the two. The names are the Ukrainian ones of the games' own texts.
#
# Who is on which card. The higher the card, the more dangerous: the stalkers stand over
# the mutants. To change a card, change its line; a picture is <name>.png in the src folder.

PICTURES = {
    "stalker": {
        "As": ("photo", "СТРІЛЕЦЬ", "strilets"),
        "Ah": ("photo", "ДЕГТЯРЬОВ", "dehtiarov"),
        "Ad": ("photo", "ШРАМ", "shram"),
        "Ac": ("photo", "СКІФ", "skif"),
        "Ks": ("photo", "ПРИВИД", "pryvyd"),
        "Kh": ("photo", "СИДОРОВИЧ", "sydorovych"),
        "Kd": ("photo", "ДОКТОР", "doktor"),
        "Kc": ("photo", "ВОВК", "vovk"),
        "Qs": ("figure", "ХИМЕРА", "khymera"),
        "Qh": ("figure", "ПОЛТЕРГЕЙСТ", "polterheist"),
        "Qd": ("figure", "КОНТРОЛЕР", "kontroler"),
        "Qc": ("figure", "ПСЕВДОГІГАНТ", "psevdohihant"),
        "Js": ("figure", "БЮРЕР", "biurer"),
        "Jh": ("figure", "СНОРК", "snork"),
        "Jd": ("figure", "КРОВОСОС", "krovosos"),
        "Jc": ("figure", "ПСЕВДОСОБАКА", "psevdosobaka"),
        "Ts": ("figure", "ТУШКАН", "tushkan"),
        "Th": ("figure", "СЛІПИЙ ПЕС", "slipyi_pes"),
        "Td": ("figure", "ПЛОТЬ", "plot"),
        "Tc": ("figure", "КАБАН", "kaban"),
    },
}

FRAME_TOP = (52.0, 13.0, 189.0, 103.0)           # the upper picture of a card that carries two
FRAME_LOW = (11.0, 177.0, 148.0, 267.0)          # the lower, turned: the upper one's half turn
FRAME_ACE = (31.0, 96.0, 169.0, 186.0)           # the one picture of an ace


@lru_cache(maxsize=None)
def source(style_name, slug):
    path = ART / style_name / "src" / f"{slug}.png"
    if not path.is_file():
        raise SystemExit(f"error: the picture is missing: {path}")
    with Image.open(path) as im:
        return im.convert("RGBA")


def as_printed(im):
    """A picture as a press would give it on this stock: a little less colour, a little warmer."""
    rgb = ImageEnhance.Color(im.convert("RGB")).enhance(0.82)
    rgb = ImageChops.multiply(rgb, Image.new("RGB", rgb.size, (255, 246, 228)))
    out = rgb.convert("RGBA")
    out.putalpha(im.getchannel("A"))
    return out


def filled(im, w, h):
    """The picture scaled and cut to fill w x h; what is cut goes more from below than above."""
    k = max(w / im.width, h / im.height)
    im = im.resize((max(1, round(im.width * k)), max(1, round(im.height * k))), LANCZOS)
    x, y = (im.width - w) // 2, (im.height - h) // 3
    return im.crop((x, y, x + w, y + h))


def stood(im, w, h):
    """The figure scaled to fit w x h and stood on the bottom of a clear sheet of that size."""
    k = min(w / im.width, h / im.height)
    im = im.resize((max(1, round(im.width * k)), max(1, round(im.height * k))), LANCZOS)
    sheet = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    sheet.alpha_composite(im, ((w - im.width) // 2, h - im.height))
    return sheet


def framed(ink, style, im, box, kind, colour, ground, turned=False):
    """A picture in a frame of card coordinates: the suit's colour outside, a gold line inside."""
    x0, y0, x1, y1 = box
    w, h = px(x1) - px(x0), px(y1) - px(y0)
    if kind == "figure":
        tile = Image.new("RGBA", (w, h), ground + (255,))
        fade = Image.linear_gradient("L").resize((w, h)).point(lambda v: v * 150 // 255)
        tile.paste(Image.new("RGBA", (w, h), style.shade + (255,)), (0, 0), fade)
        tile.alpha_composite(stood(as_printed(im), w - 6 * S, h - 4 * S), (3 * S, 4 * S))
    else:
        tile = filled(as_printed(im), w, h)
    if turned:
        tile = tile.transpose(Image.Transpose.ROTATE_180)
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, w - 1, h - 1), radius=3 * S, fill=255)
    tile.putalpha(ImageChops.multiply(tile.getchannel("A"), mask))
    ink.im.alpha_composite(tile, (px(x0), px(y0)))
    ink.rrect(x0, y0, x1, y1, 3, outline=colour, width=1.5)
    ink.rrect(x0 + 2.6, y0 + 2.6, x1 - 2.6, y1 - 2.6, 1.6, outline=style.gold, width=0.7)


def caption(ink, text, cx, cy, cap, colour, room):
    """A name in capitals `cap` high, centred on (cx, cy), narrowed if it is wider than `room`."""
    mask, base = glyphs(text, cap, track=0.06)
    if mask.width / S > room:
        mask = mask.resize((round(room * S), mask.height), LANCZOS)
    ink.stamp(mask, cx - mask.width / S / 2, cy + cap / 2 - base / S, colour)
    return mask.width / S


def picture_card(ink, style, rank, suit, spec):
    kind, name, slug = spec
    colour = style.ink(suit)
    ground = style.tints.get(rank, style.tints["Q"])
    im = source(style.name, slug)
    if rank == "A":
        suit_at(ink, suit, CX + 14, 42, 40, colour)
        framed(ink, style, im, FRAME_ACE, kind, colour, ground)
        mid = 88.0                                 # the name keeps clear of the turned index
        ink.line([(mid - 60, 198), (mid + 60, 198)], style.gold, 0.9)
        caption(ink, name, mid, 212.0, 13.0, colour, 118)
        return
    framed(ink, style, im, FRAME_TOP, kind, colour, ground)
    framed(ink, style, im, FRAME_LOW, kind, colour, ground, turned=True)
    width = caption(ink, name, CX, CY, 15.0, colour, 128)
    for side in (-1, 1):
        suit_at(ink, suit, CX + side * (width / 2 + 13), CY, 13.0, colour)
    for y in (CY - 17, CY + 17):
        ink.line([(26, y), (W - 26, y)], style.gold, 0.9)


def face_layers(style, rank, suit):
    """A face as the stock (RGB) and what is printed on it (a Layer), both on the S canvas."""
    ink = Layer()
    colour = style.ink(suit)
    spec = PICTURES.get(style.name, {}).get(rank + suit)
    if spec:
        picture_card(ink, style, rank, suit, spec)
    elif rank in COURT:
        court(ink, style, rank, suit)
    elif rank == "A":
        ace(ink, style, suit)
    else:
        count = 10 if rank == "T" else int(rank)
        size = PIP_MANY if count >= 9 else PIP
        for col, t in pip_spots(count):
            suit_at(ink, suit, COL_X[col], ROW_FIRST + (ROW_LAST - ROW_FIRST) * t, size, colour, flip=t > 0.5)
    corner = index(style, rank, suit)
    ink.add(corner)
    ink.add_turned(corner)
    return stock(style), ink


# ----------------------------------------------------------------------------- the backs


def back_classic(ink, style):
    """Deep burgundy under a white lattice, a border, and in the middle a medallion with the
    mark of the series (the owner's word, 2026-11-24: the logo of OpenZone on the classic back).
    The mark leans up and right, so the back no longer reads the same either way up."""
    ground, line = style.back, style.line
    ink.rrect(9, 9, W - 9, H - 9, 8, fill=ground)
    step, slope = 14.0, math.tan(math.radians(55))
    net = Layer()
    for n in range(-32, 33):
        x = CX + n * step
        for sign in (1, -1):
            net.line([(x - sign * 220, CY - 220 * slope), (x + sign * 220, CY + 220 * slope)], line + (236,), 1.0)
    for j in range(-16, 17):                       # a stud on every crossing
        for i in range(-16, 17):
            if (i + j) % 2 == 0:
                net.disc(CX + i * step / 2, CY + j * step / 2 * slope, 1.7, line + (255,))
    net.clip(window(16, 16, W - 16, H - 16, 4))
    ink.add(net)
    ink.rrect(12.5, 12.5, W - 12.5, H - 12.5, 5.5, outline=line, width=1.4)

    ink.disc(CX, CY, 40.0, line)
    ink.disc(CX, CY, 38.0, style.back_deep)
    ink.circle(CX, CY, 34.5, line, 0.8)
    r = 18.0                                       # the mark leans up and right; this centres it
    oz_mark(ink, CX - 0.25 * r, CY + 0.25 * r, r, style.mark, style.accent)


def back_zone(ink, style):
    """Olive under a khaki honeycomb, a border, and in the middle the mark of the series."""
    ground, line = style.back, style.line
    ink.rrect(8, 8, W - 8, H - 8, 8, fill=ground)
    a = 11.0                                       # a cell's corner radius, a corner up
    comb = Layer()
    cells = [(i, j) for j in range(-12, 13) for i in range(-7, 8)]

    def centre(i, j):
        return CX + (i + (0.5 if j % 2 else 0.0)) * math.sqrt(3) * a, CY + j * 1.5 * a

    for i, j in cells:                             # every third cell is filled: none touches another
        if (i - (j - j % 2) // 2 - j) % 3 == 0:
            comb.poly(ngon(*centre(i, j), a, 6), style.back_light + (255,))
    for i, j in cells:
        comb.line(ngon(*centre(i, j), a, 6), line + (150,), 0.9, closed=True)
    comb.clip(window(15, 15, W - 15, H - 15, 4))
    ink.add(comb)
    ink.rrect(11.5, 11.5, W - 11.5, H - 11.5, 5.5, outline=line, width=1.4)

    ink.disc(CX, CY, 38.0, line)
    ink.disc(CX, CY, 36.4, style.back_deep)
    ink.circle(CX, CY, 33.0, line, 0.7)
    r = 19.0                                       # the mark leans up and right; this centres it
    oz_mark(ink, CX - 0.25 * r, CY + 0.25 * r, r, style.mark, style.accent)


def back_stalker(ink, style):
    """Red under a pale diagonal lattice, a border, and the trefoil of the Zone in a disc.

    The pattern is the one of the playing cards GSC drew for S.T.A.L.K.E.R. 2; it is drawn
    here, not copied from them.
    """
    ground, line = style.back, style.line
    ink.rrect(8, 8, W - 8, H - 8, 8, fill=ground)
    net = Layer()
    step, slope = 9.0, math.tan(math.radians(52))
    for n in range(-50, 51):
        x = CX + n * step
        for sign in (1, -1):
            net.line([(x - sign * 220, CY - 220 * slope), (x + sign * 220, CY + 220 * slope)], line + (214,), 1.5)
    net.clip(window(14, 14, W - 14, H - 14, 4))
    ink.add(net)
    ink.rrect(11.5, 11.5, W - 11.5, H - 11.5, 5.5, outline=line, width=1.6)
    ink.disc(CX, CY, 44.0, ground)
    ink.disc(CX, CY, 41.0, line)
    ink.circle(CX, CY, 37.5, ground, 1.0)
    for a in (-90, 30, 150):                       # three blades, a sixth of the circle each
        ink.poly(arc(CX, CY, 32.0, a - 30, a + 30) + list(reversed(arc(CX, CY, 10.0, a - 30, a + 30))), ground)
    ink.disc(CX, CY, 6.0, ground)


BACKS = {"classic": back_classic, "zone": back_zone, "stalker": back_stalker}


def back_layers(style, square=False):
    ink = Layer()
    BACKS[style.name](ink, style)
    return stock(style, square), ink


# ----------------------------------------------------------------------------- wear
# All of it is Pillow: random bytes blown up into clouds, soft thresholds, tints through
# masks. Each effect has its own seeded stream, so one picture drawn at two sizes wears alike.


def rng_for(*key) -> random.Random:
    """The stream of chance of one effect on one picture: seeded by their names, nothing else."""
    return random.Random(zlib.crc32("/".join(key).encode("utf-8")))


def noise(rng, size, grain):
    """Smooth noise 0..255 with features about `grain` px across."""
    gw, gh = max(2, round(size[0] / grain) + 1), max(2, round(size[1] / grain) + 1)
    return Image.frombytes("L", (gw, gh), rng.randbytes(gw * gh)).resize(size, BICUBIC)


def cloud(rng, size, grains, weights):
    """Several noises of different grain laid over each other, each with its weight."""
    acc, total = None, 0.0
    for grain, weight in zip(grains, weights):
        layer = noise(rng, size, grain)
        total += weight
        acc = layer if acc is None else Image.blend(acc, layer, weight / total)
    return acc


def tint(rgb, colour, mask):
    """Pull `rgb` towards `colour` as far as `mask` says, in place."""
    rgb.paste(colour, (0, 0), mask)


def edge_mask(size, reach):
    """White along the four edges, gone to black about `reach` px in."""
    m = Image.new("L", size, 255)
    ImageDraw.Draw(m).rectangle((reach, reach, size[0] - 1 - reach, size[1] - 1 - reach), fill=0)
    return m.filter(ImageFilter.GaussianBlur(reach * 0.5))


def scratch_mask(rng, size, k, count, longest):
    """`count` hairlines, each a little bent, up to `longest` card px long; drawn larger and reduced."""
    q = 3
    m = Image.new("L", (size[0] * q, size[1] * q), 0)
    d = ImageDraw.Draw(m)
    for _ in range(count):
        x, y = rng.uniform(0, size[0]), rng.uniform(0, size[1])
        a, n, bend = rng.uniform(0, math.tau), rng.uniform(8, longest) * k, rng.uniform(-0.3, 0.3)
        value, width = rng.randint(110, 235), rng.uniform(0.3, 0.8) * k
        dx, dy = math.cos(a), math.sin(a)
        pts = []
        for i in range(7):
            t = i / 6
            off = bend * n * t * (1 - t)
            pts.append(((x + dx * n * t - dy * off) * q, (y + dy * n * t + dx * off) * q))
        d.line(pts, fill=value, width=max(1, round(width * q)), joint="curve")
    return m.resize(size, BOX)


def speck_mask(rng, size, k, count):
    """`count` specks of a third of a pixel to a pixel and a quarter across."""
    q = 3
    m = Image.new("L", (size[0] * q, size[1] * q), 0)
    d = ImageDraw.Draw(m)
    for _ in range(count):
        x, y = rng.uniform(0, size[0]), rng.uniform(0, size[1])
        r, value = rng.uniform(0.3, 1.25) * k, rng.randint(90, 220)
        d.ellipse(((x - r) * q, (y - r) * q, (x + r) * q, (y + r) * q), fill=value)
    return m.resize(size, BOX)


def spill(card, style, rng, k):
    """One stain: an uneven blot, a little darker along its rim."""
    w, h = card.size
    x, y = rng.uniform(0.05, 0.95) * w, rng.uniform(0.05, 0.95) * h
    r, squash, strength = rng.uniform(14, 40) * k, rng.uniform(0.6, 1.0), rng.randint(18, 36)
    spot = Image.new("L", card.size, 0)
    ImageDraw.Draw(spot).ellipse((x - r, y - r * squash, x + r, y + r * squash), fill=255)
    spot = spot.filter(ImageFilter.GaussianBlur(r * 0.3))
    rough = Image.blend(Image.new("L", card.size, 128), cloud(rng, card.size, (r * 0.9, r * 0.3), (3, 2)), 0.5)
    blot = ImageChops.add(spot, rough, 1.0, -128)
    body = ramp(blot, 112, 140, strength)
    rim = ImageChops.multiply(ramp(blot, 100, 116), ImageChops.invert(ramp(blot, 122, 140)))
    tint(card, style.stain, body)
    tint(card, style.stain, ramp(rim, 0, 255, strength + 26))


def age_stock(style, name, paper, k):
    """The stock: uneven in tone, with a little fibre in it."""
    size = paper.size
    rng = rng_for(style.name, name, "stock")
    tint(paper, style.shade, ramp(cloud(rng, size, (64 * k, 26 * k, 10 * k), (5, 3, 2)), 108, 190, 46))
    tint(paper, style.light, ramp(cloud(rng, size, (44 * k, 15 * k), (3, 2)), 128, 200, 38))
    rng = rng_for(style.name, name, "fibre")
    fibre = Image.frombytes("L", size, rng.randbytes(size[0] * size[1])).filter(ImageFilter.GaussianBlur(0.55 * k))
    tint(paper, style.shade, ramp(fibre, 128, 176, 22))


def age_print(style, name, ink, k, rubbed):
    """The print: thin in places, chipped in a few; a back is rubbed pale along its edges."""
    size = ink.size
    rng = rng_for(style.name, name, "print")
    alpha = ink.getchannel("A")
    thin = ramp(cloud(rng, size, (34 * k, 12 * k, 4 * k), (4, 3, 3)), 118, 190, 72 if rubbed else 40)
    alpha = ImageChops.multiply(alpha, ImageChops.invert(thin))
    chips = ramp(cloud(rng, size, (2.4 * k, 1.2 * k), (1, 1)), 214, 226, 150)
    alpha = ImageChops.multiply(alpha, ImageChops.invert(chips))
    if rubbed:
        rub = ImageChops.multiply(edge_mask(size, 12 * k), ramp(cloud(rng, size, (22 * k, 7 * k), (1, 1)), 90, 170))
        alpha = ImageChops.multiply(alpha, ImageChops.invert(ramp(rub, 40, 255, 110)))
    ink.putalpha(alpha)


def age_card(style, name, card, k, rubbed):
    """What handling leaves on stock and print alike: dirt at the edges, stains, specks, scratches."""
    size = card.size
    rng = rng_for(style.name, name, "grime")
    grime = ImageChops.multiply(edge_mask(size, 14 * k), ramp(cloud(rng, size, (30 * k, 11 * k), (3, 2)), 70, 190))
    tint(card, style.grime, ramp(grime, 0, 255, 84))
    rng = rng_for(style.name, name, "stains")
    for _ in range(rng.randint(0, 2)):
        spill(card, style, rng, k)
    rng = rng_for(style.name, name, "specks")
    tint(card, style.stain, ramp(speck_mask(rng, size, k, rng.randint(10, 26)), 0, 255, 150))
    rng = rng_for(style.name, name, "scratches")
    tint(card, style.light, ramp(scratch_mask(rng, size, k, 10 if rubbed else 5, 70), 0, 255, 120))
    tint(card, style.grime, ramp(scratch_mask(rng, size, k, 4, 46), 0, 255, 120))


# --- a lot of wear for every card. The recipe above ages every card of a deck alike: the
# stains, specks and scratches differ, the strength, the dark edges and the tone of the stock
# do not, and a deck laid out looks printed worn. An uneven deck has every card draw a lot
# first, seeded by its name: how hard it was used, which way its stock has turned, which
# side took the dirt, whether it was folded, dog-eared, rubbed, or left in the sun.


def toward(a, b, t):
    """From colour a towards b by t; past 1 it goes further than b."""
    return tuple(max(0, min(255, round(x + (y - x) * t))) for x, y in zip(a, b))


def side_mask(size, angle):
    """0..255 rising across the card in the direction `angle`, degrees."""
    g = Image.linear_gradient("L").rotate(angle, resample=BICUBIC)
    return g.crop((38, 38, 218, 218)).resize(size, BICUBIC)


def blot_at(rng, size, r):
    """A soft uneven patch about r px across somewhere on the card, 0..255."""
    x, y = rng.uniform(0.1, 0.9) * size[0], rng.uniform(0.1, 0.9) * size[1]
    spot = Image.new("L", size, 0)
    ImageDraw.Draw(spot).ellipse((x - r, y - r * rng.uniform(0.5, 1.0), x + r, y + r * rng.uniform(0.5, 1.0)), fill=255)
    spot = spot.filter(ImageFilter.GaussianBlur(r * 0.35))
    return ImageChops.multiply(spot, ramp(cloud(rng, size, (r * 0.7, r * 0.25), (3, 2)), 70, 190))


def fold_masks(rng, size, k, corner):
    """A fold as two hairlines side by side: its shadow and its lit edge.

    With `corner` the fold cuts a corner off, and the triangle it bent comes as a third mask.
    """
    q = 3
    w, h = size
    dark = Image.new("L", (w * q, h * q), 0)
    light = Image.new("L", (w * q, h * q), 0)
    bent = None
    if corner:
        cx, cy = rng.choice([(0, 0), (w, 0), (0, h), (w, h)])
        a, b = rng.uniform(13, 34) * k, rng.uniform(13, 34) * k
        p0 = (cx + (a if cx == 0 else -a), cy)
        p1 = (cx, cy + (b if cy == 0 else -b))
        bent = Image.new("L", (w * q, h * q), 0)
        ImageDraw.Draw(bent).polygon([(cx * q, cy * q), (p0[0] * q, p0[1] * q), (p1[0] * q, p1[1] * q)], fill=255)
        bent = bent.resize(size, BOX)
    else:
        turn = math.radians(rng.choice([0, 0, 90, 90, 45, -45, rng.uniform(0, 180)]) + rng.uniform(-9, 9))
        x, y = rng.uniform(0.2, 0.8) * w, rng.uniform(0.2, 0.8) * h
        dx, dy, far = math.cos(turn), math.sin(turn), w + h
        p0, p1 = (x - dx * far, y - dy * far), (x + dx * far, y + dy * far)
    nx, ny = p1[1] - p0[1], p0[0] - p1[0]
    n = math.hypot(nx, ny) or 1.0
    nx, ny, off, width = nx / n, ny / n, 0.85 * k, max(1, round(0.67 * k * q))
    ImageDraw.Draw(dark).line([(p0[0] * q, p0[1] * q), (p1[0] * q, p1[1] * q)], fill=rng.randint(150, 235), width=width)
    ImageDraw.Draw(light).line([((p0[0] + nx * off) * q, (p0[1] + ny * off) * q),
                                ((p1[0] + nx * off) * q, (p1[1] + ny * off) * q)], fill=rng.randint(140, 225), width=width)
    return dark.resize(size, BOX), light.resize(size, BOX), bent


def finish_uneven(style, name, paper, ink, k, rubbed):
    """finish() for a deck whose every card is worn its own way."""
    size = (W * k, H * k)
    lot = rng_for(style.name, name, "lot")
    wear = lot.triangular(0.3, 1.95, 0.8)              # how hard the card was used
    tone = lot.uniform(-1.0, 1.0)                      # its stock: darker and browner, or paler
    side = lot.uniform(0.0, 360.0)                     # where the dirt sat
    stains = min(4, int(lot.uniform(0.0, 1.6) * wear + lot.random() * 0.9))
    folds = (1 if lot.random() < 0.16 + 0.14 * wear else 0) + (1 if lot.random() < 0.05 * wear else 0)
    ears = 1 if lot.random() < 0.10 + 0.12 * wear else 0
    faded = 1.0 - lot.uniform(0.0, 0.2) * min(1.0, wear) if lot.random() < 0.45 else 1.0
    scuffs = int(lot.random() < 0.18 + 0.2 * wear) + int(lot.random() < 0.12 * wear)

    st = replace(style,
                 shade=toward(style.paper, style.shade, 0.55 + 0.55 * wear),
                 light=toward(style.paper, style.light, 0.6 + 0.4 * wear),
                 grime=toward(style.paper, style.grime, 0.45 + 0.6 * wear),
                 stain=toward(style.paper, style.stain, 0.55 + 0.5 * wear))

    card = paper.resize(size, LANCZOS)
    printed = ink.im.resize(size, LANCZOS)
    printed.putalpha(flat(printed.getchannel("A")))

    # the stock: its own tone, then the uneven ageing
    card.paste(st.shade if tone > 0 else st.light, (0, 0),
               Image.new("L", size, round(abs(tone) * (46 if tone > 0 else 60))))
    age_stock(st, name, card, k)

    # the print: thin and chipped; for some cards paler all over, or rubbed off in a patch
    age_print(st, name, printed, k, rubbed or wear > 1.45)
    alpha = printed.getchannel("A")
    if faded < 1.0:
        alpha = alpha.point([round(v * faded) for v in range(256)])
    rng = rng_for(style.name, name, "scuffs")
    for _ in range(scuffs):
        patch = ramp(blot_at(rng, size, rng.uniform(16, 44) * k), 60, 200, rng.randint(70, 150))
        alpha = ImageChops.multiply(alpha, ImageChops.invert(patch))
    printed.putalpha(alpha)
    card.paste(printed, (0, 0), printed)

    # handling: dirt along the edges, heavier on one side; stains; specks; scratches
    rng = rng_for(style.name, name, "grime")
    lean = ramp(side_mask(size, side), 96, 255)
    edge = ImageChops.multiply(edge_mask(size, round((10 + 7 * wear) * k)),
                               ramp(cloud(rng, size, (30 * k, 11 * k), (3, 2)), 70, 190))
    edge = ImageChops.multiply(edge, lean.point([70 + v * 185 // 255 for v in range(256)]))
    tint(card, st.grime, ramp(edge, 0, 255, round(38 + 50 * wear)))
    smear = ImageChops.multiply(lean, ramp(cloud(rng, size, (46 * k, 17 * k, 6 * k), (4, 3, 2)), 96, 200))
    tint(card, st.grime, ramp(smear, 40, 255, round(24 * wear)))
    rng = rng_for(style.name, name, "stains")
    for _ in range(stains):
        spill(card, st, rng, k)
    rng = rng_for(style.name, name, "specks")
    tint(card, st.stain, ramp(speck_mask(rng, size, k, round(4 + 22 * wear * rng.uniform(0.5, 1.3))), 0, 255, 150))
    rng = rng_for(style.name, name, "scratches")
    tint(card, st.light, ramp(scratch_mask(rng, size, k, round(1 + 7 * wear * rng.uniform(0.4, 1.4)), 70), 0, 255, 120))
    tint(card, st.grime, ramp(scratch_mask(rng, size, k, round(5 * wear * rng.uniform(0.2, 1.2)), 46), 0, 255, 120))

    # folds and bent corners
    rng = rng_for(style.name, name, "folds")
    for corner in [False] * folds + [True] * ears:
        dark, light, bent = fold_masks(rng, size, k, corner)
        if bent is not None:
            tint(card, st.shade, bent.point([v * 70 // 255 for v in range(256)]))
        tint(card, st.grime, ramp(dark, 0, 255, 150))
        tint(card, st.light, ramp(light, 0, 255, 170))
    return card


def finish(style, name, paper, ink, k=1, rubbed=False):
    """Stock and print brought down from the S canvas to k times a card and put together.

    A worn style ages them on the way. `name` seeds the wear.
    """
    if style.worn and style.uneven:
        return finish_uneven(style, name, paper, ink, k, rubbed)
    size = (W * k, H * k)
    card = paper.resize(size, LANCZOS)
    printed = ink.im.resize(size, LANCZOS)
    printed.putalpha(flat(printed.getchannel("A")))
    if style.worn:
        age_stock(style, name, card, k)
        age_print(style, name, printed, k, rubbed)
    card.paste(printed, (0, 0), printed)
    if style.worn:
        age_card(style, name, card, k, rubbed)
    return card


# ----------------------------------------------------------------------------- the pictures


def card_picture(style, name):
    """One of the 53 cards (52 faces and the back): 200 x 280, RGBA, the corners cut round."""
    if name == "back":
        paper, ink = back_layers(style)
    else:
        paper, ink = face_layers(style, name[2], name[3])
    card = finish(style, name, paper, ink, rubbed=name == "back").convert("RGBA")
    card.putalpha(card_alpha())
    return card


def suit_picture(style, suit):
    """A suit alone, large, in the middle of nothing: its colour everywhere, its shape in the alpha."""
    lay = Layer()
    suit_at(lay, suit, CX, CY, 156, (255, 255, 255, 255))
    out = Image.new("RGBA", (W, H), style.ink(suit))
    out.putalpha(flat(lay.im.getchannel("A").resize((W, H), LANCZOS)))
    return out


def picture(style, name):
    return suit_picture(style, name[-1]) if name.startswith("suit_") else card_picture(style, name)


# The 3D card (models/src/card.py) wears one texture on its face and one on its back, each
# the whole card stretched over it. The model rounds the corners itself, so the texture
# keeps the square card the stock is drawn on: the edge colour where the corners are cut.
CARD_TEXTURE = (256, 512)


def card_file(style, name):
    """The .paa a card's face or the back is, in its style's folder: ozg_<style>_<face>_co.paa,
    ozg_<style>_back_co.paa. The deck's ozgCards in config.cpp is the path up to the face.

    Measured 2026-11-24 (diag 1.30): with the two decks' textures named alike in two folders
    (classic/c_2s_co.paa, stalker/c_2s_co.paa), both decks in one inventory drew the same
    picture for each card, the classic one for some cards and the stalkers' for others. The
    file name is what the game told them apart by, so it has to be a name no other deck has."""
    face = name[2:] if name.startswith("c_") else name
    return DECKS / style.name / f"ozg_{style.name}_{face}_co.paa"


def card_texture(style, name):
    """A card or the back for the 3D card: drawn at twice a card's size, worn as the card is
    in the window (the same lot, the same marks), opaque, then stretched to CARD_TEXTURE."""
    if name == "back":
        paper, ink = back_layers(style)
    else:
        paper, ink = face_layers(style, name[2], name[3])
    return finish(style, name, paper, ink, k=2, rubbed=name == "back").resize(CARD_TEXTURE, LANCZOS)


def edge_band(style, size=(256, 128)):
    """The side of a stack: a card every two rows, its edge and the dark between it and the next.

    The cards lie in little packs, a few lighter, a few darker, so the band still shows
    something when it is drawn too small for single lines. Two rows a card keeps every
    line whole in a DXT block.
    """
    w, h = size
    out = Image.new("RGB", size, style.paper)
    d = ImageDraw.Draw(out)
    rng = rng_for(style.name, "deck", "edge")
    drift = 0.0
    for y in range(0, h, 2):
        drift = 0.75 * drift + rng.uniform(-1, 1) * 0.25
        tone = (0.955 + 0.045 * drift + rng.uniform(-0.015, 0.015)) * (0.9 if style.worn else 1.0)
        edge = tuple(max(0, min(255, round(c * tone))) for c in style.paper)
        dark = tuple(round(a + (b - a) * 0.55) for a, b in zip(edge, style.gap))
        d.line((0, y, w - 1, y), fill=edge)
        d.line((0, y + 1, w - 1, y + 1), fill=dark)
    streaks = Image.frombytes("L", (w // 5 + 2, 1), rng.randbytes(w // 5 + 2)).resize(size, BICUBIC)
    tint(out, style.gap, ramp(streaks, 120, 230, 22))
    if style.worn:
        tint(out, style.grime, ramp(cloud(rng, size, (40, 12), (3, 2)), 110, 200, 60))
        tint(out, style.stain, ramp(speck_mask(rng, size, 1, 40), 0, 255, 150))
    return out


def deck_sheet(style):
    """512 x 512 for the 3D deck: back | blank face over the edge band. Opaque, square corners."""
    sheet = Image.new("RGB", (512, 512), style.paper)
    paper, ink = back_layers(style, square=True)
    sheet.paste(finish(style, "back", paper, ink, k=2, rubbed=True).resize((256, 512), LANCZOS), (0, 0))
    blank = finish(style, "deck_face", stock(style, square=True), Layer(), k=2)
    sheet.paste(blank.resize((256, 384), LANCZOS), (256, 0))
    sheet.paste(edge_band(style), (256, 384))
    return sheet


def review_sheet(pictures):
    """The 57 on mid grey, a suit a row and the back and the suits under them, about 1600 px wide."""
    gap, cols = 14, len(RANKS)
    rows = [NAMES[i * cols:(i + 1) * cols] for i in range(4)] + [NAMES[52:]]
    sheet = Image.new("RGB", (cols * (W + gap) + gap, len(rows) * (H + gap) + gap), (128, 128, 128))
    for j, row in enumerate(rows):
        for i, name in enumerate(row):
            sheet.paste(pictures[name], (gap + i * (W + gap), gap + j * (H + gap)), pictures[name])
    return sheet.resize((1600, round(sheet.height * 1600 / sheet.width)), LANCZOS)


# ----------------------------------------------------------------------------- the atlas


def cell(i):
    """Where the i-th picture of NAMES lies in the atlas."""
    return CELL_X0 + (i % COLS) * PITCH_X, CELL_Y0 + (i // COLS) * PITCH_Y


def spread(img, by):
    """The picture's colours with its edge pixels repeated `by` px outwards, RGB."""
    w, h = img.size
    rgb = img.convert("RGB")
    out = Image.new("RGB", (w + 2 * by, h + 2 * by))
    out.paste(rgb, (by, by))
    out.paste(rgb.crop((0, 0, w, 1)).resize((w, by), NEAREST), (by, 0))
    out.paste(rgb.crop((0, h - 1, w, h)).resize((w, by), NEAREST), (by, by + h))
    out.paste(rgb.crop((0, 0, 1, h)).resize((by, h), NEAREST), (0, by))
    out.paste(rgb.crop((w - 1, 0, w, h)).resize((by, h), NEAREST), (by + w, by))
    for (sx, sy), (dx, dy) in (((0, 0), (0, 0)), ((w - 1, 0), (by + w, 0)),
                               ((0, h - 1), (0, by + h)), ((w - 1, h - 1), (by + w, by + h))):
        out.paste(rgb.getpixel((sx, sy)), (dx, dy, dx + by, dy + by))
    return out


def atlas(style, pictures):
    """The pictures in their cells, in the order of NAMES, on a clear sheet.

    Clear means an alpha of zero, not black: the sheet is the colour of a card's edge
    under it, and round each cell lie BLEED pixels of that cell's own edge colours.
    """
    assert len(NAMES) <= COLS * ROWS and cell(COLS * ROWS - 1)[1] + H + BLEED <= ATLAS
    sheet = Image.new("RGBA", (ATLAS, ATLAS), style.edge + (0,))
    for i, name in enumerate(NAMES):
        x, y = cell(i)
        halo = spread(pictures[name], BLEED).convert("RGBA")
        halo.putalpha(0)
        sheet.paste(halo, (x - BLEED, y - BLEED))
        sheet.paste(pictures[name], (x, y))        # no mask: the alpha is copied as it is
    return sheet


def imageset_text(set_name, texture):
    """The .imageset as the game's own are written: one space a level, LF, Flags 0, an empty Groups.

    `mpix 0` as in the other imagesets of the series: one texture, as large as RefSize.
    (The game's own sets with a single texture say 0, 1 or 3; the number only orders the
    textures of a set that has several sizes.)
    """
    lines = [
        "// GENERATED by tools/make_cards.py -- edit the script, not this file.",
        "ImageSetClass {",
        f' Name "{set_name}"',
        f" RefSize {ATLAS} {ATLAS}",
        " Textures {",
        "  ImageSetTextureClass {",
        "   mpix 0",
        f'   path "{texture}"',
        "  }",
        " }",
        " Images {",
    ]
    for i, name in enumerate(NAMES):
        x, y = cell(i)
        lines += [f"  ImageSetDefClass {name} {{", f'   Name "{name}"', f"   Pos {x} {y}",
                  f"   Size {W} {H}", "   Flags 0", "  }"]
    lines += [" }", " Groups {", " }", "}"]
    return "\n".join(lines) + "\n"


# ----------------------------------------------------------------------------- ImageToPAA


def find_image_to_paa(tools_arg):
    """ImageToPAA.exe from --tools, else $DAYZ_TOOLS, else the folder DayZ Tools left in the registry."""
    tail = Path("Bin") / "ImageToPAA" / "ImageToPAA.exe"
    roots = []
    if tools_arg:
        roots.append(Path(tools_arg))
    else:
        if os.environ.get("DAYZ_TOOLS"):
            roots.append(Path(os.environ["DAYZ_TOOLS"]))
        if sys.platform == "win32":
            import winreg
            for hive in (winreg.HKEY_CURRENT_USER, winreg.HKEY_LOCAL_MACHINE):
                try:
                    with winreg.OpenKey(hive, r"Software\Bohemia Interactive\Dayz Tools") as key:
                        roots.append(Path(winreg.QueryValueEx(key, "path")[0]))
                except OSError:
                    pass
    for root in roots:
        if (root / tail).is_file():
            return root / tail
    raise SystemExit("error: ImageToPAA.exe not found; pass --tools <DayZ Tools folder>, set DAYZ_TOOLS, "
                     "or pass --no-paa to stop at the PNG files")


PAA_KIND = {"_ca": (b"\x05\xff", "DXT5"), "_co": (b"\x01\xff", "DXT1")}


def paa_size(data):
    """The size of a .paa's largest mip, read from its bytes.

    After the two bytes of the type come the tags (`GGAT`, four letters, a length, the
    data), then a palette, empty for DXT, then the mips, largest first: width, height and
    three bytes of length. The top bit of the width only says that the mip is packed.
    """
    at = 2
    while data[at:at + 4] == b"GGAT":
        at += 12 + int.from_bytes(data[at + 8:at + 12], "little")
    at += 2 + 3 * int.from_bytes(data[at:at + 2], "little")
    return int.from_bytes(data[at:at + 2], "little") & 0x7FFF, int.from_bytes(data[at + 2:at + 4], "little")


def to_paa(exe, png, paa, size):
    """PNG -> .paa. The suffix of the PNG's name picks the format: _ca keeps the alpha, _co has none.

    What was written is read back: it must be of that format and its largest mip `size` px.
    """
    tag, kind = PAA_KIND[png.stem[-3:]]
    paa.parent.mkdir(parents=True, exist_ok=True)
    paa.unlink(missing_ok=True)                    # a stale file must not pass for a new one
    run = subprocess.run([str(exe), str(png), str(paa)], capture_output=True, text=True, errors="replace",
                         stdin=subprocess.DEVNULL, timeout=600, check=False)
    note = (run.stdout + run.stderr).strip()[-300:]
    if run.returncode != 0 or not paa.is_file() or paa.stat().st_size == 0:
        raise SystemExit(f"error: ImageToPAA (exit {run.returncode}) did not make {paa.name}: {note}")
    data = paa.read_bytes()
    if data[:2] != tag:
        raise SystemExit(f"error: {paa.name} is not {kind}: it starts {data[:2].hex()}")
    if paa_size(data) != size:
        raise SystemExit(f"error: {paa.name} is {paa_size(data)} px, wanted {size}")
    return f"{kind}, {size[0]} x {size[1]}"


# ----------------------------------------------------------------------------- checks


def check_imageset(path, sheet, pictures):
    """Read the written .imageset back and hold it against the atlas and the pictures.

    Returns the path of the texture it names.
    """
    text = path.read_text(encoding="utf-8")
    head = re.search(r'ImageSetClass \{\s*Name "(\w+)"\s*RefSize (\d+) (\d+)', text)
    if not head or head[1] != path.stem or (int(head[2]), int(head[3])) != sheet.size:
        raise SystemExit(f"error: {path.name}: its Name is not {path.stem}, or its RefSize not the atlas's {sheet.size}")
    texture = re.search(r'ImageSetTextureClass \{\s*mpix \d+\s*path "([^"]+)"', text)
    if not texture:
        raise SystemExit(f"error: {path.name}: names no texture")
    found = re.findall(r'ImageSetDefClass (\w+) \{\s*Name "(\w+)"\s*Pos (\d+) (\d+)\s*Size (\d+) (\d+)\s*Flags 0\s*\}',
                       text)
    names = [f[1] for f in found]
    if names != NAMES or any(f[0] != f[1] for f in found):
        lacks, extra = sorted(set(NAMES) - set(names)), sorted(set(names) - set(NAMES))
        raise SystemExit(f"error: {path.name}: {len(names)} images, not the {len(NAMES)} of NAMES in their order"
                         f" (missing: {' '.join(lacks) or 'none'}; not wanted: {' '.join(extra) or 'none'})")
    boxes = []
    for _, name, x, y, w, h in found:
        x, y, w, h = int(x), int(y), int(w), int(h)
        if (w, h) != (W, H) or x < 2 or y < 2 or x + w > ATLAS - 2 or y + h > ATLAS - 2:
            raise SystemExit(f"error: {path.name}: {name} has a wrong box {x} {y} {w} {h}")
        if sheet.crop((x, y, x + w, y + h)).tobytes() != pictures[name].tobytes():
            raise SystemExit(f"error: {path.name}: the atlas under {name} is not that picture")
        boxes.append((x, y, x + w, y + h))
    for i, a in enumerate(boxes):                  # two transparent pixels at least between any two
        for b in boxes[i + 1:]:
            if a[0] < b[2] + 2 and b[0] < a[2] + 2 and a[1] < b[3] + 2 and b[1] < a[3] + 2:
                raise SystemExit(f"error: {path.name}: two cells closer than 2 px: {a} {b}")
    outside = sheet.getchannel("A").copy()
    for box in boxes:
        outside.paste(0, box)
    if outside.getextrema() != (0, 0):
        raise SystemExit(f"error: {path.name}: the atlas is not transparent between the cells")
    return texture[1]


# ----------------------------------------------------------------------------- the run


def save_png(img, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path, format="PNG", optimize=True)


def build(style, exe):
    print(f"[{style.name}]")
    folder = ART / style.name
    pictures = {name: picture(style, name) for name in NAMES}
    for name, img in pictures.items():
        save_png(img, folder / f"{name}.png")
    save_png(review_sheet(pictures), folder / "_sheet.png")
    deck = deck_sheet(style)
    save_png(deck, folder / "deck_sheet.png")
    print(f"  {len(pictures)} pictures, deck_sheet.png and _sheet.png in {folder}")

    set_name = f"ozg_cards_{style.name}"
    sheet = atlas(style, pictures)
    atlas_png = BUILD / f"{set_name}_ca.png"
    deck_png = BUILD / f"ozg_deck_{style.name}_co.png"
    save_png(sheet, atlas_png)
    save_png(deck, deck_png)
    imageset = SETS / f"{set_name}.imageset"
    imageset.parent.mkdir(parents=True, exist_ok=True)
    imageset.write_text(imageset_text(set_name, f"{MOD}/gui/imagesets/{set_name}_ca.paa"),
                        encoding="utf-8", newline="\n")
    texture = ROOT / check_imageset(imageset, sheet, pictures)     # the pbo prefix is the mod's folder
    print(f"  {imageset}: {len(NAMES)} images, read back and held against the atlas")

    textures = []                                  # the 3D card's: 52 faces and the back
    for name in NAMES[:53]:
        png = BUILD / "3d" / style.name / f"{name}_co.png"
        save_png(card_texture(style, name), png)
        textures.append((png, card_file(style, name), CARD_TEXTURE))
    print(f"  {len(textures)} textures of the 3D card, {CARD_TEXTURE[0]} x {CARD_TEXTURE[1]}, in {png.parent}")

    if exe is None:
        print("  no .paa made (--no-paa)")
        return
    for png, paa, size in ((atlas_png, texture, sheet.size),
                           (deck_png, DECKS / f"ozg_deck_{style.name}_co.paa", deck.size)):
        kind = to_paa(exe, png, paa, size)
        print(f"  {paa}: {paa.stat().st_size:,} bytes, {kind}")
    total = 0
    for png, paa, size in textures:
        to_paa(exe, png, paa, size)
        total += paa.stat().st_size
    print(f"  {len(textures)} textures of the 3D card in {DECKS / style.name}: {total:,} bytes, DXT1")


def main(argv=None):
    parser = argparse.ArgumentParser(description="Draw the playing cards of OpenZone Games.")
    parser.add_argument("--style", choices=sorted(STYLES), action="append", help="only this style (may repeat)")
    parser.add_argument("--no-paa", action="store_true", help="stop at the PNG files and the .imageset")
    parser.add_argument("--tools", metavar="DIR", help="the DayZ Tools folder (default: $DAYZ_TOOLS, the registry)")
    args = parser.parse_args(argv)
    exe = None if args.no_paa else find_image_to_paa(args.tools)
    for name in args.style or list(STYLES):
        build(STYLES[name], exe)
    return 0


if __name__ == "__main__":
    sys.exit(main())
