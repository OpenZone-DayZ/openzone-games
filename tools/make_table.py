# Draws the pictures of the game window's two tables, as the design canvas of
# 2026-11-24 showed them (memory of the mock-up: research_notes, table-mock):
#
#   classic  the felt of the mat in the world (its own texture, a finer grain)
#            with round corners; the window's body under it (rim, title band,
#            side panel); white round plates and rims for names and buttons,
#            tinted by the window
#   stalker  a piece of cloth as if cut from a stalker's suit -- olive twill,
#            faded and dirty, seams with double stitching, a pocket, small holes
#            -- with an old loners' patch sewn on the pocket (GSC Game World's,
#            art/table/src/SOURCES.md); a frame of the trade screen's worn
#            steel (tools/steel.py) with some rust, open inside; the side
#            panel's dark plate; a dog tag for the names
#   both     a soft glow behind a card that can be played, white, tinted by
#            the window; the soft shadow under every card
#
# Usage: python -I tools/make_table.py [--paa <ImageToPAA.exe>]
#
# Writes art/table/<stem>.png and converts each into
# OpenZone_Games/gui/textures/<stem>.paa. Every picture is drawn at a scale of
# its own and resized to a texture with sides of powers of two; the widget
# stretches it back to its size in layout units (the size is in the table
# below and in tools/make_layouts.py). Deterministic: fixed seeds.
import math
import os
import random
import subprocess
import sys

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter

sys.dont_write_bytecode = True
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import steel as P  # noqa: E402

ART = os.path.join(REPO, "art", "table")
PAA_OUT = os.path.join(REPO, "OpenZone_Games", "gui", "textures")
MAT = os.path.join(REPO, "models", "place", "data", "ozg_place_co.png")
PATCH = os.path.join(ART, "src", "patch_neutrals.png")
IMAGE_TO_PAA = r"E:\Programs\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"

BICUBIC = Image.Resampling.BICUBIC
LANCZOS = Image.Resampling.LANCZOS

# What each picture is drawn for, in layout units (tools/make_layouts.py
# places them), and the texture it becomes.
FELT = (1100, 776)          # classic: the felt inside the window
CLOTH = (1130, 808)         # stalker: the cloth inside the frame
FRAME = (1150, 868)         # stalker: the table's frame with its title band (+ PAD all round)
SIDE = (394, 868)           # stalker: the side panel (+ PAD all round)
TAG = (190, 46)             # stalker: a name's dog tag
GLOW = (136, 176)           # both: behind a 100 x 140 card, 18 units round it
SHADOW = (116, 156)         # both: under a 100 x 140 card, 8 units round it

# The classic table's round shapes (units, corner radius, rim width): white,
# tinted by the window. A name plate, a button of the moment, a button of the
# right column, the close button.
ROUND = [
    ("ozg_round_plate_ca", (180, 48), 6, 0.0, (512, 128)),
    ("ozg_round_plate_edge_ca", (180, 48), 6, 1.5, (512, 128)),
    ("ozg_round_btn_ca", (120, 40), 4, 0.0, (512, 128)),
    ("ozg_round_btn_edge_ca", (120, 40), 4, 2.0, (512, 128)),
    ("ozg_round_wide_ca", (352, 38), 4, 0.0, (1024, 128)),
    ("ozg_round_close_ca", (32, 28), 4, 0.0, (128, 128)),
]


