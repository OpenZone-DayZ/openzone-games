"""The checker of the "3 in 1" board, for draughts, giveaway, corners and nardy (boards and
dice design §3.2), and the stalker draughts' disc (words 30 to 32).

    blender -b -t 4 --python models/src/disc.py [-- --no-preview] [-- disc] [-- disc_faces]

Builds models/src/disc.blend and disc_faces.blend and renders models/preview/disc.png and
disc_faces.png; the models they become are OpenZone_Games\\models\\disc\\ozg_disc.p3d and
ozg_disc_faces.p3d. A disc 30 mm across and 7 mm high lying flat, the middle of its
underside at the origin. The colour is the hidden selection `camo`.

ozg_disc: both caps show the middle of the picture (the turned rings), the edge its plain
margin. ozg_disc_faces: the top cap shows the picture's top left quarter, the underside its
top right quarter, the edge the band below them -- a face on each side
(models/disc/data/ozg_disc_stalker_<side>_co.png, chess_stalker_prep.py): a man, and turned
over, the king.
"""
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import turn_kit as turn  # noqa: E402

MATERIAL = kit.MOD + r"\models\disc\data\ozg_disc.rvmat"
R = 0.015
H = 0.007
SIDES = 24

MODELS = {
    "disc": dict(texture="ozg_disc_white_co", colour=(0.9, 0.86, 0.75),
                 edge_u=(0.0, 0.1), edge_v=(0.02, 0.06),
                 top=((0.5, 0.5), 0.45), under=((0.5, 0.5), 0.45)),
    "disc_faces": dict(texture="ozg_disc_stalker_white_co", colour=(0.86, 0.81, 0.70),
                       edge_u=(0.0, 1.0), edge_v=(0.08, 0.2),
                       top=((0.25, 0.75), 0.245), under=((0.75, 0.75), 0.245)),
}


def build(model, m):
    name = "ozg_" + model
    kit.start()
    texture = kit.MOD + r"\models\disc\data" + "\\" + m["texture"] + ".paa"
    mat = kit.material("disc", texture, MATERIAL, image=kit.source("disc", "data", m["texture"] + ".png"),
                       colour=m["colour"], rough=0.4)
    lod = kit.Lod()
    # The edge: a band of the picture outside its faces.
    profile = [(0.0, R - 0.0005), (0.0005, R), (H - 0.0005, R), (H, R - 0.0005)]
    turn.lathe(lod, "edge", profile, SIDES, mat, v_span=m["edge_v"], close_bottom=False, u_span=m["edge_u"])
    turn.disc(lod, "top", H, R - 0.0005, (0, 1, 0), SIDES, mat, uv_centre=m["top"][0], uv_r=m["top"][1])
    turn.disc(lod, "under", 0.0, R - 0.0005, (0, -1, 0), SIDES, mat, uv_centre=m["under"][0], uv_r=m["under"][1])
    lod.select("camo")
    lod.finish(name + " 1", kit.VISUAL, 1)
    lo = (-R, 0.0, -R)
    hi = (R, H, R)
    kit.geometry(name + " geometry", kit.GEOMETRY, lo, hi, 0.01, (("autocenter", "0"),))
    kit.geometry(name + " view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry(name + " fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\plastic.rvmat"))
    kit.memory(name + " memory", (
        ("boundingbox_min", (-R - 0.002, -0.001, -R - 0.002)),
        ("boundingbox_max", (R + 0.002, H + 0.001, R + 0.002)),
        ("invView", (0.0, 0.09, 0.06)),
        ("ce_center", (0.0, H / 2.0, 0.0)),
        ("ce_radius", (0.02, 0.0, 0.0)),
    ))
    kit.save(model)
    if kit.wants_preview():
        kit.preview(model, (0.0, H / 2.0, 0.0), yaw=190.0, pitch=40.0, distance=0.16)


def main():
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    named = [a for a in args if a in MODELS]
    for model in named or list(MODELS):
        build(model, MODELS[model])


main()
