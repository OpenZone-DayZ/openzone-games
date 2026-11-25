# Draws the marks of the game window (card sharp's design §4.4, §4.5). A sign
# is a fault drawn INTO a back's own pattern, to show to whoever looks closely
# and not at a glance; the face pencilled small and faint into the top corner
# of the back is what its reader sees beside it. The kinds, their sizes and
# inks and the pencil are those of the table mock-up the owner kept on
# 2026-11-24 (its make_marks.py, outside the repository; the shapes here
# GROW larger all round, so that they cover what the mock-up's covered); the
# shade and the nail nick are the design's two more kinds.
#
#   dot    a speck of dark ink inside one cell of the lattice
#   block  block-out: a crossing of the lattice, the next one down its line
#          and the line between them painted over in the ground's colour
#   cut    cut-out: a pale fleck scraped out of the ground, as with a razor
#   shade  a cell, or two cells along a line of the lattice, a little darker
#   nick   a nail nick: a small dent in the card's left or right edge
#
# A sign is a number 1..99: (sign - 1) % 5 picks the kind and (sign - 1) // 5
# its place among twenty of that kind, so ninety-nine signs are ninety-nine
# faults. The places are drawn by lot from the back's lattice, away from its
# emblem and from the pencil's corner, with fixed seeds: the same every run.
#
# Every fault and every pencilled face is a picture of one ink on a clear
# ground, laid over the back where it was drawn, as the mock-up laid it.
#
# On a FACE a sign is a small dark speck in the margin (design §4.5): one
# picture for every sign, put where the sign says on a line MARGIN in from the
# card's edge. The ninety-nine places are spread evenly over the stretches of
# that line where no face of the style carries ink (every face read against
# the paper round it) and dealt to the signs by lot, with a fixed seed: two
# signs a player notices sit apart on the faces as on the backs.
#
# Usage: python -I tools/make_marks.py [--paa <ImageToPAA.exe>] [--no-paa]
#
# Reads art/cards/<style>/back.png and the faces art/cards/<style>/c_*.png
# (tools/make_cards.py). Writes art/marks/<style>_signs.png (the ninety-nine
# on that back, to look at), art/marks/<style>_specks.png (the ninety-nine
# specks on one face, and where the faces of the style carry ink),
# art/marks/_view.png (each kind, a read back and a noticed face, as the
# mock-up's sheet), build/marks/ozg_marks_ca.png,
# OpenZone_Games/gui/imagesets/ozg_marks.imageset and, given ImageToPAA,
# ozg_marks_ca.paa beside it, and
# OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c.
import io
import math
import os
import random
import subprocess
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
CARDS = os.path.join(REPO, "art", "cards")
ART = os.path.join(REPO, "art", "marks")
BUILD = os.path.join(REPO, "build", "marks")
SETS = os.path.join(REPO, "OpenZone_Games", "gui", "imagesets")
CODE = os.path.join(REPO, "OpenZone_Games", "scripts", "5_Mission", "OpenZone_Games", "OZG_MarkPlaces.c")
IMAGE_TO_PAA = r"E:\Programs\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"
# The mock-up's pencil: a hand and the symbols of Windows (art/marks/SOURCES.md).
HAND = r"C:\Windows\Fonts\segoepr.ttf"
SYMBOL = r"C:\Windows\Fonts\seguisym.ttf"

SET = "ozg_marks"
TEXTURE = "OpenZone_Games/gui/imagesets/ozg_marks_ca.paa"
LANCZOS = Image.Resampling.LANCZOS

W, H = 200, 280                     # a card, px (tools/make_cards.py)
CX, CY = W / 2, H / 2
S = 4                               # drawn S times larger, then reduced
STYLES = ("classic", "stalker")
KINDS = ("dot", "block", "cut", "shade", "nick")
PER_KIND = 20
SIGNS = 99
RANKS = "23456789TJQKA"
SUITS = "shdc"
FACES = [r + u for u in SUITS for r in RANKS]
SUIT_SIGN = {"s": "\u2660", "h": "\u2665", "d": "\u2666", "c": "\u2663"}

