"""Draw the texture sources of the models. Pillow and the standard library, nothing else.

    python -I models/src/make_textures.py [place] [deck] [mark] [card] [sharpkit]

With no name it draws them all. It writes the PNG sources the shipped .paa are converted
from (asset_convert):

    models/place/data/ozg_place_co.png             512  the mat: dark green felt, a lighter sewn
                                                        binding, worn
    models/place/data/ozg_place_nohq.png           512  its relief: the binding, the stitches,
                                                        the folds, the burns
    models/place/data/ozg_place_smdi.png           256  where it shines: thread, rubbed felt, stains
    models/cards/data/ozg_deck_nohq.png            256  the deck sheet's relief: the side of the stack
    models/cards/data/ozg_deck_smdi.png             64  the deck sheet's shine
    models/mark/data/ozg_mark_nohq.png              16  flat
    models/mark/data/ozg_mark_smdi.png              16  no shine
    models/cards/data/ozg_card_nohq.png             16  one card: flat
    models/cards/data/ozg_card_smdi.png             16  one card: a soft sheen
    models/sharpkit/data/ozg_sharpkit_co.png       512  the card sharp's kit: an open tin and
                                                        the handmade things in it, every part
                                                        where sharpkit_atlas.py puts it
    models/sharpkit/data/ozg_sharpkit_nohq.png     512  its relief: the tin's beads, the file's
                                                        teeth, the tape, the creased tube
    models/sharpkit/data/ozg_sharpkit_smdi.png     256  where it shines: glass, the blade and
                                                        bare steel most, paper and sand hardly

Everything is drawn from fixed seeds: a second run gives the same pictures.

The normal maps are written the way the game reads them (as DirectX does): red is the
slope to the right, green is the slope DOWN the picture. ImageToPAA does the swizzle
of a "_nohq" by the file's name; a "_smdi" keeps green (how strong the highlight is)
and blue (how tight), and red is set to one.

The deck sheet, 512 x 512, origin at the top left:
    x   0..255, y   0..511   the card back, stretched to fill
    x 256..511, y   0..383   a blank card face
    x 256..511, y 384..511   the edge band: the side of a stack of cards
"""
import math
import os
import random
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageOps

HERE = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(HERE)
REPO = os.path.dirname(MODELS)

LANCZOS = Image.Resampling.LANCZOS
BICUBIC = Image.Resampling.BICUBIC
BOX = Image.Resampling.BOX

# ----------------------------------------------------------------------- the mat
# The mat is 500 x 350 mm and its top face takes the whole picture, so a texel is
# not square on the cloth. Everything is drawn on a canvas of square pixels, four
# to a millimetre, and squeezed into the texture at the end.
MAT_W_MM = 500
MAT_H_MM = 350
PX = 4                       # canvas pixels per millimetre
CW = MAT_W_MM * PX
CH = MAT_H_MM * PX
PLACE_SIZE = 512
PLACE_SMDI_SIZE = 256

MAT_CORNER_MM = 15.0         # the outline's corner radius: models/src/place.py has the same
TAPE_MM = 12.0               # the binding sewn round the edge
TAPE_CORNER_MM = 6.0         # the corner of its inner edge
STITCH_INSET_MM = 8.5        # the stitch line, from the mat's edge
STITCH_PITCH_MM = 8.0
STITCH_LEN_MM = 5.0
STITCH_WIDTH_MM = 1.5

FELT_DARK = (20, 50, 32)
FELT_LIGHT = (50, 98, 62)
WORN_DARK = (50, 80, 58)
WORN_LIGHT = (92, 124, 94)
STAIN = (30, 36, 22)
TAPE_DARK = (104, 108, 74)
TAPE_LIGHT = (162, 162, 118)
THREAD = (198, 190, 160)
BURN_CORE = (16, 13, 10)
BURN_HALO = (58, 44, 26)

# The height the relief is drawn in: 170 grey levels to a millimetre, so the whole
# range is 1.5 mm. The felt lies at FELT_LEVEL.
LEVELS_PER_MM = 170.0
FELT_LEVEL = 60
PLACE_RELIEF = 1.6           # the slopes are made this much steeper than they are


def mm(v):
    return int(round(v * PX))


def lut(fn):
    return [max(0, min(255, int(round(fn(v))))) for v in range(256)]


def curve(mask, lo, hi):
    """Stretch lo..hi to black..white with soft ends."""
    def f(v):
        t = (v - lo) / float(hi - lo)
        t = max(0.0, min(1.0, t))
        return 255.0 * t * t * (3.0 - 2.0 * t)
    return mask.point(lut(f))


def scale(mask, k):
    return mask.point(lut(lambda v: v * k))


def value_noise(rng, size, cell):
    """Smooth noise: random values on a grid `cell` pixels wide, enlarged."""
    w, h = size
    if cell <= 1:
        return Image.frombytes("L", size, rng.randbytes(w * h))
    gw = w // cell + 3
    gh = h // cell + 3
    small = Image.frombytes("L", (gw, gh), rng.randbytes(gw * gh))
    big = small.resize((gw * cell, gh * cell), BICUBIC)
    return big.crop((cell, cell, cell + w, cell + h))


def fbm(rng, size, cells, weights):
    """Several noises of different grain as one, stretched to the whole range."""
    out = None
    total = 0.0
    for cell, weight in zip(cells, weights):
        layer = value_noise(rng, size, cell)
        if out is None:
            out = layer
        else:
            out = Image.blend(out, layer, weight / (total + weight))
        total += weight
    return ImageOps.autocontrast(out, cutoff=0.5)


def tint(mask, dark, light):
    """A picture that runs from `dark` where the mask is black to `light` where it is white."""
    return Image.composite(Image.new("RGB", mask.size, light), Image.new("RGB", mask.size, dark), mask)


def blur(img, radius_mm):
    return img.filter(ImageFilter.GaussianBlur(radius_mm * PX))


class RoundedPath:
    """A rounded rectangle walked clockwise by arc length, from the left end of its top side."""

    def __init__(self, cx, cy, a, b, r):
        self.cx, self.cy, self.a, self.b, self.r = cx, cy, a, b, r
        self.sx = 2.0 * (a - r)
        self.sy = 2.0 * (b - r)
        self.arc = 0.5 * math.pi * r
        self.length = 2.0 * (self.sx + self.sy) + 4.0 * self.arc

    def at(self, s):
        cx, cy, a, b, r = self.cx, self.cy, self.a, self.b, self.r
        s %= self.length
        parts = (
            (self.sx, lambda t: (cx - a + r + t, cy - b)),
            (self.arc, lambda t: self._on_arc(cx + a - r, cy - b + r, -90.0, t)),
            (self.sy, lambda t: (cx + a, cy - b + r + t)),
            (self.arc, lambda t: self._on_arc(cx + a - r, cy + b - r, 0.0, t)),
            (self.sx, lambda t: (cx + a - r - t, cy + b)),
            (self.arc, lambda t: self._on_arc(cx - a + r, cy + b - r, 90.0, t)),
            (self.sy, lambda t: (cx - a, cy + b - r - t)),
            (self.arc, lambda t: self._on_arc(cx - a + r, cy - b + r, 180.0, t)),
        )
        for length, point in parts:
            if s <= length:
                return point(s)
            s -= length
        return parts[0][1](0.0)

    def _on_arc(self, ox, oy, start_deg, t):
        angle = math.radians(start_deg) + t / self.r
        return ox + self.r * math.cos(angle), oy + self.r * math.sin(angle)


def stitches(rng):
    """The stitches of the binding: (x0, y0, x1, y1, shade) in canvas pixels. Some are gone."""
    path = RoundedPath(MAT_W_MM / 2.0, MAT_H_MM / 2.0,
                       MAT_W_MM / 2.0 - STITCH_INSET_MM, MAT_H_MM / 2.0 - STITCH_INSET_MM,
                       MAT_CORNER_MM - STITCH_INSET_MM)
    count = int(round(path.length / STITCH_PITCH_MM))
    pitch = path.length / count
    gone = set()
    for start, run in ((int(count * 0.19), 4), (int(count * 0.62), 6), (int(count * 0.83), 3)):
        gone.update(range(start, start + run))
    out = []
    for i in range(count):
        lost = rng.random() < 0.03
        shade = 0.82 + 0.22 * rng.random()
        wobble = (rng.random() - 0.5) * 0.9
        long = STITCH_LEN_MM * (0.9 + 0.2 * rng.random())
        if i in gone or lost:
            continue
        s = (i + 0.5) * pitch + wobble
        x0, y0 = path.at(s - long / 2.0)
        x1, y1 = path.at(s + long / 2.0)
        out.append((x0 * PX, y0 * PX, x1 * PX, y1 * PX, shade))
    return out


