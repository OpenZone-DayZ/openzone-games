"""The can of dice and the dice laid out (boards and dice design §3.3, §4).

    blender -b -t 4 --python models/src/can.py [-- --no-preview]

Builds models/src/can.blend and can_place.blend and renders models/preview/can.png and
can_place.png. The models they become are OpenZone_Games\\models\\can\\ozg_can.p3d, the
item: a tin can 75 mm across and 90 mm high with its lid on, the middle of its bottom at
the origin; and OpenZone_Games\\models\\can\\ozg_can_place.p3d, the place: a square of the
card mat's felt, 300 x 300 mm, with the can standing at its back left corner.

The can's picture (make_textures.py, can): the side of the body in the band from 30 to 98
percent of its height, the side of the lid from 20 to 28, the lid's top and the can's
bottom in two round spots above. Every face is the selection `camo`, which the hologram
repaints; the place's felt wears the card mat's own picture.
"""
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import turn_kit as turn  # noqa: E402

TEXTURE = kit.MOD + r"\models\can\data\ozg_can_co.paa"
MATERIAL = kit.MOD + r"\models\can\data\ozg_can.rvmat"
FELT_TEXTURE = kit.MOD + r"\models\place\data\ozg_place_co.paa"
FELT_MATERIAL = kit.MOD + r"\models\place\data\ozg_place.rvmat"
SIDES = 28
R = 0.0375          # the body
R_LID = 0.0386      # the lid over it
H_BODY = 0.078
H = 0.090
FELT = 0.30
FELT_H = 0.006


def band(y_top, y_bottom):
    """A band of the picture (fractions of its height, downwards) as a lathe's v_span."""
    return (1.0 - y_bottom, 1.0 - y_top)


def can(lod, mat, centre=(0.0, 0.0), y0=0.0):
    body = [(y0 + 0.0, R - 0.001), (y0 + 0.0015, R), (y0 + H_BODY, R)]
    turn.lathe(lod, "body", body, SIDES, mat, v_span=band(0.30, 0.98), close_bottom=False, centre=centre)
    lid = [(y0 + H_BODY - 0.001, R_LID), (y0 + H - 0.0015, R_LID), (y0 + H, R_LID - 0.0008)]
    turn.lathe(lod, "lid", lid, SIDES, mat, v_span=band(0.20, 0.28), close_bottom=False, centre=centre)
    turn.disc(lod, "top", y0 + H, R_LID - 0.0008, (0, 1, 0), SIDES, mat, centre=centre,
              uv_centre=(0.12, 1.0 - 0.09), uv_r=0.085)
    turn.disc(lod, "bottom", y0 + 0.0, R - 0.001, (0, -1, 0), SIDES, mat, centre=centre,
              uv_centre=(0.36, 1.0 - 0.09), uv_r=0.085)


def can_material():
    return kit.material("can", TEXTURE, MATERIAL, image=kit.source("can", "data", "ozg_can_co.png"),
                        colour=(0.2, 0.3, 0.2), rough=0.4)


def build_item():
    kit.start()
    mat = can_material()
    lod = kit.Lod()
    can(lod, mat)
    lod.select("camo")
    lod.finish("ozg_can 1", kit.VISUAL, 1)
    lo = (-R_LID, 0.0, -R_LID)
    hi = (R_LID, H, R_LID)
    kit.geometry("ozg_can geometry", kit.GEOMETRY, lo, hi, 0.25, (("autocenter", "0"),))
    kit.geometry("ozg_can view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_can fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\metalplate.rvmat"))
    w = R_LID + 0.003
    kit.memory("ozg_can memory", (
        ("boundingbox_min", (-w, -0.001, -w)),
        ("boundingbox_max", (w, H + 0.002, w)),
        ("invView", (0.06, H * 0.8, 0.18)),
        ("ce_center", (0.0, H / 2.0, 0.0)),
        ("ce_radius", (0.06, 0.0, 0.0)),
    ))
    kit.save("can")
    if kit.wants_preview():
        kit.preview("can", (0.0, H / 2.0, 0.0), yaw=200.0, pitch=18.0, distance=0.36)


def build_place():
    kit.start()
    mat = can_material()
    felt = kit.material("felt", FELT_TEXTURE, FELT_MATERIAL, image=kit.source("place", "data", "ozg_place_co.png"),
                        colour=(0.12, 0.3, 0.18), rough=0.95)
    lod = kit.Lod()
    half = FELT / 2.0
    # The felt: the mat's whole picture on top, its stitched edge round the sides.
    lod.face("felt_top", [(-half, FELT_H, half), (half, FELT_H, half), (half, FELT_H, -half), (-half, FELT_H, -half)],
             [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)], (0, 1, 0), felt)
    lod.face("felt_under", [(-half, 0.0, -half), (half, 0.0, -half), (half, 0.0, half), (-half, 0.0, half)],
             [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)], (0, -1, 0), felt)
    edge = [(0.0, 0.0), (1.0, 0.0), (1.0, 0.02), (0.0, 0.02)]
    sides = (
        ((-half, 0.0, half), (half, 0.0, half), (0, 0, 1)),
        ((half, 0.0, -half), (-half, 0.0, -half), (0, 0, -1)),
        ((half, 0.0, half), (half, 0.0, -half), (1, 0, 0)),
        ((-half, 0.0, -half), (-half, 0.0, half), (-1, 0, 0)),
    )
    for k, (a, b, n) in enumerate(sides):
        pts = [a, b, (b[0], FELT_H, b[2]), (a[0], FELT_H, a[2])]
        lod.face("felt_side%d" % k, pts, edge, n, felt)
    can(lod, mat, centre=(-0.10, -0.10), y0=FELT_H)
    lod.select("camo")
    lod.finish("ozg_can_place 1", kit.VISUAL, 1)
    lo = (-half, 0.0, -half)
    hi = (half, FELT_H, half)
    kit.geometry("ozg_can_place geometry", kit.GEOMETRY, lo, hi, 0.6, (("autocenter", "0"),))
    kit.geometry("ozg_can_place view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_can_place fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material())
    w = half + 0.01
    kit.memory("ozg_can_place memory", (
        ("boundingbox_min", (-w, -0.001, -w)),
        ("boundingbox_max", (w, FELT_H + H + 0.002, w)),
        ("invView", (0.0, 0.35, 0.45)),
        ("ce_center", (0.0, 0.05, 0.0)),
        ("ce_radius", (0.25, 0.0, 0.0)),
    ))
    kit.save("can_place")
    if kit.wants_preview():
        kit.preview("can_place", (0.0, 0.04, 0.0), yaw=200.0, pitch=35.0, distance=0.8)


build_item()
build_place()
