"""The deck as an item a player carries and holds in one hand: a stack of cards.

    blender -b -t 4 --python models/src/deck.py [-- --no-preview]

Builds models/src/deck.blend from nothing and renders models/preview/deck.png.
The model it becomes is OpenZone_Games\\models\\deck\\ozg_deck.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The stack is drawn lying
face down, 0.063 along its right, 0.088 along its forward, 0.018 high, round its middle;
TURN then STANDS it in the model, as the game's own Matchbox stands (a box 0.065 x 0.121
x 0.037, its long side along Y and its thin side along Z, read off
dz\\gear\\consumables\\Matchbox.p3d), whose pose in the hand the deck borrows. Stood so,
the deck is 0.063 along X, 0.088 along Y and 0.018 along Z, its back towards +Z.

Measured 2026-11-24: `rotationFlags = 16` in an item's config lays the model on the ground
with its Z axis upright and +Z on top, so a thing that stands in its model lies flat (the
game's Paper) showing its +Z side; the deck as it was, lying in its model, stood on its end
on the ground, and stood with its back towards -Z it lay face up. `rotationFlags = 32` left
it standing on its end. So the back is towards +Z. In the hand the Matchbox pose turns the
model's -Z towards the thigh, which is why the underside wears the back as well (stack()'s
`back_under`): no side of the item shows the sheet's blank face. And the picture in the
inventory is taken from the `invView` point of the Memory LOD (see memory() in ozg_kit.py):
here from in front of the back, a little aside and above, so that the back and two sides
of the stack show.
"""
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

TURN = (180.0, -90.0, 0.0)  # degrees: yaw (clockwise seen from above), tilt (the top goes towards +Z),
#                             lean (the top goes towards +X); it is leaned first, then tilted, then turned.
#                             A tilt of -90 takes the back of the stack towards -Z with its far edge up;
#                             the yaw of 180 then turns it to +Z, the side rotationFlags 16 puts on top.
#                             A tilt of +90 would put the back there too, but upside down, and neither
#                             back is the same turned over: the stalkers' trefoil, the classic mark.

MASS = 0.1                  # kg


def main():
    kit.start()
    pose = kit.Pose(*TURN)
    lod = kit.Lod(pose)
    # The back on both sides: whichever way the deck is turned in the hand or on the ground,
    # it shows a back, never the sheet's blank face.
    lod.stack("deck", (0.0, -kit.DECK_HIGH / 2.0, 0.0), 0.0, kit.sheet_material(), back_under=True)
    lod.select("camo")
    lod.finish("ozg_deck 1", kit.VISUAL, 1)

    lo = (-kit.CARD_WIDE / 2.0, -kit.DECK_HIGH / 2.0, -kit.CARD_LONG / 2.0)
    hi = (kit.CARD_WIDE / 2.0, kit.DECK_HIGH / 2.0, kit.CARD_LONG / 2.0)
    # autocenter = 0: the middle of the stack is the origin, as the matchbox's is.
    kit.geometry("ozg_deck geometry", kit.GEOMETRY, lo, hi, MASS, (("autocenter", "0"),), pose=pose)
    kit.geometry("ozg_deck view", kit.VIEW_GEOMETRY, lo, hi, pose=pose)
    kit.geometry("ozg_deck fire", kit.FIRE_GEOMETRY, lo, hi, mat=kit.penetration_material(), pose=pose)
    w, h, d = kit.CARD_WIDE / 2.0 + 0.002, kit.CARD_LONG / 2.0 + 0.002, kit.DECK_HIGH / 2.0 + 0.002
    kit.memory("ozg_deck memory", (
        ("boundingbox_min", (-w, -h, -d)),
        ("boundingbox_max", (w, h, d)),
        ("invView", (0.078, 0.065, 0.24)),
        ("ce_center", (0.0, 0.0, 0.0)),
        ("ce_radius", (0.055, 0.0, 0.0)),
    ))
    kit.save("deck")
    if kit.wants_preview():
        kit.preview("deck", (0.0, 0.0, 0.0), yaw=162.0, pitch=15.0, distance=0.36, floor_y=-0.05)


main()
