"""Draw the texture sources of the models. Pillow and the standard library, nothing else.

    python -I models/src/make_textures.py

It writes the PNG sources the shipped .paa are converted from (asset_convert):

    models/place/data/ozg_place_co.png             512  the mat: dark green felt, a lighter sewn
                                                        binding, worn
    models/place/data/ozg_place_nohq.png           512  its relief: the binding, the stitches,
                                                        the folds, the burns
    models/place/data/ozg_place_smdi.png           256  where it shines: thread, rubbed felt, stains
    models/cards/data/ozg_deck_nohq.png            256  the deck sheet's relief: the side of the stack
    models/cards/data/ozg_deck_smdi.png             64  the deck sheet's shine
    models/mark/data/ozg_mark_nohq.png              16  flat
    models/mark/data/ozg_mark_smdi.png              16  no shine

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

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageOps

HERE = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(HERE)

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


def save(img, *parts):
    path = os.path.join(MODELS, *parts)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path, optimize=True)
    print("%-48s %4d x %-4d %s" % ("/".join(("models",) + parts), img.width, img.height, img.mode))


def main():
    layers = place_layers()
    save(place_colour(layers), "place", "data", "ozg_place_co.png")
    save(normal_map(place_height(layers), (PLACE_SIZE, PLACE_SIZE), PLACE_RELIEF, LEVELS_PER_MM, PX),
         "place", "data", "ozg_place_nohq.png")
    save(place_shine(layers), "place", "data", "ozg_place_smdi.png")
    save(deck_relief(), "cards", "data", "ozg_deck_nohq.png")
    save(deck_shine(), "cards", "data", "ozg_deck_smdi.png")
    save(flat(16, (128, 128, 255)), "mark", "data", "ozg_mark_nohq.png")
    save(flat(16, (255, 0, 0)), "mark", "data", "ozg_mark_smdi.png")


if __name__ == "__main__":
    main()