def draw_dashes(draw, dashes, width, colour=None):
    """Short thick strokes with round ends. With no colour each is drawn in its own shade of thread."""
    half = width / 2.0
    for x0, y0, x1, y1, shade in dashes:
        fill = colour
        if fill is None:
            fill = tuple(int(round(c * shade)) for c in THREAD)
        draw.line([(x0, y0), (x1, y1)], fill=fill, width=int(round(width)))
        for x, y in ((x0, y0), (x1, y1)):
            draw.ellipse([x - half, y - half, x + half, y + half], fill=fill)


def soft_spots(spots, radius_mm):
    """White ovals (x, y, rx, ry, strength) in millimetres, blurred into one mask."""
    mask = Image.new("L", (CW, CH), 0)
    draw = ImageDraw.Draw(mask)
    for x, y, rx, ry, strength in spots:
        draw.ellipse([mm(x - rx), mm(y - ry), mm(x + rx), mm(y + ry)], fill=int(strength))
    return blur(mask, radius_mm)


def place_layers():
    """Everything the three maps of the mat are made of, as masks and pictures on the canvas."""
    rng = random.Random(20261124)
    size = (CW, CH)
    layers = {}

    # The cloth itself: blotches, patches, fibres.
    blotch = fbm(rng, size, (480, 200), (1.0, 0.7))
    patch = fbm(rng, size, (64, 24), (1.0, 0.8))
    fibre = fbm(rng, size, (8, 4, 2), (1.0, 1.0, 0.6))
    tone = Image.blend(Image.blend(blotch, patch, 0.35), fibre, 0.42)
    layers["fibre"] = fibre
    felt = tint(tone, FELT_DARK, FELT_LIGHT)

    # Rubbed pale where hands and cards work: the middle, and in front of every side.
    wear = soft_spots((
        (250, 175, 130, 62, 190),
        (250, 52, 150, 30, 150), (250, 298, 150, 30, 165),
        (62, 175, 26, 80, 120), (438, 175, 26, 80, 130),
    ), 26.0)
    wear = ImageChops.multiply(wear, curve(fbm(rng, size, (140, 40, 12), (1.0, 0.8, 0.5)), 40, 215))
    layers["wear"] = wear
    felt = Image.composite(tint(tone, WORN_DARK, WORN_LIGHT), felt, scale(wear, 0.72))

    # Folded in four and carried: a pale ridge along each fold.
    crease = Image.new("L", size, 0)
    draw = ImageDraw.Draw(crease)
    x = MAT_W_MM / 2.0 + 1.5
    points = [(mm(x + 0.9 * math.sin(i * 1.7)), mm(i * MAT_H_MM / 14.0)) for i in range(15)]
    draw.line(points, fill=255, width=mm(1.6))
    y = MAT_H_MM / 2.0 - 2.0
    points = [(mm(i * MAT_W_MM / 20.0), mm(y + 0.9 * math.sin(i * 1.3 + 1.0))) for i in range(21)]
    draw.line(points, fill=255, width=mm(1.6))
    crease = blur(crease, 0.9)
    layers["crease"] = crease
    felt = Image.composite(tint(tone, WORN_DARK, WORN_LIGHT), felt, scale(crease, 0.55))
    felt = Image.composite(tint(tone, WORN_DARK, WORN_LIGHT), felt, scale(blur(crease, 5.0), 0.5))

    # Stains: a few dark blots, and the ring a mug left twice.
    blots = curve(fbm(rng, size, (130, 48, 16), (1.0, 0.8, 0.5)), 172, 222)
    blots = ImageChops.multiply(blots, soft_spots((
        (120, 250, 70, 45, 255), (390, 90, 60, 50, 255), (300, 220, 50, 40, 200),
    ), 14.0))
    ring = Image.new("L", size, 0)
    draw = ImageDraw.Draw(ring)
    for cx, cy, r, strength in ((104, 92, 37, 230), (131, 108, 37, 150)):
        draw.ellipse([mm(cx - r), mm(cy - r), mm(cx + r), mm(cy + r)], outline=strength, width=mm(2.6))
    ring = ImageChops.multiply(blur(ring, 0.8), curve(fbm(rng, size, (90, 30), (1.0, 0.7)), 60, 190))
    stain = ImageChops.lighter(blots, ring)
    layers["stain"] = stain
    felt = Image.composite(Image.new("RGB", size, STAIN), felt, scale(stain, 0.5))

    # Burns: somebody put a cigarette out here, more than once.
    burn = Image.new("L", size, 0)
    draw = ImageDraw.Draw(burn)
    for cx, cy, r in ((338, 292, 3.2), (146, 246, 2.4), (421, 58, 2.8), (226, 131, 1.8)):
        draw.ellipse([mm(cx - r), mm(cy - r * 0.85), mm(cx + r), mm(cy + r * 0.85)], fill=255)
    ragged = curve(ImageChops.multiply(blur(burn, 0.5), fbm(rng, size, (10, 4), (1.0, 0.8))), 40, 120)
    halo = blur(burn, 2.2)
    layers["burn"] = blur(ragged, 0.4)
    felt = Image.composite(Image.new("RGB", size, BURN_HALO), felt, scale(curve(halo, 10, 200), 0.75))
    felt = Image.composite(Image.new("RGB", size, BURN_CORE), felt, ragged)

    # The binding: a lighter tape sewn round the edge, dirtier than the felt.
    tape = Image.new("L", size, 255)
    draw = ImageDraw.Draw(tape)
    draw.rounded_rectangle([mm(TAPE_MM), mm(TAPE_MM), CW - mm(TAPE_MM), CH - mm(TAPE_MM)],
                           radius=mm(TAPE_CORNER_MM), fill=0)
    layers["tape_hard"] = tape
    tape = blur(tape, 0.2)
    layers["tape"] = tape
    weave = fbm(rng, size, (220, 40, 6, 2), (1.0, 0.7, 0.8, 0.5))
    layers["weave"] = weave
    cloth = tint(weave, TAPE_DARK, TAPE_LIGHT)
    grime = curve(fbm(rng, size, (160, 50, 14), (1.0, 0.8, 0.4)), 110, 235)
    cloth = Image.composite(Image.new("RGB", size, (58, 56, 40)), cloth, scale(grime, 0.45))
    layers["grime"] = grime

    # Under the tape's inner edge the felt lies in its shade and keeps the dirt.
    edge = Image.new("L", size, 0)
    draw = ImageDraw.Draw(edge)
    inset = TAPE_MM + 0.8
    draw.rounded_rectangle([mm(inset), mm(inset), CW - mm(inset), CH - mm(inset)],
                           radius=mm(TAPE_CORNER_MM), outline=255, width=mm(2.2))
    felt = Image.composite(Image.new("RGB", size, (10, 18, 12)), felt, scale(blur(edge, 1.1), 0.5))

    picture = Image.composite(cloth, felt, tape)

    # The stitches: their shade first, then the thread, then the needle holes.
    dashes = stitches(rng)
    thread = Image.new("L", size, 0)
    draw_dashes(ImageDraw.Draw(thread), dashes, mm(STITCH_WIDTH_MM), 255)
    layers["thread"] = thread
    shade = blur(ImageChops.offset(thread, mm(0.45), mm(0.45)), 0.45)
    picture = Image.composite(Image.new("RGB", size, (30, 32, 22)), picture, scale(shade, 0.6))
    draw_dashes(ImageDraw.Draw(picture), dashes, mm(STITCH_WIDTH_MM))
    holes = Image.new("L", size, 0)
    draw = ImageDraw.Draw(holes)
    r = 0.55 * PX
    for x0, y0, x1, y1, _ in dashes:
        for x, y in ((x0, y0), (x1, y1)):
            draw.ellipse([x - r, y - r, x + r, y + r], fill=255)
    holes = blur(holes, 0.12)
    layers["holes"] = holes
    picture = Image.composite(Image.new("RGB", size, (26, 26, 18)), picture, scale(holes, 0.7))
    # The thread is cloth too: it takes the grime and has a grain of its own.
    picture = Image.composite(Image.new("RGB", size, (70, 66, 48)), picture,
                              scale(ImageChops.multiply(thread, grime), 0.5))

    # Dirt gathers towards the edge the mat is picked up by.
    inner = Image.new("L", size, 0)
    draw = ImageDraw.Draw(inner)
    draw.rounded_rectangle([mm(3.5), mm(3.5), CW - mm(3.5), CH - mm(3.5)], radius=mm(MAT_CORNER_MM - 3.5),
                           fill=255)
    inner = blur(inner, 4.5).point(lut(lambda v: 176 + v * 79.0 / 255.0))
    picture = ImageChops.multiply(picture, Image.merge("RGB", (inner, inner, inner)))
    cast = fbm(rng, size, (700, 300), (1.0, 0.6)).point(lut(lambda v: 226 + v * 29.0 / 255.0))
    picture = ImageChops.multiply(picture, Image.merge("RGB", (cast, cast, cast)))

    layers["picture"] = picture
    layers["rng"] = rng
    return layers


