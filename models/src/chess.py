"""The six chess pieces of the "3 in 1" board, turned in wood (boards and dice design §3.2).

    blender -b -t 4 --python models/src/chess.py [-- --no-preview]

Builds models/src/chess_<k>.blend for k in k q r b n p and renders models/preview/chess_<k>.png.
The models they become are OpenZone_Games\\models\\chess\\ozg_chess_<k>.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. A piece STANDS in its model
as it stands on the board, the middle of its base at the origin; rotationFlags 0 keeps it
so on the ground. One model a kind: the colour is the hidden selection `camo`, light or
dark wood (OZG_Chess_Classic_w*, _b*). The knight looks towards +Z.

Every face takes the whole wood picture: round a turned body it runs along u, up it along
v; the knight's head is laid on from the side.
"""
import math
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import turn_kit as turn  # noqa: E402

TEXTURE = kit.MOD + r"\models\chess\data\ozg_chess_white_co.paa"
MATERIAL = kit.MOD + r"\models\chess\data\ozg_chess.rvmat"
SIDES = 20
MM = 0.001

# The profiles, (height, radius) in millimetres from the base up.
BASE = [(0, 12.5), (1.6, 12.5), (2.6, 11.4), (3.6, 11.4), (4.8, 9.8), (7.5, 8.2)]
PROFILES = {
    "p": [(0, 10.5), (1.5, 10.5), (2.5, 9.6), (3.5, 9.6), (4.5, 8.4), (7, 6.6), (11, 5.2), (17, 4.2),
          (18.5, 6.0), (20, 6.0), (21, 4.2), (22.5, 4.6), (24.5, 6.0), (27, 6.4), (29.5, 6.0), (31.5, 4.8),
          (33, 2.8), (34, 0)],
    "r": BASE + [(12, 7.6), (24, 7.0), (26, 7.8), (28, 9.2), (29, 9.6), (35, 9.6), (35, 6.8), (33, 6.8),
                 (33, 0)],
    "b": BASE + [(12, 6.4), (26, 4.8), (27.5, 7.6), (29, 7.6), (30, 4.6), (31.5, 5.2), (35, 7.6), (39, 8.0),
                 (43, 7.0), (46.5, 4.8), (48.5, 2.6), (49, 1.6), (49.6, 2.2), (50.6, 2.7), (51.6, 2.1),
                 (52, 0)],
    "q": [(0, 13.5), (1.6, 13.5), (2.6, 12.4), (3.6, 12.4), (4.8, 10.6), (7.5, 8.8), (12, 7.0), (34, 5.4),
          (35.5, 8.4), (37, 8.4), (38, 5.6), (41, 6.6), (46, 8.8), (49, 10.0), (50, 10.0), (50, 8.4),
          (52.5, 5.6), (54.5, 3.2), (55, 2.2), (56.2, 3.0), (57.8, 3.0), (59.2, 2.0), (60, 0)],
    "k": [(0, 13.5), (1.6, 13.5), (2.6, 12.4), (3.6, 12.4), (4.8, 10.6), (7.5, 8.8), (12, 7.2), (38, 5.6),
          (39.5, 8.6), (41, 8.6), (42, 5.8), (45, 6.8), (50, 9.2), (53, 10.2), (54, 10.2), (55, 8.2),
          (57.5, 5.6), (59, 4.0), (59, 0)],
    "n": BASE + [(10, 7.8), (12, 7.8), (12, 0)],
}
HEIGHT = {"p": 34, "r": 39, "b": 52, "q": 60, "k": 68, "n": 51}
MASS = {"p": 0.015, "r": 0.025, "b": 0.022, "q": 0.03, "k": 0.035, "n": 0.022}
# The knight's head seen from its right, (z, y) in millimetres, the muzzle towards +Z.
HORSE = [(-8, 12), (7, 12), (9, 16), (7, 20), (6, 23), (11, 25), (16, 27), (17, 31), (14, 33.5),
         (8, 38), (5.5, 42), (4, 47), (2, 51), (0.5, 46.5), (-2, 46), (-4, 43), (-3, 41), (-6, 39),
         (-5, 37), (-8, 34), (-7, 32), (-9.5, 28), (-10, 22), (-9.5, 16), (-8.5, 13)]