# The lattices of the backs (tools/make_cards.py, back_classic and
# back_stalker): the step between parallel lines and their slope. A crossing
# lies at (CX + i * step / 2, CY + j * step / 2 * slope) with i + j even, the
# middle of a cell where i + j is odd.
LATTICE = {"classic": (14.0, math.tan(math.radians(55))), "stalker": (9.0, math.tan(math.radians(52)))}
# The box the lattice is clipped to, and the emblem in its middle that no
# fault touches: its middle and a radius with room round it.
FIELD = {"classic": (16, 16, 184, 264), "stalker": (14, 14, 186, 266)}
EMBLEM = {"classic": (94.5, 145.5, 50.0), "stalker": (100.0, 140.0, 50.0)}
# The inks: the mock-up's dot, scrape and pencil; a shade a little darker
# than the ground; a nick in a darker edge colour.
DOT = (18, 8, 8, 235)
PALE = {"classic": (236, 226, 204, 230), "stalker": (224, 208, 172, 220)}
SHADE = (0, 0, 0, 64)
NICK = {"classic": (70, 62, 50, 235), "stalker": (54, 44, 28, 235)}
PENCIL = (22, 12, 12, 125)
ROOM = 6                            # kept clear round the emblem and the pencil
GAP = {"dot": 22, "block": 22, "cut": 22, "shade": 22, "nick": 12}
# The mock-up drew at a card's size, pixel by pixel, and Pillow counts a shape's last
# pixels in whole; drawn larger and reduced, a shape covers as much when it is this
# much larger all round, in card pixels (held against the mock-up's sheet, 2026-11-25).
GROW = 0.5
# The speck on a face (design §4.5): a dark ink, as the back's dot; its radius
# (GROW comes on top, as on the backs); how far in from a card's edge its
# middle lies, on a line that runs round the corners; the least distance
# between two specks' middles. CORNER is a card's corner radius, as
# tools/make_cards.py draws it.
SPECK = (20, 10, 8, 230)
SPECK_R = 2.2
MARGIN = 6.0
SPECK_GAP = 6.5
CORNER = 13.0
# What is ink on a face: a pixel this much darker than the paper round it (a
# median of MEDIAN px), or this much more coloured, in a patch OPEN px across
# at the least -- the dirt and the creases of a worn face are thinner, and
# are paper; a speck's middle keeps a square of CLEAR px round it free of ink.
INK_DARK = 28
INK_COLOUR = 45
MEDIAN = 21
OPEN = 3
CLEAR = 11


def at(style, i, j):
    step, slope = LATTICE[style]
    return CX + i * step / 2, CY + j * step / 2 * slope


def s(*v):
    """Card pixels on the S sheet."""
    return [a * S for a in v]


def sheet(ink):
    """A clear sheet a card large, S times, of one ink."""
    return Image.new("RGBA", (W * S, H * S), ink[:3] + (0,))


def reduce(big, ink):
    """The S sheet brought to a card's size: its alpha resampled, its colour the ink
    everywhere, so that neither a resampling nor a texture filter that reaches past an
    edge finds black there."""
    out = Image.new("RGBA", (W, H), ink[:3] + (0,))
    out.putalpha(big.getchannel("A").resize((W, H), LANCZOS))
    return out