def place_colour(layers):
    rng = layers["rng"]
    small = layers["picture"].resize((PLACE_SIZE, PLACE_SIZE), LANCZOS)
    grain = value_noise(rng, small.size, 1).point(lut(lambda v: 240 + v * 15.0 / 255.0))
    small = ImageChops.multiply(small, Image.merge("RGB", (grain, grain, grain)))
    return small.filter(ImageFilter.UnsharpMask(radius=1.0, percent=45, threshold=0))


def place_height(layers):
    """The relief of the mat as grey levels, LEVELS_PER_MM of them to a millimetre."""
    size = (CW, CH)
    fibre = layers["fibre"]
    height = fibre.point(lut(lambda v: FELT_LEVEL + (v - 128) * 0.20))
    # Rubbed felt lies flatter and a little lower.
    height = Image.composite(Image.new("L", size, FELT_LEVEL - 9), height, scale(layers["wear"], 0.6))
    height = ImageChops.add(height, scale(layers["crease"], 0.20 * LEVELS_PER_MM / 255.0))
    height = ImageChops.subtract(height, scale(layers["burn"], 0.30 * LEVELS_PER_MM / 255.0))
    # The tape lies on the felt, 0.55 mm proud, with a rounded shoulder.
    top = FELT_LEVEL + 0.55 * LEVELS_PER_MM
    tape = layers["weave"].point(lut(lambda v: top + (v - 128) * 0.05))
    height = Image.composite(tape, height, blur(layers["tape_hard"], 0.45))
    height = ImageChops.add(height, scale(blur(layers["thread"], 0.3), 0.40 * LEVELS_PER_MM / 255.0))
    height = ImageChops.subtract(height, scale(layers["holes"], 0.30 * LEVELS_PER_MM / 255.0))
    return height


def normal_map(height, out_size, relief, levels_per_mm, px_per_mm):
    """A normal map of a height picture, squeezed to `out_size`.

    The slope is taken across two canvas pixels, squeezed, and only then turned into a
    vector: averaging slopes is right, averaging vectors shortens them."""
    smooth = height.filter(ImageFilter.GaussianBlur(1.0))
    right = ImageChops.offset(smooth, -1, 0)     # the pixel to the right
    left = ImageChops.offset(smooth, 1, 0)
    below = ImageChops.offset(smooth, 0, -1)     # the pixel below
    above = ImageChops.offset(smooth, 0, 1)
    gx = ImageChops.subtract(right, left, 1.0, 128).resize(out_size, LANCZOS).tobytes()
    gy = ImageChops.subtract(below, above, 1.0, 128).resize(out_size, LANCZOS).tobytes()
    # One grey level across two canvas pixels, as a slope.
    k = relief / (levels_per_mm * 2.0 / px_per_mm)
    out = bytearray(len(gx) * 3)
    for i in range(len(gx)):
        sx = (gx[i] - 128) * k
        sy = (gy[i] - 128) * k
        inv = 1.0 / math.sqrt(sx * sx + sy * sy + 1.0)
        out[3 * i] = int(round(127.5 - 127.5 * sx * inv))
        out[3 * i + 1] = int(round(127.5 - 127.5 * sy * inv))
        out[3 * i + 2] = int(round(127.5 + 127.5 * inv))
    return Image.frombytes("RGB", out_size, bytes(out))


def place_shine(layers):
    """Green: how strong the highlight is. Blue: how tight. Felt has next to none of either."""
    size = (CW, CH)
    strong = Image.new("L", size, 14)
    tight = Image.new("L", size, 30)
    for mask, s, t in ((layers["wear"], 22, 46), (layers["stain"], 30, 66), (layers["crease"], 12, 24)):
        strong = ImageChops.add(strong, scale(mask, s / 255.0))
        tight = ImageChops.add(tight, scale(mask, t / 255.0))
    strong = ImageChops.subtract(strong, scale(layers["burn"], 14 / 255.0))
    tape = layers["tape"]
    strong = Image.composite(Image.new("L", size, 24), strong, tape)
    tight = Image.composite(Image.new("L", size, 46), tight, tape)
    grime = scale(ImageChops.multiply(layers["grime"], tape), 0.08)
    strong = ImageChops.add(strong, grime)
    tight = ImageChops.add(tight, scale(grime, 2.0))
    thread = layers["thread"]
    strong = Image.composite(Image.new("L", size, 40), strong, thread)
    tight = Image.composite(Image.new("L", size, 70), tight, thread)
    out = (PLACE_SMDI_SIZE, PLACE_SMDI_SIZE)
    return Image.merge("RGB", (Image.new("L", out, 255), strong.resize(out, BOX), tight.resize(out, BOX)))


# ----------------------------------------------------------------- the deck sheet
def deck_relief():
    """Flat but for the edge band, where the cards lie on each other as soft ridges."""
    size = 256
    rng = random.Random(52)
    img = Image.new("RGB", (size, size), (128, 128, 255))
    x0, y0, rows = size // 2, size * 3 // 4, size // 4
    # A height for every row of the band: cards in uneven little packs, flat at both ends.
    profile = [0.0] * rows
    level = 0.0
    for j in range(rows):
        level = 0.6 * level + (rng.random() - 0.5)
        profile[j] = level
    for _ in range(2):
        profile = [(profile[max(j - 1, 0)] + 2.0 * profile[j] + profile[min(j + 1, rows - 1)]) / 4.0
                   for j in range(rows)]
    for j in range(3):
        profile[j] *= j / 3.0
        profile[rows - 1 - j] *= j / 3.0
    px = img.load()
    for j in range(rows):
        slope = (profile[min(j + 1, rows - 1)] - profile[max(j - 1, 0)]) * 0.9
        inv = 1.0 / math.sqrt(slope * slope + 1.0)
        g = int(round(127.5 - 127.5 * slope * inv))
        b = int(round(127.5 + 127.5 * inv))
        for i in range(x0, size):
            px[i, y0 + j] = (128, g, b)
    return img


def deck_shine():
    """The back is coated and has a soft sheen, the face less, the side of the stack none."""
    size = 64
    strong = Image.new("L", (size, size), 26)
    tight = Image.new("L", (size, size), 90)
    draw_s = ImageDraw.Draw(strong)
    draw_t = ImageDraw.Draw(tight)
    half, band = size // 2, size * 3 // 4
    draw_s.rectangle([half, 0, size - 1, band - 1], fill=18)
    draw_t.rectangle([half, 0, size - 1, band - 1], fill=64)
    draw_s.rectangle([half, band, size - 1, size - 1], fill=10)
    draw_t.rectangle([half, band, size - 1, size - 1], fill=31)
    return Image.merge("RGB", (Image.new("L", (size, size), 255), strong, tight))


# ---------------------------------------------------------------------- the mark
def flat(size, colour):
    return Image.new("RGB", (size, size), colour)


def save(img, *parts, root=MODELS):
    path = os.path.join(root, *parts)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path, optimize=True)
    shown = os.path.relpath(root, REPO).replace(os.sep, "/")
    print("%-48s %4d x %-4d %s" % ("/".join((shown,) + parts), img.width, img.height, img.mode))


# ------------------------------------------------------------ the card sharp's kit
# An open tin, standing in its lid, and the handmade things a card sharp keeps in it
# (models/src/sharpkit.py). Every part lies on one picture, where sharpkit_atlas.py says;
# it is drawn on a canvas twice the size and halved.
KIT_SIZE = 512
KIT_SMDI_SIZE = 256
KIT_LEVEL = 100              # the height every part lies at; the relief goes up and down from it
KIT_LEVELS_PER_MM = 400.0    # 255 levels are 0.64 mm
KIT_PX_PER_MM = 12.0         # about: the regions are not all drawn to one scale
KIT_RELIEF = 1.3
KIT_FONT = os.path.join(REPO, "art", "fonts", "oswald", "Oswald[wght].ttf")

