"""The "3 in 1" board: folded, the item; unfolded, the place it is laid out as (boards and
dice design §3.1, §4).

    blender -b -t 4 --python models/src/board.py [-- --no-preview]

Builds models/src/board.blend and models/src/board_open.blend and renders their previews;
the models they become are OpenZone_Games\\models\\board\\ozg_board.p3d and
ozg_board_open.p3d. Both take the one picture of the board (make_textures.py,
board_picture): the 8 x 8 field in its frame, 320 mm a side.

Folded, the board is a box 320 x 160 mm and 50 mm high lying flat, the field's far half
on top and its near half underneath, as the halves fold together. Unfolded it is 320 x 320
mm and 25 mm high, the field up. The middle of the underside is the origin of both.
"""
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import turn_kit as turn  # noqa: E402

TEXTURE = kit.MOD + r"\models\board\data\ozg_board_co.paa"
MATERIAL = kit.MOD + r"\models\board\data\ozg_board.rvmat"
SIDE = 0.32
# The frame's plain corner, where the sides and the underside take their wood.
WOOD = (0.005, 0.005, 0.055, 0.055)


def material():
    return kit.material("board", TEXTURE, MATERIAL, image=kit.source("board", "data", "ozg_board_co.png"),
                        normal=kit.source("board", "data", "ozg_board_nohq.png"), colour=(0.4, 0.26, 0.16),
                        rough=0.4)


def folded():
    kit.start()
    mat = material()
    lod = kit.Lod()
    w, d, h = SIDE / 2.0, SIDE / 4.0, 0.05
    # Top: the far half of the field (the picture's upper half); underneath, the near half.
    turn.box(lod, "box", (-w, 0.0, -d), (w, h, d), mat, uvs={
        "top": (0.0, 0.5, 1.0, 1.0),
        "bottom": (0.0, 0.0, 1.0, 0.5),
        "front": WOOD, "back": WOOD, "right": WOOD, "left": WOOD,
    })
    lod.select("camo")
    lod.finish("ozg_board 1", kit.VISUAL, 1)
    lo, hi = (-w, 0.0, -d), (w, h, d)
    kit.geometry("ozg_board geometry", kit.GEOMETRY, lo, hi, 1.5, (("autocenter", "0"),))
    kit.geometry("ozg_board view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_board fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\wood.rvmat"))
    kit.memory("ozg_board memory", (
        ("boundingbox_min", (-w - 0.005, -0.001, -d - 0.005)),
        ("boundingbox_max", (w + 0.005, h + 0.005, d + 0.005)),
        ("invView", (0.08, 0.42, 0.42)),
        ("ce_center", (0.0, h / 2.0, 0.0)),
        ("ce_radius", (0.18, 0.0, 0.0)),
    ))
    kit.save("board")
    if kit.wants_preview():
        kit.preview("board", (0.0, h / 2.0, 0.0), yaw=200.0, pitch=35.0, distance=0.9)


def unfolded():
    kit.start()
    mat = material()
    lod = kit.Lod()
    w, h = SIDE / 2.0, 0.025
    turn.box(lod, "board", (-w, 0.0, -w), (w, h, w), mat, uvs={
        "top": (0.0, 0.0, 1.0, 1.0),
        "bottom": WOOD, "front": WOOD, "back": WOOD, "right": WOOD, "left": WOOD,
    })
    lod.select("camo")
    lod.finish("ozg_board_open 1", kit.VISUAL, 1)
    lo, hi = (-w, 0.0, -w), (w, h, w)
    # autocenter = 0: the origin stays under the middle of the board.
    kit.geometry("ozg_board_open geometry", kit.GEOMETRY, lo, hi, 1.5, (("autocenter", "0"),))
    kit.geometry("ozg_board_open view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_board_open fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\wood.rvmat"))
    kit.save("board_open")
    if kit.wants_preview():
        kit.preview("board_open", (0.0, h, 0.0), yaw=180.0, pitch=50.0, distance=1.2)


folded()
unfolded()