def ground(back, x, y):
    """The ground's colour round a cell's middle, where no line runs (the mock-up's)."""
    px = [back.getpixel((int(x + dx), int(y + dy)))[:3] for dx in (-1, 0, 1) for dy in (-1, 0, 1)]
    return tuple(sum(c[k] for c in px) // len(px) for k in range(3))


# ---------------------------------------------------------------- the pencil

def pencil(face):
    """The face pencilled into the top corner of a back, as the mock-up wrote it: the rank
    in the hand, the suit's sign beside it a little lower."""
    rank, suit = face[0], face[1]
    text = "10" if rank == "T" else rank
    big = sheet(PENCIL)
    d = ImageDraw.Draw(big)
    hand = ImageFont.truetype(HAND, 30 * S)
    sym = ImageFont.truetype(SYMBOL, 24 * S)
    d.text((19 * S, 12 * S), text, font=hand, fill=PENCIL)
    d.text((19 * S + d.textlength(text, font=hand) + S, 20 * S), SUIT_SIGN[suit], font=sym, fill=PENCIL)
    return reduce(big, PENCIL)


def union_box(images):
    """The box all of them cover, a pixel round it: (x0, y0, x1, y1), x1 and y1 past it."""
    boxes = [im.getchannel("A").getbbox() for im in images]
    x0 = max(0, min(b[0] for b in boxes) - 1)
    y0 = max(0, min(b[1] for b in boxes) - 1)
    x1 = min(W, max(b[2] for b in boxes) + 1)
    y1 = min(H, max(b[3] for b in boxes) + 1)
    return x0, y0, x1, y1


# ---------------------------------------------------------------- places

def free(style, x, y, keep):
    x0, y0, x1, y1 = FIELD[style]
    if not (x0 + 8 <= x <= x1 - 8 and y0 + 8 <= y <= y1 - 8):
        return False
    ex, ey, er = EMBLEM[style]
    if math.hypot(x - ex, y - ey) < er:
        return False
    k0, k1, k2, k3 = keep
    return not (k0 - ROOM <= x <= k2 + ROOM and k1 - ROOM <= y <= k3 + ROOM)


def candidates(style, kind, keep):
    """Every place a fault of this kind can take: (place, the point it is judged by)."""
    out = []
    if kind == "nick":
        for side in (-1, 1):
            for y in range(60, 231, 2):
                out.append(((side, y), (CX + side * CX, float(y))))
        return out
    for j in range(-40, 41):
        for i in range(-40, 41):
            crossing = (i + j) % 2 == 0
            if kind == "block":
                if not crossing:
                    continue
                points = [at(style, i, j), at(style, i + 1, j + 1)]
            else:
                if crossing:
                    continue
                points = [at(style, i, j)]
                if kind == "shade":
                    points.append(at(style, i + 1, j + 1))
            if all(free(style, x, y, keep) for x, y in points):
                out.append(((i, j), points[0]))
    return out


def places(style, kind, keep):
    """Twenty places of a kind on a back, drawn by lot, GAP apart."""
    rng = random.Random("%s:%s" % (style, kind))
    pool = candidates(style, kind, keep)
    rng.shuffle(pool)
    chosen = []
    for place, (x, y) in pool:
        if all(math.hypot(x - cx, y - cy) >= GAP[kind] for _, (cx, cy) in chosen):
            chosen.append((place, (x, y)))
            if len(chosen) == PER_KIND:
                return [p for p, _ in chosen]
    raise SystemExit("error: %s: only %d places for a %s" % (style, len(chosen), kind))


# ---------------------------------------------------------------- the faults

def cell(style, i, j, shrink=0.66):
    """A cell of the lattice drawn smaller about its middle, clear of the lines round it."""
    x, y = at(style, i, j)
    step, slope = LATTICE[style]
    hx, hy = step / 2 * shrink, step / 2 * slope * shrink
    return s(x, y - hy, x + hx, y, x, y + hy, x - hx, y)


def fault(style, back, sign, spots):
    """The fault of a sign on a clear sheet a card large, and its kind."""
    kind = KINDS[(sign - 1) % len(KINDS)]
    slot = (sign - 1) // len(KINDS)
    place = spots[kind][slot]
    step, slope = LATTICE[style]
    if kind == "dot":
        ink = DOT
    elif kind == "block":
        ink = ground(back, *at(style, place[0] + 1, place[1])) + (255,)
    elif kind == "cut":
        ink = PALE[style]
    elif kind == "shade":
        ink = SHADE
    else:
        ink = NICK[style]
    big = sheet(ink)
    d = ImageDraw.Draw(big)
    g = GROW
    if kind == "dot":
        x, y = at(style, *place)
        d.ellipse(s(x - 3.4 - g, y - 3.0 - g, x + 3.2 + g, y + 3.4 + g), fill=ink)
    elif kind == "block":
        x, y = at(style, *place)
        dx, dy = step / 2, step / 2 * slope
        for k in (0, 1):                                # the crossing and the next one down the line
            cx, cy = x + k * dx, y + k * dy
            d.ellipse(s(cx - 3.6 - g, cy - 3.6 - g, cx + 3.6 + g, cy + 3.6 + g), fill=ink)
        d.line(s(x, y, x + dx, y + dy), fill=ink, width=round((5 + 2 * g) * S))
    elif kind == "cut":
        x, y = at(style, *place)
        d.line(s(x - 5, y + 3, x + 5, y - 3), fill=ink, width=round((3 + 2 * g) * S))
        d.ellipse(s(x + 5 - g, y - g, x + 7 + g, y + 2 + g), fill=ink)
        d.ellipse(s(x - 4 - g, y - 5 - g, x - 2 + g, y - 3 + g), fill=ink)
    elif kind == "shade":
        i, j = place
        d.polygon(cell(style, i, j), fill=ink)
        if slot % 2 == 1:                               # and the next cell along its strip
            d.polygon(cell(style, i + 1, j + 1), fill=ink)
    else:
        side, y = place
        edge = 0.0 if side < 0 else float(W)
        d.polygon(s(edge, y - 2.6, edge - side * 3.4, y, edge, y + 2.6), fill=ink)
    return reduce(big, ink), kind


def cut_out(img):
    """A picture cut to what it covers, a pixel round it, and that box (x, y, w, h)."""
    b = img.getchannel("A").getbbox()
    if not b:
        raise SystemExit("error: a mark drew nothing")
    x0, y0 = max(0, b[0] - 1), max(0, b[1] - 1)
    x1, y1 = min(W, b[2] + 1), min(H, b[3] + 1)
    return img.crop((x0, y0, x1, y1)), (x0, y0, x1 - x0, y1 - y0)


# ---------------------------------------------------------------- the specks

def face_names(style):
    """The faces a style has pictures of: art/cards/<style>/c_*.png, jokers too."""
    folder = os.path.join(CARDS, style)
    return sorted(n for n in os.listdir(folder) if n.startswith("c_") and n.endswith(".png"))


def inked(style):
    """Where any face of a style carries ink or a picture, at a card's size (255 inked,
    0 clear): each face read against the paper round every pixel, the darker or more
    coloured patches kept, all faces laid together. And how many faces there are."""
    out = None
    names = face_names(style)
    for name in names:
        rgb = Image.open(os.path.join(CARDS, style, name)).convert("RGB")
        if rgb.size != (W, H):
            raise SystemExit("error: %s/%s is %s, not %d x %d" % (style, name, rgb.size, W, H))
        lum = rgb.convert("L")
        paper = lum.filter(ImageFilter.MedianFilter(MEDIAN))
        dark = ImageChops.subtract(paper, lum).point(lambda v: 255 if v > INK_DARK else 0)
        r, g, b = rgb.split()
        colour = ImageChops.subtract(ImageChops.lighter(ImageChops.lighter(r, g), b),
                                     ImageChops.darker(ImageChops.darker(r, g), b))
        tinted = ImageChops.subtract(colour, colour.filter(ImageFilter.MedianFilter(MEDIAN)))
        ink = ImageChops.lighter(dark, tinted.point(lambda v: 255 if v > INK_COLOUR else 0))
        ink = ink.filter(ImageFilter.MinFilter(OPEN)).filter(ImageFilter.MaxFilter(OPEN))
        out = ink if out is None else ImageChops.lighter(out, ink)
    if out is None:
        raise SystemExit("error: no faces in " + os.path.join(CARDS, style))
    return out, len(names)


def margin_line():
    """The line a speck's middle may lie on: MARGIN in from a card's edge and round its
    corners, clockwise from the top left, a point about every half pixel."""
    pts = []
    r = CORNER - MARGIN

    def line(x0, y0, x1, y1):
        n = max(2, int(math.hypot(x1 - x0, y1 - y0) / 0.5))
        pts.extend((x0 + (x1 - x0) * k / n, y0 + (y1 - y0) * k / n) for k in range(n))

    def bend(cx, cy, a0):
        n = max(2, int(math.pi / 2 * r / 0.5))
        for k in range(n):
            a = math.radians(a0 + 90.0 * k / n)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))

    line(CORNER, MARGIN, W - CORNER, MARGIN)
    bend(W - CORNER, CORNER, -90)
    line(W - MARGIN, CORNER, W - MARGIN, H - CORNER)
    bend(W - CORNER, H - CORNER, 0)
    line(W - CORNER, H - MARGIN, CORNER, H - MARGIN)
    bend(CORNER, H - CORNER, 90)
    line(MARGIN, H - CORNER, MARGIN, CORNER)
    bend(CORNER, CORNER, 180)
    return pts