ENAMEL = ((40, 48, 33), (78, 88, 60))
STEEL = ((100, 101, 97), (170, 169, 162))
TINPLATE = ((104, 102, 96), (174, 172, 164))
RUST = ((64, 35, 19), (140, 80, 40))
ALU = ((142, 144, 142), (214, 216, 212))
WOOD = ((104, 68, 36), (182, 138, 86))
LEAD = ((70, 72, 80), (120, 122, 130))
SAND = ((98, 68, 40), (166, 122, 76))
GRIME = (62, 56, 44)
GLUE_PRINT = (204, 156, 36)
GLUE_INK = (112, 20, 16)
PAPER_OLD = (218, 204, 168)
INK = (12, 12, 20)
GLASS = (20, 22, 26)
BAKELITE = (26, 22, 20)
CAP_RED = (146, 30, 22)
TAPE = (24, 24, 26)

# (how strong the highlight is, how tight) for each stuff, as the _smdi's green and blue.
KIT_SHINE = {
    "enamel": (64, 96), "steel": (150, 140), "tin": (168, 160), "rust": (12, 18),
    "alu": (150, 120), "print": (56, 80), "paper": (18, 26), "glass": (230, 236),
    "bakelite": (96, 130), "red": (84, 110), "wood": (22, 28), "tape": (52, 64),
    "lead": (40, 46), "sand": (4, 8), "blade": (205, 210), "grime": (20, 24),
}


def kit_atlas():
    sys.path.insert(0, HERE)
    import sharpkit_atlas
    return sharpkit_atlas


def px_blur(img, radius):
    return img.filter(ImageFilter.GaussianBlur(radius))


def mix(*parts):
    """Masks added up, each scaled: mix((mask, 0.6), (other, 0.4))."""
    out = None
    for mask, k in parts:
        layer = scale(mask, k)
        out = layer if out is None else ImageChops.add(out, layer)
    return out


def kit_shine(size, base, *layers):
    """The shine of a region: `base` stuff everywhere, each (mask, stuff) over it."""
    s, t = KIT_SHINE[base]
    strong = Image.new("L", size, s)
    tight = Image.new("L", size, t)
    for mask, name in layers:
        s, t = KIT_SHINE[name]
        strong = Image.composite(Image.new("L", size, s), strong, mask)
        tight = Image.composite(Image.new("L", size, t), tight, mask)
    return strong, tight


def kit_scratches(rng, size, count, length, width=1, angle=None, spread=20.0):
    """Thin straight scratches, white on black. With `angle` (degrees from the x axis) they
    run that way, give or take `spread`; without it, every way."""
    mask = Image.new("L", size, 0)
    draw = ImageDraw.Draw(mask)
    w, h = size
    for _ in range(count):
        x, y = rng.uniform(0, w), rng.uniform(0, h)
        a = rng.uniform(0.0, 180.0) if angle is None else angle + rng.uniform(-spread, spread)
        a = math.radians(a)
        n = rng.uniform(*length)
        draw.line([(x, y), (x + math.cos(a) * n, y + math.sin(a) * n)],
                  fill=int(255 * rng.uniform(0.45, 1.0)), width=width)
    return mask


def band_wear(rng, size, top, bottom, lo=150, hi=200):
    """Where the paint of a band round a tin is rubbed off: near its top edge (within `top`
    pixels) and its bottom edge (`bottom`), broken up by noise."""
    w, h = size
    ramp = Image.new("L", size, 0)
    draw = ImageDraw.Draw(ramp)
    for y in range(h):
        v = 0.0
        if top:
            v = max(v, 1.0 - y / float(top))
        if bottom:
            v = max(v, 1.0 - (h - 1 - y) / float(bottom))
        draw.line([(0, y), (w, y)], fill=int(255 * max(0.0, v)))
    noise = fbm(rng, size, (40, 12, 4), (1.0, 0.6, 0.35))
    return curve(mix((ramp, 0.75), (noise, 0.55)), lo, hi)


