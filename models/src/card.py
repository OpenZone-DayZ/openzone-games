"""One playing card as an item: a thin card with a face and a back, each its own picture.

    blender -b -t 4 --python models/src/card.py [-- --no-preview]

Builds models/src/card.blend from nothing and renders models/preview/card.png.
The model it becomes is OpenZone_Games\\models\\card\\ozg_card.p3d (models/README.md).

Every card of every deck is this one model: what tells them apart is the pictures, which
the config and the script put on the two hidden selections, `face` and `back`. Each
selection takes a whole texture, stretched over the card.

Sizes are the game's: X to the right, Y up, Z forward, metres. The card STANDS in its
model, 0.063 along X and 0.088 along Y, its face towards -Z and its back towards +Z, as
the game's own PunchedCard stands (its printed side towards -Z, where its inventory camera
is). A thing that stands so is laid flat on the ground by `rotationFlags = 16` in its
config, and its picture in the inventory is taken from the `invView` point in front of
its face (see memory() in ozg_kit.py).

SHIFT moves the card in the model, and so in the hand: the pose borrowed for it is the
PunchedCard's, which holds a card 0.106 wide by its end.
"""
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

SHIFT = (0.0, 0.0, 0.0)     # metres along X, Y, Z: the card leaves the origin by this much

THICK = 0.0003              # between the face and the back
ROUND = 0.0035              # the radius of a corner
MASS = 0.002                # kg

# The classic deck's pictures (tools/make_cards.py names them ozg_<style>_<face>_co.paa);
# the game puts the card's own on the two hidden selections.
FACE_TEXTURE = kit.MOD + r"\data\cards\classic\ozg_classic_As_co.paa"
BACK_TEXTURE = kit.MOD + r"\data\cards\classic\ozg_classic_back_co.paa"
CARD_MATERIAL = kit.MOD + r"\data\cards\ozg_card.rvmat"


def picture(name):
    path = os.path.join(kit.REPO, "build", "cards", "3d", "classic", name + "_co.png")
    return path if os.path.isfile(path) else ""


def visual():
    face_mat = kit.material("card face", FACE_TEXTURE, CARD_MATERIAL, image=picture("c_As"),
                            colour=(0.95, 0.93, 0.88), rough=0.6)
    back_mat = kit.material("card back", BACK_TEXTURE, CARD_MATERIAL, image=picture("back"),
                            colour=(0.45, 0.08, 0.12), rough=0.5)
    pose = kit.Pose(shift=SHIFT)
    lod = kit.Lod(pose)
    w, h = kit.CARD_WIDE, kit.CARD_LONG
    outline = kit.card_outline(w, h, ROUND)
    # The face, as somebody at -Z sees it: his right is +X, his up +Y.
    lod.face("face", [(x, y, -THICK / 2.0) for x, y in outline],
             [((x + w / 2.0) / w, (y + h / 2.0) / h) for x, y in outline], (0.0, 0.0, -1.0), face_mat)
    # The back, as somebody at +Z sees it: his right is -X.
    lod.face("back", [(x, y, THICK / 2.0) for x, y in outline],
             [((w / 2.0 - x) / w, (y + h / 2.0) / h) for x, y in outline], (0.0, 0.0, 1.0), back_mat)
    lod.select("face", "face")
    lod.select("back", "back")
    lod.finish("ozg_card 1", kit.VISUAL, 1)
    return pose


def main():
    kit.start()
    pose = visual()
    w, h = kit.CARD_WIDE, kit.CARD_LONG
    lo, hi = (-w / 2.0, -h / 2.0, -0.001), (w / 2.0, h / 2.0, 0.001)
    # autocenter = 0: the origin stays where SHIFT leaves it, which is what places the card in the hand.
    kit.geometry("ozg_card geometry", kit.GEOMETRY, lo, hi, MASS, (("autocenter", "0"),), pose=pose)
    kit.geometry("ozg_card view", kit.VIEW_GEOMETRY, lo, hi, pose=pose)
    sx, sy, sz = SHIFT
    kit.memory("ozg_card memory", (
        ("boundingbox_min", (sx - w / 2.0 - 0.002, sy - h / 2.0 - 0.002, sz - 0.002)),
        ("boundingbox_max", (sx + w / 2.0 + 0.002, sy + h / 2.0 + 0.002, sz + 0.002)),
        ("invView", (sx, sy, sz - 0.30)),
        ("ce_center", (sx, sy, sz)),
        ("ce_radius", (sx + 0.055, sy, sz)),
    ))
    kit.save("card")
    if kit.wants_preview():
        kit.preview("card", (SHIFT[0], SHIFT[1], 0.0), yaw=20.0, pitch=12.0, distance=0.30, floor_y=-0.05)


main()