def piece(kind):
    mat = kit.material("wood", TEXTURE, MATERIAL,
                       image=kit.source("chess", "data", "ozg_chess_white_co.png"),
                       colour=(0.85, 0.72, 0.5), rough=0.45)
    lod = kit.Lod()
    profile = [(y * MM, r * MM) for y, r in PROFILES[kind]]
    turn.lathe(lod, "body", profile, SIDES, mat)
    h = HEIGHT[kind] * MM
    if kind == "r":
        # Four merlons on the rim.
        for k in range(4):
            a = math.radians(45.0 + 90.0 * k)
            cx, cz = 8.2 * MM * math.sin(a), 8.2 * MM * math.cos(a)
            half = 2.6 * MM
            turn.box(lod, "merlon%d" % k, (cx - half, 35 * MM, cz - half), (cx + half, 39 * MM, cz + half), mat)
    if kind == "q":
        # Eight points round the crown.
        for k in range(8):
            a = 2.0 * math.pi * k / 8.0
            cx, cz = 9.0 * MM * math.sin(a), 9.0 * MM * math.cos(a)
            turn.lathe(lod, "point%d" % k, [(50 * MM, 1.4 * MM), (52.6 * MM, 0.0)], 6, mat, centre=(cx, cz),
                       close_bottom=False)
    if kind == "k":
        # The cross.
        t = 1.7 * MM
        turn.box(lod, "crossv", (-t, 59 * MM, -t), (t, 68 * MM, t), mat)
        turn.box(lod, "crossh", (-4.4 * MM, 62.2 * MM, -t), (4.4 * MM, 65.6 * MM, t), mat)
    if kind == "n":
        outline = [(z * MM, y * MM) for z, y in HORSE]
        turn.slab(lod, "head", outline, -4.6 * MM, 4.6 * MM, mat,
                  uv_of=lambda z, y: (0.5 + z / (40 * MM), y / (50 * MM)),
                  side_uv=(0.0, 0.2, 1.0, 0.35))
    lod.select("camo")
    lod.finish("ozg_chess_%s 1" % kind, kit.VISUAL, 1)
    return h


def main():
    for kind in ("k", "q", "r", "b", "n", "p"):
        kit.start()
        h = piece(kind)
        r = PROFILES[kind][0][1] * MM
        lo = (-r, 0.0, -r)
        hi = (r, h, r)
        kit.geometry("ozg_chess_%s geometry" % kind, kit.GEOMETRY, lo, hi, MASS[kind], (("autocenter", "0"),))
        kit.geometry("ozg_chess_%s view" % kind, kit.VIEW_GEOMETRY, lo, hi)
        kit.geometry("ozg_chess_%s fire" % kind, kit.FIRE_GEOMETRY, lo, hi,
                     mat=kit.penetration_material(r"dz\data\data\penetration\wood.rvmat"))
        w = r + 0.002
        kit.memory("ozg_chess_%s memory" % kind, (
            ("boundingbox_min", (-w, -0.001, -w)),
            ("boundingbox_max", (w, h + 0.001, w)),
            ("invView", (0.03, h * 0.75, 0.16)),
            ("ce_center", (0.0, h / 2.0, 0.0)),
            ("ce_radius", (h / 2.0 + 0.01, 0.0, 0.0)),
        ))
        kit.save("chess_%s" % kind)
        if kit.wants_preview():
            kit.preview("chess_%s" % kind, (0.0, h / 2.0, 0.0), yaw=200.0, pitch=12.0, distance=0.28)


main()