def speck_places(style, ink):
    """Ninety-nine places for a speck on the faces of a style, spread evenly along the
    stretches of the margin line clear of every face's ink and SPECK_GAP apart at the
    least, then dealt to the signs by lot: element n - 1 is sign n's middle. And the
    spacing they were spread at."""
    near = ink.filter(ImageFilter.MaxFilter(CLEAR))
    path = margin_line()
    clear = [near.getpixel((min(W - 1, int(round(x))), min(H - 1, int(round(y))))) == 0 for x, y in path]
    length = 0.0
    for k in range(1, len(path)):
        if clear[k] and clear[k - 1]:
            length += math.hypot(path[k][0] - path[k - 1][0], path[k][1] - path[k - 1][1])
    spacing = length / SIGNS
    while spacing >= SPECK_GAP:
        chosen, run = [], spacing
        for k, (x, y) in enumerate(path):
            if k > 0 and clear[k] and clear[k - 1]:
                run += math.hypot(x - path[k - 1][0], y - path[k - 1][1])
            if not clear[k] or run < spacing:
                continue
            if all(math.hypot(x - cx, y - cy) >= SPECK_GAP for cx, cy in chosen):
                chosen.append((x, y))
                run = 0.0
        if len(chosen) >= SIGNS:
            chosen = chosen[:SIGNS]
            random.Random("%s:specks" % style).shuffle(chosen)
            return chosen, spacing
        spacing *= 0.98
    raise SystemExit("error: %s: the margin has room for fewer than %d specks" % (style, SIGNS))


