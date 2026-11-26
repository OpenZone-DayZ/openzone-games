"""The dice of three colours and the stalker dice (boards and dice design, the owner's word
33: "dice of three kinds: white, black and red; the stalker variant: small radiation signs
instead of the pips").

    python -I models/src/die_looks.py

Writes models/die/data/ozg_die_<look>_co.png, 256 x 128, in the cells of the die's model
(make_textures.py DIE_CELLS: a face in each 64 px cell, opposite faces adding up to seven;
the last two cells the plain body for the cut edges and corners), and the window's
pictures of each face, art/dice/<look>_<n>.png, 96 px, a die seen square on with round
corners, as the design canvas's dice draw them.

Looks: black, red (dots), stalker, stalker_black, stalker_red (radiation signs). The white
die with dots is the classic die's own picture (make_textures.py die) and its window
pictures are written here too, as "white".
Colours are the canvas's: white #f2ead7 / pips #1d1712, black #23201d / #efe6d2, red
#a3201b / #f6efe2; the radiation sign is the canvas's: three blades 60 degrees wide
between 3.4 and 11.5 units of a 24-unit box, round a hub of 2.2.
"""
import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(HERE)
REPO = os.path.dirname(MODELS)
CELLS = {1: (0, 0), 2: (1, 0), 3: (2, 0), 4: (3, 0), 5: (0, 1), 6: (1, 1)}
PIPS = {1: [(0.5, 0.5)], 2: [(0.25, 0.25), (0.75, 0.75)], 3: [(0.25, 0.25), (0.5, 0.5), (0.75, 0.75)],
        4: [(0.25, 0.25), (0.75, 0.25), (0.25, 0.75), (0.75, 0.75)],
        5: [(0.25, 0.25), (0.75, 0.25), (0.5, 0.5), (0.25, 0.75), (0.75, 0.75)],
        6: [(0.25, 0.22), (0.75, 0.22), (0.25, 0.5), (0.75, 0.5), (0.25, 0.78), (0.75, 0.78)]}
COLOURS = {"white": ((242, 234, 215), (29, 23, 18)), "black": ((35, 32, 29), (239, 230, 210)),
           "red": ((163, 32, 27), (246, 239, 226))}
LOOKS = {"white": ("white", False), "black": ("black", False), "red": ("red", False),
         "stalker": ("white", True), "stalker_black": ("black", True), "stalker_red": ("red", True)}
SS = 4


def trefoil(draw, cx, cy, size, colour):
    """The radiation sign, `size` px across, at (cx, cy)."""
    k = size / 24.0
    r0, r1, hub = 3.4 * k, 11.5 * k, 2.2 * k
    for middle in (-90, 30, 150):
        a0 = math.radians(middle - 30)
        a1 = math.radians(middle + 30)
        pts = []
        steps = 12
        for i in range(steps + 1):
            a = a0 + (a1 - a0) * i / steps
            pts.append((cx + r1 * math.cos(a), cy + r1 * math.sin(a)))
        for i in range(steps, -1, -1):
            a = a0 + (a1 - a0) * i / steps
            pts.append((cx + r0 * math.cos(a), cy + r0 * math.sin(a)))
        draw.polygon(pts, fill=colour)
    draw.ellipse((cx - hub, cy - hub, cx + hub, cy + hub), fill=colour)


def pips(draw, n, x0, y0, cell, colour, rad):
    for fx, fy in PIPS[n]:
        cx = x0 + fx * cell
        cy = y0 + fy * cell
        if rad:
            size = cell * (0.36 if n == 1 else 0.27)
            trefoil(draw, cx, cy, size, colour)
        else:
            r = cell * (0.14 if n == 1 else 0.1)
            draw.ellipse((cx - r, cy - r, cx + r, cy + r), fill=colour)


def grain(img, rng, amount):
    """A little unevenness, as the classic die's bone has."""
    noise = Image.effect_noise(img.size, 30).filter(ImageFilter.GaussianBlur(1.2))
    px = img.load()
    nz = noise.load()
    for y in range(img.size[1]):
        for x in range(img.size[0]):
            d = (nz[x, y] - 128) / 128.0 * amount
            r, g, b = px[x, y]
            px[x, y] = (max(0, min(255, int(r + d))), max(0, min(255, int(g + d))), max(0, min(255, int(b + d))))
    return img


def sheet(look, rng):
    body, pip = COLOURS[LOOKS[look][0]]
    rad = LOOKS[look][1]
    w, h = 256 * SS, 128 * SS
    img = Image.new("RGB", (w, h), body)
    d = ImageDraw.Draw(img)
    for n, (cx, cy) in CELLS.items():
        pips(d, n, cx * 64 * SS, cy * 64 * SS, 64 * SS, pip, rad)
    img = img.resize((256, 128), Image.Resampling.LANCZOS)
    return grain(img, rng, 6)


def face_picture(look, n):
    """The window's picture of a face: the die square on, round corners, a soft edge."""
    body, pip = COLOURS[LOOKS[look][0]]
    rad = LOOKS[look][1]
    size = 96 * SS
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    r = 16 * SS
    d.rounded_rectangle((0, 0, size - 1, size - 1), radius=r, fill=body + (255,))
    # a lighter top and a darker bottom, as a die lit from above
    shade = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shade)
    for y in range(size):
        t = y / float(size)
        a = int(28 * (t - 0.5) * 2) if t > 0.5 else 0
        sd.line((0, y, size, y), fill=(0, 0, 0, max(0, a)))
    img = Image.alpha_composite(img, Image.composite(shade, Image.new("RGBA", (size, size), (0, 0, 0, 0)), img.getchannel("A")))
    d = ImageDraw.Draw(img)
    inset = 8 * SS
    pips(d, n, inset, inset, size - 2 * inset, pip + (255,), rad)
    return img.resize((96, 96), Image.Resampling.LANCZOS)


def main():
    rng = random.Random(1133)
    out = os.path.join(MODELS, "die", "data")
    for look in LOOKS:
        if look != "white":
            sheet(look, rng).save(os.path.join(out, "ozg_die_%s_co.png" % look))
            print("wrote models/die/data/ozg_die_%s_co.png" % look)
    art = os.path.join(REPO, "art", "dice")
    os.makedirs(art, exist_ok=True)
    for look in LOOKS:
        for n in range(1, 7):
            face_picture(look, n).save(os.path.join(art, "%s_%d.png" % (look, n)))
    print("wrote art/dice: %d pictures" % (len(LOOKS) * 6))


if __name__ == "__main__":
    main()