def kit_text(size, text, fit, turn=0, weight=600):
    """`text` in white on black, Oswald, as large as fits in `fit` = (along the line, across
    it) pixels, in the middle of a picture of `size`, turned by `turn` degrees
    (anticlockwise, as PIL turns)."""
    font = ImageFont.truetype(KIT_FONT, 200)
    try:
        font.set_variation_by_axes([weight])
    except (OSError, AttributeError):
        pass
    box = font.getbbox(text)
    tw, th = box[2] - box[0], box[3] - box[1]
    words = Image.new("L", (tw, th), 0)
    ImageDraw.Draw(words).text((-box[0], -box[1]), text, font=font, fill=255)
    k = min(fit[0] / float(tw), fit[1] / float(th))
    words = words.resize((max(1, int(tw * k)), max(1, int(th * k))), LANCZOS)
    if turn:
        words = words.rotate(turn, resample=BICUBIC, expand=True)
    out = Image.new("L", size, 0)
    out.paste(words, ((size[0] - words.width) // 2, (size[1] - words.height) // 2))
    return out


def blob(rng, size, cx, cy, r, drops=6):
    """A spilt drop with a few splashes round it, white on black, soft."""
    mask = Image.new("L", size, 0)
    draw = ImageDraw.Draw(mask)
    draw.ellipse([cx - r, cy - r * 0.8, cx + r, cy + r * 0.8], fill=255)
    for _ in range(drops):
        a = rng.uniform(0.0, 2.0 * math.pi)
        d = r * rng.uniform(0.9, 1.9)
        s = r * rng.uniform(0.12, 0.35)
        x, y = cx + math.cos(a) * d, cy + math.sin(a) * d * 0.8
        draw.ellipse([x - s, y - s, x + s, y + s], fill=255)
    return px_blur(mask, r * 0.08)


def kit_wall(rng, size, top, bottom, beads=()):
    """A band of olive enamel round a tin, rubbed down to the steel at its edges and
    scratched up and down; `beads` are the rows a raised bead runs round at."""
    colour = tint(fbm(rng, size, (60, 18, 5), (1.0, 0.5, 0.3)), *ENAMEL)
    bare = ImageChops.lighter(band_wear(rng, size, top, bottom),
                              kit_scratches(rng, size, size[0] // 14, (5, 26), 2, angle=90.0))
    for y in beads:
        ridge = Image.new("L", size, 0)
        ImageDraw.Draw(ridge).line([(0, y), (size[0], y)], fill=255, width=3)
        bare = ImageChops.lighter(bare, curve(ImageChops.multiply(
            ridge, fbm(rng, size, (30, 8), (1.0, 0.5))), 60, 130))
    colour = Image.composite(tint(fbm(rng, size, (30, 6), (1.0, 0.4)), *STEEL), colour, bare)
    rust = curve(ImageChops.multiply(px_blur(bare, 4), fbm(rng, size, (20, 6), (1.0, 0.5))), 70, 120)
    colour = Image.composite(tint(fbm(rng, size, (8, 3), (1.0, 0.5)), *RUST), colour, rust)
    height = Image.new("L", size, KIT_LEVEL)
    for y in beads:
        ridge = Image.new("L", size, 0)
        ImageDraw.Draw(ridge).line([(0, y), (size[0], y)], fill=255, width=7)
        height = ImageChops.add(height, scale(px_blur(ridge, 2.5), 60 / 255.0))
    height = ImageChops.subtract(height, scale(bare, 10 / 255.0))
    return (colour, height) + kit_shine(size, "enamel", (bare, "steel"), (rust, "rust"))


def kit_tinplate(rng, size, grime_rows=0):
    """Bare tinplate, scratched, dirtier towards its last `grime_rows` rows."""
    colour = tint(fbm(rng, size, (70, 20, 5), (1.0, 0.5, 0.3)), *TINPLATE)
    scr = kit_scratches(rng, size, max(12, size[0] * size[1] // 2500), (6, 40), 1)
    colour = Image.composite(Image.new("RGB", size, TINPLATE[1]), colour, scale(scr, 0.6))
    grime = Image.new("L", size, 0)
    if grime_rows:
        draw = ImageDraw.Draw(grime)
        h = size[1]
        for y in range(h):
            v = max(0.0, 1.0 - (h - 1 - y) / float(grime_rows))
            draw.line([(0, y), (size[0], y)], fill=int(200 * v))
        grime = ImageChops.multiply(grime, fbm(rng, size, (20, 6), (1.0, 0.5)))
        colour = Image.composite(Image.new("RGB", size, GRIME), colour, grime)
    height = ImageChops.subtract(Image.new("L", size, KIT_LEVEL), scale(scr, 8 / 255.0))
    return (colour, height) + kit_shine(size, "tin", (curve(grime, 60, 140), "grime"))


def kit_floor(rng, size, stains):
    """The floor inside the tin: tinplate gone dull, grime where it meets the wall, rust
    specks, and what was spilt there: `stains` = (fx, fy, radius_px, colour, stuff)."""
    colour, height, strong, tight = kit_tinplate(rng, size)
    w, h = size
    disc = Image.new("L", size, 0)
    ImageDraw.Draw(disc).ellipse([4, 4, w - 5, h - 5], fill=255)
    edge = ImageChops.subtract(disc, px_blur(disc, 18))
    edge = ImageChops.lighter(edge, ImageChops.invert(disc))
    grime = curve(mix((edge, 0.9), (fbm(rng, size, (40, 10), (1.0, 0.5)), 0.5)), 110, 200)
    colour = Image.composite(Image.new("RGB", size, GRIME), colour, grime)
    rust = curve(fbm(rng, size, (14, 5, 2), (1.0, 0.6, 0.4)), 205, 235)
    rust = ImageChops.lighter(rust, curve(ImageChops.multiply(grime, fbm(rng, size, (10, 4), (1.0, 0.5))), 120, 170))
    colour = Image.composite(tint(fbm(rng, size, (6, 2), (1.0, 0.5)), *RUST), colour, rust)
    marks = [(grime, "grime"), (rust, "rust")]
    for fx, fy, r, tone, stuff in stains:
        spot = blob(rng, size, fx * w, fy * h, r)
        colour = Image.composite(Image.new("RGB", size, tone), colour, spot)
        marks.append((curve(spot, 100, 160), stuff))
    return (colour, height) + kit_shine(size, "tin", *marks)


def kit_glue(rng, size):
    """The glue tube, opened out: yellow print on aluminium, a pale panel with КЛЕЙ (glue)
    running along the tube, creased where it was squeezed. The flattened end is at the top
    of the picture; round the tube runs along x, its middle the panel's."""
    w, h = size
    colour = tint(fbm(rng, size, (40, 10), (1.0, 0.5)), (186, 140, 30), GLUE_PRINT)
    bare = Image.new("L", size, 0)
    draw = ImageDraw.Draw(bare)
    draw.rectangle([0, 0, w, 14], fill=255)
    draw.rectangle([0, h - 12, w, h], fill=255)
    panel = Image.new("L", size, 0)
    ImageDraw.Draw(panel).rectangle([int(w * 0.28), int(h * 0.12), int(w * 0.72), int(h * 0.90)], fill=255)
    colour = Image.composite(Image.new("RGB", size, (226, 214, 182)), colour, panel)
    words = kit_text(size, "КЛЕЙ", (int(h * 0.66), int(w * 0.34)), turn=-90, weight=700)
    colour = Image.composite(Image.new("RGB", size, GLUE_INK), colour, words)
    small = Image.new("L", size, 0)
    draw = ImageDraw.Draw(small)
    for i in range(4):
        x = int(w * 0.64) + 0 * i
        y0 = int(h * 0.18) + i * 4
        draw.line([(x - 3 - i * 6, y0), (x - 3 - i * 6, int(h * 0.84))], fill=200, width=2)
    colour = Image.composite(Image.new("RGB", size, (150, 120, 60)), colour, scale(small, 0.7))
    creases = kit_scratches(rng, size, 26, (w * 0.3, w * 0.9), 2, angle=0.0, spread=25.0)
    bare = ImageChops.lighter(bare, curve(ImageChops.multiply(creases, fbm(rng, size, (12, 4), (1.0, 0.5))), 90, 150))
    colour = Image.composite(tint(fbm(rng, size, (20, 5), (1.0, 0.5)), *ALU), colour, bare)
    colour = Image.composite(Image.new("RGB", size, (70, 56, 30)), colour, scale(px_blur(creases, 1.2), 0.35))
    height = ImageChops.subtract(Image.new("L", size, KIT_LEVEL), scale(px_blur(creases, 1.5), 40 / 255.0))
    return (colour, height) + kit_shine(size, "print", (bare, "alu"), (panel, "paper"))


def kit_crimp(rng, size):
    """The tube's flattened end: aluminium pressed into ridges across it."""
    colour = tint(fbm(rng, size, (24, 6), (1.0, 0.5)), *ALU)
    ridges = Image.new("L", size, 0)
    draw = ImageDraw.Draw(ridges)
    for y in range(size[1] // 3, size[1], 10):
        draw.line([(0, y), (size[0], y)], fill=255, width=3)
    colour = Image.composite(Image.new("RGB", size, ALU[0]), colour, scale(ridges, 0.5))
    stripe = Image.new("L", size, 0)
    ImageDraw.Draw(stripe).rectangle([0, 0, size[0], size[1] // 4], fill=255)
    colour = Image.composite(Image.new("RGB", size, GLUE_PRINT), colour, stripe)
    height = ImageChops.add(Image.new("L", size, KIT_LEVEL), scale(px_blur(ridges, 1.5), 50 / 255.0))
    return (colour, height) + kit_shine(size, "alu", (stripe, "print"))


def kit_ribbed(rng, size, tone, stuff, ribs, glass_rows=0):
    """A cap: plastic with ribs round it, and, under `glass_rows`, the bottle's neck."""
    colour = tint(fbm(rng, size, (16, 4), (1.0, 0.5)), tuple(max(0, c - 18) for c in tone), tone)
    lines = Image.new("L", size, 0)
    draw = ImageDraw.Draw(lines)
    step = size[0] / float(ribs)
    for i in range(ribs):
        x = int(i * step)
        draw.line([(x, 0), (x, size[1])], fill=255, width=2)
    colour = Image.composite(Image.new("RGB", size, tuple(min(255, c + 40) for c in tone)), colour, scale(lines, 0.5))
    height = ImageChops.add(Image.new("L", size, KIT_LEVEL), scale(px_blur(lines, 1.2), 40 / 255.0))
    glass = Image.new("L", size, 0)
    if glass_rows:
        ImageDraw.Draw(glass).rectangle([0, size[1] - glass_rows, size[0], size[1]], fill=255)
        colour = Image.composite(Image.new("RGB", size, GLASS), colour, glass)
        height = Image.composite(Image.new("L", size, KIT_LEVEL), height, glass)
    return (colour, height) + kit_shine(size, stuff, (glass, "glass"))


def kit_ink(rng, size):
    """The ink bottle, opened out: dark glass full of ink, the shine of it in two soft
    stripes, and a paper label with ТУШ (Indian ink) on the side that faces out. Round the
    bottle runs along x, the label in its middle; the foot is at the bottom."""
    w, h = size
    colour = Image.new("RGB", size, GLASS)
    shine = Image.new("L", size, 0)
    draw = ImageDraw.Draw(shine)
    for fx in (0.36, 0.40, 0.84):
        draw.line([(int(w * fx), 0), (int(w * fx), h)], fill=255, width=4)
    shine = px_blur(shine, 3)
    colour = Image.composite(Image.new("RGB", size, (120, 126, 136)), colour, scale(shine, 0.55))
    label = Image.new("L", size, 0)
    ImageDraw.Draw(label).rectangle([int(w * 0.30), int(h * 0.16), int(w * 0.70), int(h * 0.80)], fill=255)
    paper = tint(fbm(rng, size, (20, 6), (1.0, 0.5)), (186, 170, 132), PAPER_OLD)
    colour = Image.composite(paper, colour, label)
    words = kit_text((w, int(h * 0.66)), "ТУШ", (int(w * 0.27), int(h * 0.34)), weight=700)
    word_layer = Image.new("L", size, 0)
    word_layer.paste(words, (0, int(h * 0.14)))
    colour = Image.composite(Image.new("RGB", size, INK), colour, word_layer)
    line = Image.new("L", size, 0)
    ImageDraw.Draw(line).line([(int(w * 0.36), int(h * 0.70)), (int(w * 0.64), int(h * 0.70))], fill=255, width=3)
    colour = Image.composite(Image.new("RGB", size, INK), colour, line)
    dribble = blob(rng, size, w * 0.62, h * 0.28, 7, drops=3)
    colour = Image.composite(Image.new("RGB", size, INK), colour, scale(dribble, 0.9))
    height = Image.new("L", size, KIT_LEVEL)
    return (colour, height) + kit_shine(size, "glass", (label, "paper"))


def kit_wood(rng, size):
    """The awl's handle, opened out: pale wood with its grain along the handle, black with
    hands round the middle, the butt end battered."""
    w, h = size
    grain = Image.new("L", size, 0)
    draw = ImageDraw.Draw(grain)
    for _ in range(46):
        x = rng.uniform(0, w)
        wobble = rng.uniform(2, 6)
        pts = [(x + math.sin(y / rng.uniform(14, 30)) * wobble, y) for y in range(0, h + 8, 8)]
        draw.line(pts, fill=int(rng.uniform(90, 255)), width=int(rng.choice((1, 2, 2, 3))))
    base = tint(fbm(rng, size, (40, 10), (1.0, 0.5)), WOOD[0], WOOD[1])
    colour = Image.composite(Image.new("RGB", size, WOOD[0]), base, scale(px_blur(grain, 0.8), 0.7))
    band = Image.new("L", size, 0)
    draw = ImageDraw.Draw(band)
    for y in range(h):
        t = abs(y / float(h) - 0.55) / 0.30
        draw.line([(0, y), (w, y)], fill=int(255 * max(0.0, 1.0 - t)))
    grime = curve(ImageChops.multiply(band, fbm(rng, size, (16, 5), (1.0, 0.6))), 50, 160)
    colour = Image.composite(Image.new("RGB", size, (58, 40, 26)), colour, scale(grime, 0.8))
    height = ImageChops.subtract(Image.new("L", size, KIT_LEVEL), scale(px_blur(grain, 1.0), 14 / 255.0))
    return (colour, height) + kit_shine(size, "wood", (curve(grime, 120, 200), "tape"))


def kit_file(rng, size):
    """The file's cut faces: its teeth cut across it both ways, the handle end knurled."""
    w, h = size
    colour = tint(fbm(rng, size, (30, 6), (1.0, 0.5)), (78, 80, 82), (132, 134, 136))
    teeth = Image.new("L", size, 0)
    draw = ImageDraw.Draw(teeth)
    grip = int(w * 0.30)
    for x in range(grip - h, w + h, 7):
        draw.line([(x, 0), (x + h * 0.6, h)], fill=255, width=2)
    for x in range(grip - h, w + h, 11):
        draw.line([(x + h * 0.4, 0), (x, h)], fill=170, width=2)
    cut = Image.new("L", size, 0)
    ImageDraw.Draw(cut).rectangle([grip, 0, w, h], fill=255)
    teeth = ImageChops.multiply(teeth, cut)
    knurl = Image.new("L", size, 0)
    draw = ImageDraw.Draw(knurl)
    for x in range(-h, grip, 6):
        draw.line([(x, 0), (x + h, h)], fill=255, width=1)
        draw.line([(x + h, 0), (x, h)], fill=255, width=1)
    knurl = ImageChops.multiply(knurl, ImageChops.invert(cut))
    marks = ImageChops.lighter(teeth, knurl)
    colour = Image.composite(Image.new("RGB", size, (172, 174, 176)), colour, scale(marks, 0.55))
    height = ImageChops.add(Image.new("L", size, KIT_LEVEL - 20), scale(px_blur(marks, 0.8), 50 / 255.0))
    return (colour, height) + kit_shine(size, "steel")


def kit_plain(rng, size, tones, stuff, lines=0, angle=0.0):
    """One stuff all over, with `lines` faint strokes `angle` degrees from the x axis."""
    colour = tint(fbm(rng, size, (20, 5), (1.0, 0.5)), *tones)
    height = Image.new("L", size, KIT_LEVEL)
    if lines:
        strokes = kit_scratches(rng, size, lines, (size[0] * 0.2, size[0] * 0.8), 2, angle=angle, spread=4.0)
        colour = Image.composite(Image.new("RGB", size, tones[1]), colour, scale(strokes, 0.4))
        height = ImageChops.add(height, scale(px_blur(strokes, 1.0), 20 / 255.0))
    return (colour, height) + kit_shine(size, stuff)


def kit_sand(rng, size, tones=SAND, grain=0.6, worn_tone=(128, 118, 104), where=(0.55, 0.5)):
    """A scrap of sandpaper: grit of `tones`, rubbed smooth and grey where it did its work."""
    w, h = size
    grit = Image.frombytes("L", size, rng.randbytes(w * h))
    colour = tint(ImageOps.autocontrast(px_blur(grit, grain)), *tones)
    worn = curve(blob(rng, size, w * where[0], h * where[1], w * 0.22, drops=2), 40, 200)
    colour = Image.composite(Image.new("RGB", size, worn_tone), colour, scale(worn, 0.7))
    edge = Image.new("L", size, 255)
    ImageDraw.Draw(edge).rectangle([8, 8, w - 9, h - 9], fill=0)
    colour = Image.composite(Image.new("RGB", size, (200, 186, 150)), colour, px_blur(edge, 2))
    height = ImageChops.add(Image.new("L", size, KIT_LEVEL - 10), scale(px_blur(grit, 0.8), 24 / 255.0))
    return (colour, height) + kit_shine(size, "sand")


def kit_blade(rng, size):
    """A double-edged razor blade, its long side along x: bright steel, the ground edges
    brighter still, and the slot down its middle that shows what it lies on."""
    w, h = size
    colour = tint(fbm(rng, size, (30, 8), (1.0, 0.5)), (150, 152, 156), (196, 198, 202))
    edges = Image.new("L", size, 0)
    draw = ImageDraw.Draw(edges)
    draw.rectangle([0, 0, w, 12], fill=255)
    draw.rectangle([0, h - 13, w, h], fill=255)
    colour = Image.composite(Image.new("RGB", size, (224, 226, 230)), colour, edges)
    slot = Image.new("L", size, 0)
    draw = ImageDraw.Draw(slot)
    cy = h // 2
    draw.rounded_rectangle([int(w * 0.16), cy - 7, int(w * 0.84), cy + 7], radius=7, fill=255)
    for fx in (0.12, 0.88):
        draw.ellipse([int(w * fx) - 13, cy - 13, int(w * fx) + 13, cy + 13], fill=255)
    for fx in (0.36, 0.64):
        draw.rectangle([int(w * fx) - 5, cy - 18, int(w * fx) + 5, cy + 18], fill=255)
    colour = Image.composite(Image.new("RGB", size, (64, 60, 52)), colour, slot)
    spots = curve(fbm(rng, size, (10, 3), (1.0, 0.5)), 215, 240)
    colour = Image.composite(tint(fbm(rng, size, (6, 2), (1.0, 0.5)), *RUST), colour, spots)
    height = Image.composite(Image.new("L", size, KIT_LEVEL - 40), Image.new("L", size, KIT_LEVEL), slot)
    return (colour, height) + kit_shine(size, "blade", (slot, "grime"), (spots, "rust"))


def sharpkit_maps():
    """The colour, relief and shine of the kit, as the three PNGs."""
    atlas = kit_atlas()
    rng = random.Random(1125)
    n = atlas.CANVAS
    colour = Image.new("RGB", (n, n), STEEL[0])
    height = Image.new("L", (n, n), KIT_LEVEL)
    strong = Image.new("L", (n, n), KIT_SHINE["steel"][0])
    tight = Image.new("L", (n, n), KIT_SHINE["steel"][1])

    def size_of(name):
        x0, y0, x1, y1 = atlas.REGIONS[name]
        return (x1 - x0, y1 - y0)

    parts = {
        # Ink spilt by the bottle and glue that ran out of the tube, where sharpkit.py
        # stands them (the floor: +X to the right, +Z down).
        "floor": kit_floor(rng, size_of("floor"), [(0.35, 0.15, 26, INK, "glass"),
                                                   (0.34, 0.80, 16, (150, 112, 40), "print")]),
        "bottom": kit_plain(rng, size_of("bottom"), STEEL, "steel", lines=10, angle=30.0),
        "wall": kit_wall(rng, size_of("wall"), 18, 10, beads=(30, 62)),
        "inner": kit_tinplate(rng, size_of("inner"), grime_rows=28),
        "lid": kit_wall(rng, size_of("lid"), 10, 6),
        "rim": kit_plain(rng, size_of("rim"), STEEL, "steel", lines=30, angle=0.0),
        "glue": kit_glue(rng, size_of("glue")),
        "crimp": kit_crimp(rng, size_of("crimp")),
        "glue_cap": kit_ribbed(rng, size_of("glue_cap"), CAP_RED, "red", ribs=18),
        "ink": kit_ink(rng, size_of("ink")),
        "ink_cap": kit_ribbed(rng, size_of("ink_cap"), BAKELITE, "bakelite", ribs=24, glass_rows=32),
        "wood": kit_wood(rng, size_of("wood")),
        "file": kit_file(rng, size_of("file")),
        "steel": kit_plain(rng, size_of("steel"), STEEL, "steel", lines=40, angle=0.0),
        "tape": kit_plain(rng, size_of("tape"), (TAPE, (52, 52, 56)), "tape", lines=16, angle=70.0),
        "lead": kit_plain(rng, size_of("lead"), LEAD, "lead"),
        "sand": kit_sand(rng, size_of("sand")),
        "sand2": kit_sand(rng, size_of("sand2"), ((34, 34, 38), (90, 90, 96)), 0.35,
                          (112, 110, 106), (0.38, 0.62)),
        "blade": kit_blade(rng, size_of("blade")),
    }
    for name, (c, hgt, s, t) in parts.items():
        x0, y0 = atlas.REGIONS[name][:2]
        colour.paste(c, (x0, y0))
        height.paste(hgt, (x0, y0))
        strong.paste(s, (x0, y0))
        tight.paste(t, (x0, y0))

    out = (KIT_SIZE, KIT_SIZE)
    smdi = (KIT_SMDI_SIZE, KIT_SMDI_SIZE)
    return (colour.resize(out, LANCZOS),
            normal_map(height, out, KIT_RELIEF, KIT_LEVELS_PER_MM, KIT_PX_PER_MM),
            Image.merge("RGB", (Image.new("L", smdi, 255), strong.resize(smdi, BOX), tight.resize(smdi, BOX))))


def draw_place():
    layers = place_layers()
    save(place_colour(layers), "place", "data", "ozg_place_co.png")
    save(normal_map(place_height(layers), (PLACE_SIZE, PLACE_SIZE), PLACE_RELIEF, LEVELS_PER_MM, PX),
         "place", "data", "ozg_place_nohq.png")
    save(place_shine(layers), "place", "data", "ozg_place_smdi.png")


def draw_deck():
    save(deck_relief(), "cards", "data", "ozg_deck_nohq.png")
    save(deck_shine(), "cards", "data", "ozg_deck_smdi.png")


def draw_mark():
    save(flat(16, (128, 128, 255)), "mark", "data", "ozg_mark_nohq.png")
    save(flat(16, (255, 0, 0)), "mark", "data", "ozg_mark_smdi.png")


def draw_card():
    # One card: flat, with the soft sheen of a coated card on both sides.
    save(flat(16, (128, 128, 255)), "cards", "data", "ozg_card_nohq.png")
    save(flat(16, (255, 22, 77)), "cards", "data", "ozg_card_smdi.png")


def draw_sharpkit():
    colour, relief, shine = sharpkit_maps()
    save(colour, "sharpkit", "data", "ozg_sharpkit_co.png")
    save(relief, "sharpkit", "data", "ozg_sharpkit_nohq.png")
    save(shine, "sharpkit", "data", "ozg_sharpkit_smdi.png")


# ------------------------------------------------------- the "3 in 1" board and its pieces
# Boards and dice design §3: the board (one picture for the folded item and the
# laid-out place), the chess pieces in light and dark wood, the checkers, the die.
BOARD_SIZE = 512
BOARD_MM = 320.0                 # the unfolded board: 320 x 320 mm
FIELD_MM = 280.0                 # the 8 x 8 field, 35 mm a square, a border of 20 mm round it
MAPLE = ((206, 172, 120), (232, 204, 156))
WALNUT = ((92, 56, 30), (134, 88, 52))
FRAME = ((58, 34, 20), (94, 60, 36))
BOXWOOD = ((212, 180, 128), (238, 214, 170))
EBONY = ((26, 20, 17), (66, 50, 40))
IVORY = ((214, 204, 180), (238, 232, 214))
JET = ((20, 19, 18), (52, 48, 44))


def wood(rng, size, tones, grain_px=3.0, rings=18):
    """Wood of `tones` with its grain running down the picture."""
    w, h = size
    streaks = Image.new("L", size, 0)
    draw = ImageDraw.Draw(streaks)
    for _ in range(rings * 3):
        x = rng.uniform(0, w)
        wobble = rng.uniform(1.0, 4.0)
        period = rng.uniform(30.0, 90.0)
        pts = [(x + math.sin(y / period + x) * wobble, y) for y in range(0, h + 6, 6)]
        draw.line(pts, fill=int(rng.uniform(60, 200)), width=int(rng.choice((1, 1, 2))))
    streaks = px_blur(streaks, grain_px * 0.4)
    base = fbm(rng, size, (max(8, w // 6), max(4, w // 24)), (1.0, 0.5))
    shade = ImageChops.add(scale(base, 0.75), scale(streaks, 0.45))
    return tint(ImageOps.autocontrast(shade, cutoff=1), tones[0], tones[1]), streaks


def board_picture(rng, n=BOARD_SIZE, turned=False):
    """The board's one picture: the field with its frame and the files and
    ranks; the frame's wood is where the sides and the underside look. `n`
    pixels a side; `turned` writes the files and ranks the way Black reads
    them (the field itself reads the same turned)."""
    px = n / BOARD_MM
    k = n / 512.0
    picture, _ = wood(rng, (n, n), FRAME, 2.0 * k, 10)
    light, _ = wood(rng, (n, n), MAPLE, 2.0 * k, 24)
    dark, _ = wood(rng, (n, n), WALNUT, 2.0 * k, 24)
    edge = int(round((BOARD_MM - FIELD_MM) / 2.0 * px))
    side = int(round(FIELD_MM / 8.0 * px))
    field = Image.new("L", (n, n), 0)
    draw = ImageDraw.Draw(field)
    for rank in range(8):
        for file in range(8):
            if (rank + file) % 2 == 1:      # a1 is dark; h1 light
                x0 = edge + file * side
                y0 = edge + (7 - rank) * side
                draw.rectangle([x0, y0, x0 + side - 1, y0 + side - 1], fill=255)
    squares = Image.new("L", (n, n), 0)
    ImageDraw.Draw(squares).rectangle([edge, edge, edge + 8 * side - 1, edge + 8 * side - 1], fill=255)
    picture = Image.composite(dark, picture, squares)
    picture = Image.composite(light, picture, field)
    # A light line inlaid round the field, and the files and ranks in the frame.
    inlay = Image.new("L", (n, n), 0)
    d = int(round(4 * k))
    ImageDraw.Draw(inlay).rectangle([edge - d, edge - d, edge + 8 * side + d - 1, edge + 8 * side + d - 1],
                                    outline=255, width=max(2, int(round(2 * k))))
    picture = Image.composite(Image.new("RGB", (n, n), MAPLE[1]), picture, inlay)
    font = ImageFont.truetype(KIT_FONT, int(edge * 0.62))
    try:
        font.set_variation_by_axes([500])
    except (OSError, AttributeError):
        pass
    letters = Image.new("L", (n, n), 0)
    draw = ImageDraw.Draw(letters)
    files = "hgfedcba" if turned else "abcdefgh"
    for i in range(8):
        file_letter = files[i]
        rank_number = str(8 - i) if turned else str(i + 1)
        cx = edge + i * side + side / 2.0
        for cy in (edge / 2.0, n - edge / 2.0):
            draw.text((cx, cy), file_letter, font=font, fill=255, anchor="mm")
        cy = edge + (7 - i) * side + side / 2.0
        for cx in (edge / 2.0, n - edge / 2.0):
            draw.text((cx, cy), rank_number, font=font, fill=255, anchor="mm")
    picture = Image.composite(Image.new("RGB", (n, n), MAPLE[1]), picture, letters)
    relief = Image.new("L", (n, n), KIT_LEVEL)
    relief = ImageChops.subtract(relief, scale(px_blur(inlay, 0.6 * k), 30 / 255.0))
    strong, tight = kit_shine((n, n), "bakelite", (letters, "paper"))
    return picture, relief, strong, tight


# The nardy field: two halves of twelve points, the bar between them; the same
# frame as the chess side, so the board's sides look the same either way.
CHERRY = ((120, 52, 34), (162, 82, 56))


def nardy_picture(rng, n=BOARD_SIZE):
    px = n / BOARD_MM
    k = n / 512.0
    picture, _ = wood(rng, (n, n), FRAME, 2.0 * k, 10)
    field, _ = wood(rng, (n, n), MAPLE, 2.0 * k, 24)
    dark, _ = wood(rng, (n, n), WALNUT, 2.0 * k, 24)
    red, _ = wood(rng, (n, n), CHERRY, 2.0 * k, 24)
    edge = int(round((BOARD_MM - FIELD_MM) / 2.0 * px))
    inner = n - 2 * edge
    bar = int(round(16 * px))
    inside = Image.new("L", (n, n), 0)
    ImageDraw.Draw(inside).rectangle([edge, edge, n - edge - 1, n - edge - 1], fill=255)
    picture = Image.composite(field, picture, inside)
    half = (inner - bar) / 2.0
    point_w = half / 6.0
    point_h = inner * 0.42
    darks = Image.new("L", (n, n), 0)
    reds = Image.new("L", (n, n), 0)
    for side in range(2):
        x0 = edge + side * (half + bar)
        for i in range(6):
            left = x0 + i * point_w
            for top in (True, False):
                if top:
                    tri = [(left, edge), (left + point_w, edge), (left + point_w / 2.0, edge + point_h)]
                else:
                    tri = [(left, n - edge), (left + point_w, n - edge), (left + point_w / 2.0, n - edge - point_h)]
                mask = darks if (i + (0 if top else 1)) % 2 == 0 else reds
                ImageDraw.Draw(mask).polygon(tri, fill=255)
    picture = Image.composite(dark, picture, darks)
    picture = Image.composite(red, picture, reds)
    bar_mask = Image.new("L", (n, n), 0)
    bx = edge + int(round(half))
    ImageDraw.Draw(bar_mask).rectangle([bx, edge, bx + bar - 1, n - edge - 1], fill=255)
    frame_wood, _ = wood(rng, (n, n), FRAME, 2.0 * k, 10)
    picture = Image.composite(frame_wood, picture, bar_mask)
    inlay = Image.new("L", (n, n), 0)
    d = int(round(4 * k))
    ImageDraw.Draw(inlay).rectangle([edge - d, edge - d, n - edge + d - 1, n - edge + d - 1],
                                    outline=255, width=max(2, int(round(2 * k))))
    picture = Image.composite(Image.new("RGB", (n, n), MAPLE[1]), picture, inlay)
    return picture


def piece_wood(rng, tones):
    picture, streaks = wood(rng, (256, 256), tones, 2.5, 20)
    relief = ImageChops.subtract(Image.new("L", (256, 256), KIT_LEVEL), scale(streaks, 10 / 255.0))
    return picture, relief


def disc_picture(rng, tones):
    """A checker: the top face in the middle with its turned rings, the rest the
    colour of its edge."""
    n = 256
    base = tint(fbm(rng, (n, n), (40, 10), (1.0, 0.5)), tones[0], tones[1])
    rings = Image.new("L", (n, n), 0)
    draw = ImageDraw.Draw(rings)
    for r in (110, 96, 70, 44, 20):
        draw.ellipse([n / 2 - r, n / 2 - r, n / 2 + r, n / 2 + r], outline=255, width=3)
    lighter = tuple(min(255, c + 26) for c in tones[1])
    darker = tuple(max(0, c - 26) for c in tones[0])
    shadow = ImageChops.offset(rings, 1, 1)
    base = Image.composite(Image.new("RGB", (n, n), darker), base, scale(shadow, 0.8))
    base = Image.composite(Image.new("RGB", (n, n), lighter), base, scale(rings, 0.8))
    relief = ImageChops.add(Image.new("L", (n, n), KIT_LEVEL), scale(px_blur(rings, 1.0), 40 / 255.0))
    return base, relief


# A die's six faces on a picture of 256 x 128: four across, two down, 64 px a
# face; the seventh and eighth cells are the plain bone of the rounded edges.
DIE_CELLS = {1: (0, 0), 2: (1, 0), 3: (2, 0), 4: (3, 0), 5: (0, 1), 6: (1, 1)}
DIE_PIPS = {1: [(0.5, 0.5)], 2: [(0.25, 0.25), (0.75, 0.75)], 3: [(0.25, 0.25), (0.5, 0.5), (0.75, 0.75)],
            4: [(0.25, 0.25), (0.75, 0.25), (0.25, 0.75), (0.75, 0.75)],
            5: [(0.25, 0.25), (0.75, 0.25), (0.5, 0.5), (0.25, 0.75), (0.75, 0.75)],
            6: [(0.25, 0.22), (0.75, 0.22), (0.25, 0.5), (0.75, 0.5), (0.25, 0.78), (0.75, 0.78)]}


def die_picture(rng):
    w, h = 256, 128
    bone = tint(fbm(rng, (w, h), (24, 6), (1.0, 0.5)), (222, 214, 192), (242, 238, 224))
    pips = Image.new("L", (w, h), 0)
    draw = ImageDraw.Draw(pips)
    for face, (cx, cy) in DIE_CELLS.items():
        for fx, fy in DIE_PIPS[face]:
            x = cx * 64 + fx * 64
            y = cy * 64 + fy * 64
            r = 6.5 if face != 1 else 9.0
            draw.ellipse([x - r, y - r, x + r, y + r], fill=255)
    bone = Image.composite(Image.new("RGB", (w, h), (24, 22, 20)), bone, pips)
    relief = ImageChops.subtract(Image.new("L", (w, h), KIT_LEVEL), scale(px_blur(pips, 1.0), 60 / 255.0))
    return bone, relief


def draw_board():
    rng = random.Random(1126)
    picture, relief, strong, tight = board_picture(rng)
    n = BOARD_SIZE
    save(picture, "board", "data", "ozg_board_co.png")
    save(normal_map(relief, (n, n), 1.0, KIT_LEVELS_PER_MM, n / BOARD_MM), "board", "data", "ozg_board_nohq.png")
    out = (256, 256)
    save(Image.merge("RGB", (Image.new("L", out, 255), strong.resize(out, BOX), tight.resize(out, BOX))),
         "board", "data", "ozg_board_smdi.png")
    save(nardy_picture(random.Random(1130)), "board", "data", "ozg_board_nardy_co.png")
    # The table window's pictures of the field (boards and dice design §7): the same
    # wood twice the size, the files and ranks as White reads them and as Black does.
    for side, turned in (("white", False), ("black", True)):
        big = board_picture(random.Random(1126), 1024, turned)[0]
        save(big, "boards", "ozg_board_win_%s_co.png" % side, root=os.path.join(REPO, "art"))


def draw_chess():
    rng = random.Random(1127)
    relief = None
    for name, tones in (("white", BOXWOOD), ("black", EBONY)):
        picture, relief = piece_wood(rng, tones)
        save(picture, "chess", "data", "ozg_chess_%s_co.png" % name)
    save(normal_map(relief, (256, 256), 0.8, KIT_LEVELS_PER_MM, 6.0), "chess", "data", "ozg_chess_nohq.png")
    save(flat(64, (255, 120, 150)), "chess", "data", "ozg_chess_smdi.png")


def draw_disc():
    rng = random.Random(1128)
    relief = None
    for name, tones in (("white", IVORY), ("black", JET)):
        picture, relief = disc_picture(rng, tones)
        save(picture, "disc", "data", "ozg_disc_%s_co.png" % name)
    save(normal_map(relief, (256, 256), 1.0, KIT_LEVELS_PER_MM, 8.0), "disc", "data", "ozg_disc_nohq.png")
    save(flat(64, (255, 110, 140)), "disc", "data", "ozg_disc_smdi.png")


def draw_die():
    rng = random.Random(1129)
    picture, relief = die_picture(rng)
    save(picture, "die", "data", "ozg_die_co.png")
    save(normal_map(relief, (256, 128), 1.0, KIT_LEVELS_PER_MM, 4.0), "die", "data", "ozg_die_nohq.png")
    save(flat(64, (255, 130, 160)), "die", "data", "ozg_die_smdi.png")


DRAWN = {"place": draw_place, "deck": draw_deck, "mark": draw_mark, "card": draw_card,
         "sharpkit": draw_sharpkit, "board": draw_board, "chess": draw_chess, "disc": draw_disc,
         "die": draw_die}


def main():
    wanted = sys.argv[1:] or list(DRAWN)
    for name in wanted:
        if name not in DRAWN:
            sys.exit("nothing called %r is drawn here; there are %s" % (name, ", ".join(DRAWN)))
    for name in wanted:
        DRAWN[name]()


if __name__ == "__main__":
    main()