def speck():
    """The speck as one picture cut out round it, and the box it was cut from on a card
    where the speck's middle is the card's middle."""
    big = sheet(SPECK)
    d = ImageDraw.Draw(big)
    g, r = GROW, SPECK_R
    d.ellipse(s(CX - r - g, CY - r * 0.9 - g, CX + r * 0.95 + g, CY + r + g), fill=SPECK)
    return cut_out(reduce(big, SPECK))


def speck_box(box, place):
    """The box of the speck's picture on a card, for a speck whose middle is `place`."""
    bx, by, bw, bh = box
    return bx + int(round(place[0] - CX)), by + int(round(place[1] - CY)), bw, bh


def specks_sheet(style, ink, places, pic, box):
    """The queen of spades of a style with all ninety-nine specks on her, and beside her
    where the faces of the style carry ink (grey) with the specks laid on it; both twice a
    card's size."""
    on = Image.open(os.path.join(CARDS, style, "c_Qs.png")).convert("RGBA")
    grey = Image.new("RGBA", (W, H), (255, 255, 255, 255))
    grey.paste((150, 150, 150, 255), None, ink)
    for place in places:
        x, y, _, _ = speck_box(box, place)
        on.alpha_composite(pic, (x, y))
        grey.alpha_composite(pic, (x, y))
    big_on = on.resize((2 * W, 2 * H), LANCZOS)
    out = Image.new("RGB", (4 * W + 30, 2 * H + 20), (45, 45, 45))
    out.paste(big_on, (10, 10), big_on)
    out.paste(grey.resize((2 * W, 2 * H), LANCZOS).convert("RGB"), (2 * W + 20, 10))
    return out


# ---------------------------------------------------------------- the sheets

def pack(pieces, size):
    """Shelves, the tallest pieces first, 2 px apart; None when they do not fit."""
    order = sorted(pieces, key=lambda n: (-pieces[n].height, n))
    x, y, row, spots = 2, 2, 0, {}
    for name in order:
        w, h = pieces[name].size
        if x + w + 2 > size[0]:
            x, y, row = 2, y + row + 2, 0
        if y + h + 2 > size[1]:
            return None
        spots[name] = (x, y)
        x, row = x + w + 2, max(row, h)
    return spots


def atlas(pieces):
    """The pieces on the smallest sheet of the three that takes them, each with a pixel of
    its own ink round it, clear."""
    for size in ((512, 512), (1024, 512), (1024, 1024)):
        spots = pack(pieces, size)
        if spots:
            break
    else:
        raise SystemExit("error: the marks do not fit a sheet of 1024 x 1024")
    out = Image.new("RGBA", size, (0, 0, 0, 0))
    d = ImageDraw.Draw(out)
    for name, (x, y) in spots.items():
        im = pieces[name]
        d.rectangle((x - 1, y - 1, x + im.width, y + im.height), fill=im.getpixel((0, 0))[:3] + (0,))
        out.paste(im, (x, y))
    return out, spots


