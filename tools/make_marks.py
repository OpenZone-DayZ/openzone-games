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
# Usage: python -I tools/make_marks.py [--paa <ImageToPAA.exe>] [--no-paa]
#
# Reads art/cards/<style>/back.png (tools/make_cards.py). Writes
# art/marks/<style>_signs.png (the ninety-nine on that back, to look at),
# art/marks/_view.png (each kind and a read back, as the mock-up's sheet),
# build/marks/ozg_marks_ca.png, OpenZone_Games/gui/imagesets/ozg_marks.imageset
# and, given ImageToPAA, ozg_marks_ca.paa beside it, and
# OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c.
import io
import math
import os
import random
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

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
    """A row a style: the back, a fault of each kind, a read back; at their size and as
    small as a fan shows them, as the mock-up's sheet."""
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
// Where the game window draws a mark on a back (card sharp's design §4.4,
// §4.5): the box of each sign's picture in set:ozg_marks, by the style of the
// back, and the box of the pencilled face, in pixels of a 200 x 280 card. A
// style with no signs here (another mod's deck) shows the pencil alone.
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
        rows.append([back] + [firsts[k] for k in KINDS] + [read])
        print("%s: %d signs, %d places of each kind" % (style, SIGNS, PER_KIND))
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
