"""The deck as an item a player carries and holds in one hand: a stack of cards.

    blender -b -t 4 --python models/src/deck.py [-- --no-preview]

Builds models/src/deck.blend from nothing and renders models/preview/deck.png.
The model it becomes is OpenZone_Games\\models\\deck\\ozg_deck.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The stack lies face
down, 0.063 along X, 0.088 along Z, 0.018 high; it is drawn with the middle of its
underside at the origin. The Geometry LOD carries no autocenter property, so the
engine centres the model itself, on the middle of the stack.

TURN stands the stack another way in the model, and so in the hand. The game's own
Matchbox, whose pose a deck may borrow, is a box 0.065 x 0.121 x 0.037 that STANDS: its
long side along Y and its thin side along Z (read off dz\\gear\\consumables\\Matchbox.p3d).
A tilt of 90 or of -90 stands the deck the same way, its back towards +Z or towards -Z.
"""
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

TURN = (0.0, 0.0, 0.0)      # degrees: yaw (clockwise seen from above), tilt (the top goes towards +Z),
#                             lean (the top goes towards +X); it is leaned first, then tilted, then turned

MASS = 0.1                  # kg


def main():
    kit.start()
    pose = kit.Pose(*TURN)
    lod = kit.Lod(pose)
    lod.stack("deck", (0.0, 0.0, 0.0), 0.0, kit.sheet_material())
    lod.select("camo")
    lod.finish("ozg_deck 1", kit.VISUAL, 1)

    lo = (-kit.CARD_WIDE / 2.0, 0.0, -kit.CARD_LONG / 2.0)
    hi = (kit.CARD_WIDE / 2.0, kit.DECK_HIGH, kit.CARD_LONG / 2.0)
    kit.geometry("ozg_deck geometry", kit.GEOMETRY, lo, hi, MASS, pose=pose)
    kit.geometry("ozg_deck view", kit.VIEW_GEOMETRY, lo, hi, pose=pose)
    kit.geometry("ozg_deck fire", kit.FIRE_GEOMETRY, lo, hi, mat=kit.penetration_material(), pose=pose)
    kit.save("deck")
    if kit.wants_preview():
        kit.preview("deck", (0.0, 0.008, 0.0), yaw=32.0, pitch=36.0, distance=0.42)


main()