def signs_sheet(faulted):
    """The ninety-nine at half size, ten a row, sign 1 at the top left."""
    cols, w, h, gap = 10, W // 2, H // 2, 6
    rows = (len(faulted) + cols - 1) // cols
    out = Image.new("RGB", (cols * (w + gap) + gap, rows * (h + gap) + gap), (45, 45, 45))
    for k, im in enumerate(faulted):
        small = im.resize((w, h), LANCZOS)
        out.paste(small, (gap + (k % cols) * (w + gap), gap + (k // cols) * (h + gap)), small)
    return out


def view_sheet(rows):
    """A row a style: the back, a fault of each kind, a read back, a noticed face; at
    their size and as small as a fan shows them, as the mock-up's sheet."""
    n = len(rows[0])
    out = Image.new("RGB", (n * 210 + n * 80, len(rows) * 300 - 10), (45, 45, 45))
    for r, shown in enumerate(rows):
        for k, im in enumerate(shown):
            out.paste(im, (k * 210, 5 + r * 300), im)
            small = im.resize((70, 98), LANCZOS)
            out.paste(small, (n * 210 + k * 80, 5 + r * 300), small)
    return out


def imageset(size, spots, pieces):
    lines = ["// GENERATED by tools/make_marks.py -- edit the script, not this file.",
             "ImageSetClass {", ' Name "%s"' % SET, " RefSize %d %d" % size, " Textures {",
             "  ImageSetTextureClass {", "   mpix 0", '   path "%s"' % TEXTURE, "  }", " }", " Images {"]
    for name in sorted(spots):
        x, y = spots[name]
        w, h = pieces[name].size
        lines += ["  ImageSetDefClass %s {" % name, '   Name "%s"' % name, "   Pos %d %d" % (x, y),
                  "   Size %d %d" % (w, h), "   Flags 0", "  }"]
    lines += [" }", " Groups {", " }", "}"]
    return "\n".join(lines) + "\n"


CODE_TOP = """// GENERATED by tools/make_marks.py -- edit the script, not this file.
//
// Where the game window draws a mark (card sharp's design §4.4, §4.5), in
// pixels of a 200 x 280 card: on a back, the box of each sign's picture in
// set:ozg_marks, by the style of the back, and the box of the pencilled
// face; on a face, the box of the speck each sign puts in its margin, by the
// style of the face. A style with no signs here (another mod's deck) shows
// the pencil alone and no speck.
class OZG_MarkPlaces
{
    static const string IMAGESET = "ozg_marks";
    static const float CARD_W = 200;
    static const float CARD_H = 280;

    protected static ref map<string, ref array<int>> s_Boxes;

    // The box of sign `sign` on a back of `style`; false: none is drawn.
    static bool SignBox(string style, int sign, out int x, out int y, out int w, out int h)
    {
        return BoxOf(style + "_s" + sign.ToString(), x, y, w, h);
    }

    // The box the pencil writes a face in, the same for every face.
    static bool PencilBox(out int x, out int y, out int w, out int h)
    {
        return BoxOf("pencil", x, y, w, h);
    }

    static string SignImage(string style, int sign)
    {
        return "set:" + IMAGESET + " image:" + style + "_s" + sign.ToString();
    }

    static string PencilImage(string face)
    {
        return "set:" + IMAGESET + " image:p_" + face;
    }

    // The box of the speck sign `sign` puts on a face of `style`; false:
    // none is drawn.
    static bool SpeckBox(string style, int sign, out int x, out int y, out int w, out int h)
    {
        return BoxOf(style + "_f" + sign.ToString(), x, y, w, h);
    }

    // The speck: one picture for every sign and every style.
    static string SpeckImage()
    {
        return "set:" + IMAGESET + " image:speck";
    }

    protected static bool BoxOf(string key, out int x, out int y, out int w, out int h)
    {
        if (!s_Boxes)
            FillBoxes();
        array<int> b;
        if (!s_Boxes.Find(key, b) || !b)
            return false;
        x = b[0];
        y = b[1];
        w = b[2];
        h = b[3];
        return true;
    }

    protected static void PutBox(string key, int x, int y, int w, int h)
    {
        array<int> b = new array<int>();
        b.Insert(x);
        b.Insert(y);
        b.Insert(w);
        b.Insert(h);
        s_Boxes.Set(key, b);
    }

    protected static void FillBoxes()
    {
        s_Boxes = new map<string, ref array<int>>();
"""

CODE_END = """    }
}
"""


def code(boxes):
    keys = ["pencil"] + ["%s_s%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
    keys += ["%s_f%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
    body = ['        PutBox("%s", %d, %d, %d, %d);' % ((k,) + tuple(boxes[k])) for k in keys]
    return CODE_TOP + "\n".join(body) + "\n" + CODE_END


# ---------------------------------------------------------------- the run

def main():
    paa = IMAGE_TO_PAA
    if "--paa" in sys.argv:
        paa = sys.argv[sys.argv.index("--paa") + 1]
    if "--no-paa" in sys.argv or not os.path.isfile(paa):
        paa = None
    for font in (HAND, SYMBOL):
        if not os.path.isfile(font):
            raise SystemExit("error: the pencil's font is missing: " + font)
    for folder in (ART, BUILD, SETS):
        os.makedirs(folder, exist_ok=True)

    pieces, boxes = {}, {}
    pencils = {face: pencil(face) for face in FACES}
    keep = union_box(list(pencils.values()))
    for face, img in pencils.items():
        pieces["p_" + face] = img.crop(keep)
    boxes["pencil"] = (keep[0], keep[1], keep[2] - keep[0], keep[3] - keep[1])

    speck_pic, speck_cut = speck()
    pieces["speck"] = speck_pic
    rows = []
    for style in STYLES:
        back = Image.open(os.path.join(CARDS, style, "back.png")).convert("RGBA")
        if back.size != (W, H):
            raise SystemExit("error: the %s back is %s, not %d x %d" % (style, back.size, W, H))
        spots = {kind: places(style, kind, keep) for kind in KINDS}
        faulted, firsts = [], {}
        for sign in range(1, SIGNS + 1):
            full, kind = fault(style, back, sign, spots)
            name = "%s_s%d" % (style, sign)
            pieces[name], boxes[name] = cut_out(full)
            on = back.copy()
            on.alpha_composite(full)
            faulted.append(on)
            firsts.setdefault(kind, on)
        signs_sheet(faulted).save(os.path.join(ART, "%s_signs.png" % style))
        read = firsts["cut"].copy()
        read.alpha_composite(pencils["Kh"])
        ink, faces = inked(style)
        specks, spacing = speck_places(style, ink)
        for sign, place in enumerate(specks, start=1):
            boxes["%s_f%d" % (style, sign)] = speck_box(speck_cut, place)
        specks_sheet(style, ink, specks, speck_pic, speck_cut).save(os.path.join(ART, "%s_specks.png" % style))
        noticed = Image.open(os.path.join(CARDS, style, "c_Kh.png")).convert("RGBA")
        x, y, _, _ = boxes["%s_f1" % style]
        noticed.alpha_composite(speck_pic, (x, y))
        rows.append([back] + [firsts[k] for k in KINDS] + [read, noticed])
        print("%s: %d signs, %d places of each kind; %d specks round %d faces, %.1f px apart"
              % (style, SIGNS, PER_KIND, SIGNS, faces, spacing))
    view_sheet(rows).save(os.path.join(ART, "_view.png"))

    sheet_img, spots = atlas(pieces)
    png = os.path.join(BUILD, SET + "_ca.png")
    sheet_img.save(png)
    with io.open(os.path.join(SETS, SET + ".imageset"), "w", encoding="utf-8", newline="\n") as f:
        f.write(imageset(sheet_img.size, spots, pieces))
    with io.open(CODE, "w", encoding="utf-8", newline="\n") as f:
        f.write(code(boxes))
    print("%s: %d pictures on %d x %d; OZG_MarkPlaces.c: %d boxes"
          % (SET, len(pieces), sheet_img.size[0], sheet_img.size[1], len(boxes)))
    if paa is None:
        print("no .paa made")
        return
    target = os.path.join(SETS, SET + "_ca.paa")
    if os.path.exists(target):
        os.remove(target)
    done = subprocess.run([paa, png, target], capture_output=True, text=True, stdin=subprocess.DEVNULL)
    if done.returncode != 0 or not os.path.isfile(target):
        raise SystemExit("error: ImageToPAA failed: " + done.stdout + done.stderr)
    print("%s_ca.paa: %d KB" % (SET, os.path.getsize(target) // 1024))


if __name__ == "__main__":
    main()
