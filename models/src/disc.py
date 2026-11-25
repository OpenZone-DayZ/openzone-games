"""The checker of the "3 in 1" board, for draughts, giveaway, corners and nardy (boards and
dice design §3.2).

    blender -b -t 4 --python models/src/disc.py [-- --no-preview]

Builds models/src/disc.blend and renders models/preview/disc.png; the model it becomes is
OpenZone_Games\\models\\disc\\ozg_disc.p3d. A disc 30 mm across and 7 mm high lying flat,
the middle of its underside at the origin. The colour is the hidden selection `camo`.
"""
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import turn_kit as turn  # noqa: E402

TEXTURE = kit.MOD + r"\models\disc\data\ozg_disc_white_co.paa"
MATERIAL = kit.MOD + r"\models\disc\data\ozg_disc.rvmat"
R = 0.015
H = 0.007
SIDES = 24


def main():
    kit.start()
    mat = kit.material("disc", TEXTURE, MATERIAL, image=kit.source("disc", "data", "ozg_disc_white_co.png"),
                       colour=(0.9, 0.86, 0.75), rough=0.4)
    lod = kit.Lod()
    # The edge: its picture is the plain margin of the texture, out of the rings.
    profile = [(0.0, R - 0.0005), (0.0005, R), (H - 0.0005, R), (H, R - 0.0005)]
    turn.lathe(lod, "edge", profile, SIDES, mat, v_span=(0.02, 0.06), close_bottom=False, u_span=(0.0, 0.1))
    turn.disc(lod, "top", H, R - 0.0005, (0, 1, 0), SIDES, mat)
    turn.disc(lod, "under", 0.0, R - 0.0005, (0, -1, 0), SIDES, mat)
    lod.select("camo")
    lod.finish("ozg_disc 1", kit.VISUAL, 1)
    lo = (-R, 0.0, -R)
    hi = (R, H, R)
    kit.geometry("ozg_disc geometry", kit.GEOMETRY, lo, hi, 0.01, (("autocenter", "0"),))
    kit.geometry("ozg_disc view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_disc fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\plastic.rvmat"))
    kit.memory("ozg_disc memory", (
        ("boundingbox_min", (-R - 0.002, -0.001, -R - 0.002)),
        ("boundingbox_max", (R + 0.002, H + 0.001, R + 0.002)),
        ("invView", (0.0, 0.09, 0.06)),
        ("ce_center", (0.0, H / 2.0, 0.0)),
        ("ce_radius", (0.02, 0.0, 0.0)),
    ))
    kit.save("disc")
    if kit.wants_preview():
        kit.preview("disc", (0.0, H / 2.0, 0.0), yaw=190.0, pitch=40.0, distance=0.16)


main()