def noise(rnd, size, grain):
    gw, gh = max(2, size[0] // grain + 1), max(2, size[1] // grain + 1)
    return Image.frombytes("L", (gw, gh), rnd.randbytes(gw * gh)).resize(size, BICUBIC)


# ---------------------------------------------------------------- classic

def felt():
    """The mat's own texture over the whole felt, a finer grain over it."""
    w, h = FELT[0] * 2, FELT[1] * 2
    mat = Image.open(MAT).convert("RGB").resize((w, h), LANCZOS)
    rnd = random.Random(7)
    grain = noise(rnd, (w, h), 2).filter(ImageFilter.GaussianBlur(0.6))
    mat = Image.composite(mat.point(lambda v: min(255, int(v * 1.08))), mat.point(lambda v: int(v * 0.92)), grain)
    return mat


# ---------------------------------------------------------------- stalker: the cloth

S = 2                                        # pixels a unit while drawing the cloth
CW, CH = CLOTH[0] * S, CLOTH[1] * S
BASE = (92, 90, 62)
THREAD = (168, 158, 116)


def tint(img, colour, mask):
    img.paste(Image.new("RGB", img.size, colour), (0, 0), mask)


def twill(size):
    """A tight twill: diagonal ribs three threads wide, a lighter and a darker thread."""
    tile = Image.new("L", (6, 6))
    for y in range(6):
        for x in range(6):
            tile.putpixel((x, y), 168 if (x + y) % 6 < 3 else 88)
    big = Image.new("L", size)
    for y in range(0, size[1], 6):
        for x in range(0, size[0], 6):
            big.paste(tile, (x, y))
    return big.filter(ImageFilter.GaussianBlur(0.7))


def weave(rnd):
    img = Image.new("RGB", (CW, CH), BASE)
    # colour drift: faded, sun-bleached and darker stretches
    tint(img, (118, 112, 80), noise(rnd, (CW, CH), 260).point(lambda v: max(0, v - 150) * 2))
    tint(img, (62, 60, 42), noise(rnd, (CW, CH), 180).point(lambda v: max(0, 110 - v) * 2))
    ribs = twill((CW, CH))
    img = Image.composite(img.point(lambda v: min(255, int(v * 1.16))), img.point(lambda v: int(v * 0.84)), ribs)
    fibre = noise(rnd, (CW, CH), 2).filter(ImageFilter.GaussianBlur(0.5))
    return Image.composite(img.point(lambda v: min(255, int(v * 1.06))), img.point(lambda v: int(v * 0.95)), fibre)


def stitch_line(d, pts, colour, dash=14, gap=9, width=3):
    """Stitches along a polyline, `dash` long and `gap` apart."""
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        ln = math.hypot(x1 - x0, y1 - y0)
        ux, uy = (x1 - x0) / ln, (y1 - y0) / ln
        t = 0.0
        while t < ln:
            e = min(ln, t + dash)
            d.line((x0 + ux * t, y0 + uy * t, x0 + ux * e, y0 + uy * e), fill=colour, width=width)
            t = e + gap


def seam(img, rnd, y, wobble=8):
    """A seam across the cloth: a fold of shadow and light, two rows of stitching."""
    pts = [(x, y + wobble * math.sin(x / 260.0 + rnd.random())) for x in range(-20, CW + 40, 40)]
    shade = Image.new("L", (CW, CH), 0)
    ImageDraw.Draw(shade).line(pts, fill=150, width=14)
    img.paste(Image.new("RGB", (CW, CH), (34, 32, 22)), (0, 0), shade.filter(ImageFilter.GaussianBlur(5)))
    light = Image.new("L", (CW, CH), 0)
    ImageDraw.Draw(light).line([(x, yy - 9) for x, yy in pts], fill=90, width=4)
    img.paste(Image.new("RGB", (CW, CH), (140, 134, 96)), (0, 0), light.filter(ImageFilter.GaussianBlur(2)))
    d = ImageDraw.Draw(img)
    stitch_line(d, [(x, yy - 20) for x, yy in pts], THREAD)
    stitch_line(d, [(x, yy + 20) for x, yy in pts], THREAD)


def pocket(img, x0, y0, x1, y1):
    """A patch pocket with a flap, stitched round, its edges a little darker and worn."""
    over = Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    ImageDraw.Draw(over).rectangle((x0 + 6, y0 + 8, x1 + 6, y1 + 8), fill=(0, 0, 0, 90))
    over = over.filter(ImageFilter.GaussianBlur(6))
    img.paste(over, (0, 0), over)
    d = ImageDraw.Draw(img)
    d.rectangle((x0, y0, x1, y1), outline=(48, 46, 32), width=4)
    stitch_line(d, [(x0 + 14, y0 + 14), (x1 - 14, y0 + 14), (x1 - 14, y1 - 14), (x0 + 14, y1 - 14), (x0 + 14, y0 + 14)],
                THREAD, 12, 8, 3)
    fy = y0 + (y1 - y0) * 0.32                                  # the flap
    d.line((x0, fy, x1, fy), fill=(40, 38, 26), width=6)
    d.line((x0, fy - 5, x1, fy - 5), fill=(126, 120, 86), width=2)
    stitch_line(d, [(x0 + 14, fy - 18), (x1 - 14, fy - 18)], THREAD, 12, 8, 3)
    cx = (x0 + x1) / 2
    d.rounded_rectangle((cx - 60, fy - 52, cx + 60, fy - 26), radius=6, fill=(70, 68, 48), outline=(54, 52, 36), width=2)


def hole(img, rnd, cx, cy, r):
    """A small tear: a dark hole, frayed threads round it."""
    pts = []
    for k in range(14):
        a = k / 14 * math.tau
        rr = r * rnd.uniform(0.6, 1.15)
        pts.append((cx + math.cos(a) * rr * 1.3, cy + math.sin(a) * rr))
    over = Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    od = ImageDraw.Draw(over)
    od.polygon([(x + 2, y + 3) for x, y in pts], fill=(10, 9, 6, 255))
    for _ in range(26):
        a = rnd.uniform(0, math.tau)
        x, y = cx + math.cos(a) * r * 1.2, cy + math.sin(a) * r * 0.95
        ln = rnd.uniform(8, 26)
        b = a + rnd.uniform(-0.6, 0.6)
        od.line((x, y, x + math.cos(b) * ln, y + math.sin(b) * ln), fill=THREAD + (220,), width=2)
    soft = over.filter(ImageFilter.GaussianBlur(0.6))
    img.paste(soft, (0, 0), soft)


def stains(img, rnd):
    """Oil, rust, dried salt, mud: soft blotches and rings."""
    over = Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    od = ImageDraw.Draw(over)
    for colour, n in (((30, 26, 18, 70), 4), ((90, 52, 24, 60), 2), ((70, 62, 44, 70), 4)):
        for _ in range(n):
            cx, cy = rnd.uniform(0, CW), rnd.uniform(0, CH)
            for _ in range(5):
                rx, ry = rnd.uniform(20, 90), rnd.uniform(14, 60)
                ox, oy = rnd.uniform(-40, 40), rnd.uniform(-30, 30)
                od.ellipse((cx + ox - rx, cy + oy - ry, cx + ox + rx, cy + oy + ry), fill=colour)
    for _ in range(3):                                           # dried salt rings
        cx, cy, r = rnd.uniform(200, CW - 200), rnd.uniform(200, CH - 200), rnd.uniform(60, 120)
        od.ellipse((cx - r, cy - r * 0.8, cx + r, cy + r * 0.8), outline=(190, 184, 150, 70), width=5)
    over = over.filter(ImageFilter.GaussianBlur(14))
    img.paste(over, (0, 0), over)


def patch(img, rnd, cx, cy, size, angle):
    """The loners' patch, old: its black ground cut away, colours faded, dirt over it,
    a corner rubbed thin, sewn on with a shadow under it."""
    src = Image.open(PATCH).convert("RGB")
    mask = src.convert("L").point(lambda v: 255 if v > 26 else 0)
    mask = mask.filter(ImageFilter.MaxFilter(3)).filter(ImageFilter.MinFilter(5)).filter(ImageFilter.GaussianBlur(1.2))
    faded = ImageEnhance.Brightness(ImageEnhance.Color(src).enhance(0.75)).enhance(0.86)
    dirt = noise(rnd, src.size, 60).filter(ImageFilter.GaussianBlur(10))
    faded = Image.composite(faded.point(lambda v: int(v * 0.62)), faded, dirt.point(lambda v: max(0, v - 120) * 2))
    rub = Image.new("L", src.size, 0)
    ImageDraw.Draw(rub).ellipse((src.width * 0.72, src.height * 0.70, src.width * 1.05, src.height * 1.02), fill=160)
    faded = Image.composite(faded.point(lambda v: min(255, int(v * 0.8 + 30))), faded, rub.filter(ImageFilter.GaussianBlur(40)))
    piece = faded.convert("RGBA")
    piece.putalpha(mask)
    scale = size / max(src.size)
    piece = piece.resize((int(src.width * scale), int(src.height * scale)), LANCZOS)
    piece = piece.rotate(angle, resample=BICUBIC, expand=True)
    shadow = Image.new("RGBA", piece.size, (0, 0, 0, 0))
    shadow.putalpha(piece.split()[3].point(lambda v: v * 150 // 255))
    shadow = shadow.filter(ImageFilter.GaussianBlur(8))
    x, y = int(cx - piece.width / 2), int(cy - piece.height / 2)
    out = img.convert("RGBA")
    out.alpha_composite(shadow, (x + 6, y + 9))
    out.alpha_composite(piece, (x, y))
    return out.convert("RGB")


def lamp(img):
    """Light from a lamp over the middle of the table, the corners in shadow."""
    light = Image.new("L", (CW, CH), 0)
    ImageDraw.Draw(light).ellipse((CW * 0.08, -CH * 0.05, CW * 0.92, CH * 1.08), fill=255)
    return Image.composite(img, img.point(lambda v: int(v * 0.5)), light.filter(ImageFilter.GaussianBlur(200)))


def cloth():
    rnd = random.Random(29)
    img = weave(rnd)
    stains(img, rnd)
    seam(img, rnd, int(CH * 0.30))
    pocket(img, int(CW * 0.70), int(CH * 0.70), int(CW * 0.97), int(CH * 1.05))
    hole(img, rnd, int(CW * 0.23), int(CH * 0.86), 11)
    hole(img, rnd, int(CW * 0.58), int(CH * 0.13), 8)
    # On the pocket, below its flap: no fan covers it, whatever the number of players.
    img = patch(img, rnd, int(CW * 0.835), int(CH * 0.90), 300, -3)
    return lamp(img)


# ---------------------------------------------------------------- stalker: steel

def poly_mask(size, pts):
    m = Image.new("L", size, 0)
    ImageDraw.Draw(m).polygon(P.to_px(pts), fill=255)
    return m


def rust(rnd, art, mask, amount=1.0):
    """Rust in blotches over the steel: orange-brown where the noise is high, darker at
    its heart. Only where `mask` is."""
    W, H = art.size
    seeded = random.Random(rnd.random())
    gw, gh = max(2, W // 90), max(2, H // 90)
    lo = Image.frombytes("L", (gw, gh), seeded.randbytes(gw * gh)).resize((W, H), BICUBIC)
    lo = lo.filter(ImageFilter.GaussianBlur(12))
    fine = Image.effect_noise((W, H), 60).convert("L").filter(ImageFilter.GaussianBlur(0.8))
    spots = ImageChops.multiply(lo.point(lambda v: max(0, (v - 168) * 5)), fine.point(lambda v: min(255, v + 70)))
    spots = ImageChops.multiply(spots, mask).point(lambda v: int(v * 0.42 * amount))
    rust_light = Image.new("RGBA", (W, H), (132, 68, 30, 255))
    rust_dark = Image.new("RGBA", (W, H), (82, 40, 18, 255))
    body = Image.composite(rust_dark, rust_light, lo.point(lambda v: max(0, (v - 205) * 6)))
    body.putalpha(spots)
    art.alpha_composite(body)


def runs(rnd, art, x, y, w, count):
    """Rust run down from a seam: thin, fading streaks."""
    layer = Image.new("RGBA", art.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    for _ in range(count):
        sx = rnd.uniform(x + 20, x + w - 20) * P.SCALE
        sy = y * P.SCALE
        ln = rnd.uniform(4, 16) * P.SCALE
        for k in range(int(ln)):
            d.point((sx + rnd.uniform(-0.6, 0.6), sy + k), fill=(120, 58, 24, int(110 * (1 - k / ln))))
    art.alpha_composite(layer.filter(ImageFilter.GaussianBlur(0.8)))


def table_frame():
    """The table's panel: the title band on top, then a frame open inside, through
    which the cloth shows."""
    w, h = FRAME
    rnd = random.Random(301)
    W, H = int((w + 2 * P.PAD) * P.SCALE), int((h + 2 * P.PAD) * P.SCALE)
    art = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    P.head_band(rnd, art, P.PAD, P.PAD, w)
    rust(rnd, art, poly_mask((W, H), P.outline(random.Random(302), P.PAD, P.PAD, w, P.HEAD_H)), 0.9)
    top = P.PAD + P.HEAD_H + P.HEAD_GAP
    box_h = P.PAD + h - top
    band = P.BAND + 2
    outer = P.outline(rnd, P.PAD, top, w, box_h)
    inner = P.outline(rnd, P.PAD + band, top + band, w - 2 * band, box_h - 2 * band)
    ring = ImageChops.subtract(poly_mask((W, H), outer), poly_mask((W, H), inner))
    art.paste(P.steel(rnd, (W, H), P.FRAME, 4, 10), (0, 0), ring)
    d = ImageDraw.Draw(art, "RGBA")
    P.bevel(d, outer, True, P.EDGE_LIGHT, P.EDGE_DARK, 2)
    P.bevel(d, inner, False, (60, 57, 53, 140), (0, 0, 0, 220), 2)
    d.line(P.to_px(inner + inner[:1]), fill=(0, 0, 0, 200), width=P.SCALE)
    rust(rnd, art, ring, 1.4)
    P.wear(rnd, art, outer, ring, P.PAD, top, w, box_h, band)
    runs(rnd, art, P.PAD, top + band, w, 14)
    # a soft shadow the frame throws onto the cloth inside it
    shade = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(shade).line(P.to_px(inner + inner[:1]), fill=(0, 0, 0, 150), width=P.SCALE * 6)
    shade = shade.filter(ImageFilter.GaussianBlur(P.SCALE * 4))
    shade.putalpha(ImageChops.multiply(shade.split()[3], poly_mask((W, H), inner)))
    art.alpha_composite(shade)
    return art.filter(ImageFilter.GaussianBlur(radius=0.35))


def side_panel():
    """The side panel: the trader's dark plate with its title band, and some rust."""
    w, h = SIDE
    rnd = random.Random(409)
    W, H = int((w + 2 * P.PAD) * P.SCALE), int((h + 2 * P.PAD) * P.SCALE)
    art = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    P.head_band(rnd, art, P.PAD, P.PAD, w)
    top = P.PAD + P.HEAD_H + P.HEAD_GAP
    box_h = P.PAD + h - top
    outer, frame_mask = P.framed_box(rnd, art, P.PAD, top, w, box_h, P.BAND)
    inner = P.outline(random.Random(414), P.PAD + P.BAND, top + P.BAND, w - 2 * P.BAND, box_h - 2 * P.BAND)
    rust(rnd, art, ImageChops.subtract(frame_mask, poly_mask((W, H), inner)), 1.3)
    P.wear(rnd, art, outer, frame_mask, P.PAD, top, w, box_h, P.BAND)
    return art.filter(ImageFilter.GaussianBlur(radius=0.35))


def tag():
    """A dog tag for a name: a rounded plate of dull steel, lighter at the top, a
    hole near the left end with a dark ring, scratches. Its edge is drawn apart
    (tag_edge), so that the window can tint it gold for the seat whose move it is."""
    k = 4
    w, h = TAG[0] * k, TAG[1] * k
    rnd = random.Random(53)
    art = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    body = Image.new("L", (w, h), 0)
    ImageDraw.Draw(body).rounded_rectangle((2 * k, 2 * k, w - 2 * k, h - 2 * k), radius=16 * k, fill=255)
    top, bottom = (78, 73, 66), (40, 37, 33)
    grad = Image.new("RGB", (w, h))
    gd = ImageDraw.Draw(grad)
    for y in range(h):
        t = y / (h - 1)
        gd.line((0, y, w, y), fill=tuple(int(top[i] + (bottom[i] - top[i]) * t) for i in range(3)))
    grain = Image.effect_noise((w, h), 18).convert("L")
    grad = Image.composite(grad.point(lambda v: min(255, int(v * 1.06))), grad.point(lambda v: int(v * 0.94)), grain)
    plate = grad.convert("RGBA")
    plate.putalpha(body)
    art.alpha_composite(plate)
    d = ImageDraw.Draw(art, "RGBA")
    d.rounded_rectangle((4 * k, 3 * k, w - 4 * k, h // 2), radius=14 * k, outline=(150, 144, 132, 70), width=k)
    for _ in range(40):
        sx, sy = rnd.uniform(18 * k, w - 8 * k), rnd.uniform(6 * k, h - 6 * k)
        ln = rnd.uniform(3, 14) * k
        a = rnd.uniform(-0.4, 0.4)
        d.line((sx, sy, sx + ln * math.cos(a), sy + ln * math.sin(a)), fill=(130, 124, 112, rnd.randint(20, 70)), width=1)
    cx, cy, r = 15 * k, h / 2, 5 * k
    d.ellipse((cx - r - k, cy - r - k, cx + r + k, cy + r + k), fill=(110, 104, 94, 255))
    d.ellipse((cx - r, cy - r, cx + r, cy + r), fill=(0, 0, 0, 0))
    hole_mask = Image.new("L", (w, h), 255)
    ImageDraw.Draw(hole_mask).ellipse((cx - r, cy - r, cx + r, cy + r), fill=0)
    art.putalpha(ImageChops.multiply(art.split()[3], hole_mask))
    return art


def tag_edge():
    """The dog tag's edge alone, white: tinted by the window."""
    k = 4
    w, h = TAG[0] * k, TAG[1] * k
    art = Image.new("RGBA", (w, h), (255, 255, 255, 0))
    d = ImageDraw.Draw(art)
    d.rounded_rectangle((2 * k, 2 * k, w - 2 * k, h - 2 * k), radius=16 * k, outline=(255, 255, 255, 255), width=int(1.6 * k))
    return art.filter(ImageFilter.GaussianBlur(0.6))


def glow():
    """A soft light round a card, white: the window tints it (gold for a card that
    can be played)."""
    k = 2
    w, h = GLOW[0] * k, GLOW[1] * k
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle((14 * k, 14 * k, w - 14 * k, h - 14 * k), radius=10 * k, fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(7 * k))
    art = Image.new("RGBA", (w, h), (255, 255, 255, 0))
    art.putalpha(mask.point(lambda v: min(255, int(v * 1.25))))
    return art


def shadow():
    """The shadow a card throws on the cloth, as the mock-up's drop-shadow (3 units
    down, a blur of 4): black, its strength the widget's alpha. Drawn for a
    100 x 140 card with 8 units round it."""
    k = 2
    w, h = SHADOW[0] * k, SHADOW[1] * k
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle((8 * k, 8 * k, w - 8 * k, h - 8 * k), radius=8 * k, fill=255)
    mask = mask.filter(ImageFilter.GaussianBlur(3 * k))
    art = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    art.putalpha(mask)
    return art


def rounded(size, radius, edge=0.0, k=4):
    """A white rounded plate of `size` units (or its rim alone, `edge` units wide),
    tinted by the widget: the classic table's plates and buttons, which the
    mock-up drew with round corners."""
    w, h = int(size[0] * k), int(size[1] * k)
    art = Image.new("RGBA", (w, h), (255, 255, 255, 0))
    d = ImageDraw.Draw(art)
    box = (k // 2, k // 2, w - 1 - k // 2, h - 1 - k // 2)
    if edge > 0:
        d.rounded_rectangle(box, radius=radius * k, outline=(255, 255, 255, 255), width=max(1, int(edge * k)))
    else:
        d.rounded_rectangle(box, radius=radius * k, fill=(255, 255, 255, 255))
    return art.filter(ImageFilter.GaussianBlur(0.4))


def classic_window():
    """The classic window's own body, as the mock-up drew it: a grey-green rim,
    the dark window, its title band and the side panel, all with round corners.
    The felt lies over it, a picture of its own. 1600 x 900 units."""
    k = 2
    art = Image.new("RGBA", (1600 * k, 900 * k), (0, 0, 0, 0))
    d = ImageDraw.Draw(art)

    def box(x, y, w, h):
        return (x * k, y * k, (x + w) * k - 1, (y + h) * k - 1)

    d.rounded_rectangle(box(38, 26, 1524, 848), radius=9 * k, fill=(0x46, 0x54, 0x4c, 255))
    d.rounded_rectangle(box(40, 28, 1520, 844), radius=8 * k, fill=(0x12, 0x16, 0x14, 255))
    d.rounded_rectangle(box(40, 28, 1520, 44), radius=8 * k, fill=(0x0d, 0x1b, 0x16, 255),
                        corners=(True, True, False, False))
    d.rounded_rectangle(box(1164, 84, 384, 776), radius=6 * k, fill=(0x16, 0x1b, 0x19, 255))
    return art


def felt_round(img):
    """The felt with the mock-up's round corners (6 units)."""
    w, h = img.size
    k = w / FELT[0]
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, w - 1, h - 1), radius=int(6 * k), fill=255)
    out = img.convert("RGBA")
    out.putalpha(mask)
    return out


# ---------------------------------------------------------------- out

def save(img, stem, size, paa):
    """`img` resized to `size`, saved as art/table/<stem>.png and, given the converter,
    as OpenZone_Games/gui/textures/<stem>.paa."""
    img = img.resize(size, LANCZOS)
    png = os.path.join(ART, stem + ".png")
    img.save(png)
    line = "%s %dx%d" % (stem, size[0], size[1])
    if paa:
        target = os.path.join(PAA_OUT, stem + ".paa")
        if os.path.exists(target):
            os.remove(target)
        done = subprocess.run([paa, png, target], capture_output=True, text=True, stdin=subprocess.DEVNULL)
        if done.returncode != 0 or not os.path.isfile(target):
            raise SystemExit("ImageToPAA failed for " + stem + ": " + done.stdout + done.stderr)
        line += "  -> %d KB" % (os.path.getsize(target) // 1024)
    print(line)


def main():
    paa = IMAGE_TO_PAA
    if "--paa" in sys.argv:
        paa = sys.argv[sys.argv.index("--paa") + 1]
    if "--no-paa" in sys.argv or not os.path.isfile(paa):
        paa = None
    os.makedirs(ART, exist_ok=True)
    os.makedirs(PAA_OUT, exist_ok=True)
    save(felt_round(felt()), "ozg_table_felt_ca", (2048, 1024), paa)
    save(classic_window(), "ozg_table_window_ca", (2048, 1024), paa)
    save(cloth(), "ozg_table_cloth_co", (2048, 1024), paa)
    save(table_frame(), "ozg_table_frame_ca", (2048, 1024), paa)
    save(side_panel(), "ozg_table_side_ca", (512, 1024), paa)
    save(tag(), "ozg_table_tag_ca", (512, 128), paa)
    save(tag_edge(), "ozg_table_tagedge_ca", (512, 128), paa)
    save(glow(), "ozg_table_glow_ca", (256, 256), paa)
    save(shadow(), "ozg_table_shadow_ca", (256, 256), paa)
    for stem, size, radius, edge, tex in ROUND:
        save(rounded(size, radius, edge), stem, tex, paa)


if __name__ == "__main__":
    main()
