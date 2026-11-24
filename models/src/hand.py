"""A fan of eight playing cards as held in one hand.

    blender -b -t 4 --python models/src/hand.py [-- --no-preview]

Builds models/src/hand.blend from nothing and renders models/preview/hand.png.
The model it becomes is OpenZone_Games\\models\\hand\\ozg_hand.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The fan is drawn in a
frame of its own: the pivot, where the fingers hold it, is the origin; the cards rise
towards +Y, the fan spreads along X, the backs look towards +Z and the faces towards
-Z, where the holder's eyes are. card1 is the holder's leftmost card (at -X) and card8
his rightmost; each card lies over its left neighbour, the way a fan is held so that
every card's corner shows. BACKS, TURN and SHIFT then say how that stands in the model,
and so in the hand: the model is that frame when they are "+Z" and zeros.

The game's own Paper, whose pose a fan may borrow, stands the same way: a sheet 0.22
wide along X and 0.29 high along Y, thin along Z, with its MIDDLE at the origin (read off
dz\\gear\\consumables\\Paper.p3d). So in that pose the fan's pivot is where the middle
of the sheet would be.
"""
import math
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

BACKS = "+Z"                # where the backs look: "+Z", or "-Z" (the fan turned half round the vertical axis)
TURN = (0.0, 0.0, 0.0)      # then, degrees: yaw (clockwise seen from above), tilt (the top goes towards +Z),
#                             lean (the top goes towards +X); it is leaned first, then tilted, then turned
SHIFT = (0.0, 0.0, 0.0)     # then moved, metres along X, Y, Z: the pivot leaves the origin by this much

CARDS = 8
STEP = 12.0                 # degrees between neighbours: 84 from the first card to the last
PIVOT_UP = 0.012            # the pivot, up from a card's bottom edge on its middle line
GAP = 0.0006                # each card this much nearer the holder than its left neighbour
THICK = 0.0003              # between the two sides of one card

MASS = 0.05                 # kg


def pose():
    return kit.Pose(TURN[0], TURN[1], TURN[2], SHIFT, backwards=(BACKS == "-Z"))


def visual():
    sheet = kit.sheet_material()
    lod = kit.Lod(pose())
    w, h = kit.CARD_WIDE, kit.CARD_LONG
    for i in range(CARDS):
        part = "card%d" % (i + 1)
        angle = math.radians((i - (CARDS - 1) / 2.0) * STEP)
        right = (math.cos(angle), -math.sin(angle), 0.0)         # the card's own right, as its holder sees it
        up = (math.sin(angle), math.cos(angle), 0.0)
        depth = ((CARDS - 1) / 2.0 - i) * GAP

        def corner(side, z):
            """The card's bottom corner on the holder's left (-1) or right (+1)."""
            return (side * right[0] * w / 2.0 - up[0] * PIVOT_UP,
                    side * right[1] * w / 2.0 - up[1] * PIVOT_UP,
                    z)

        # The back, as somebody in front of the holder sees it: the holder's right is his left.
        lod.rect(part, corner(+1, depth + THICK / 2.0), (-right[0], -right[1], 0.0), up,
                 w, h, kit.SHEET_BACK, sheet)
        # The face, as the holder sees it.
        lod.rect(part, corner(-1, depth - THICK / 2.0), right, up, w, h, kit.SHEET_FACE, sheet)
        lod.select(part, part)
    return lod.finish("ozg_hand 1", kit.VISUAL, 1)


def main():
    kit.start()
    visual()
    # Something for the engine to weigh and to drop: a box the size of one card standing
    # at the pivot. autocenter = 0 keeps the origin where it is drawn.
    kit.geometry("ozg_hand geometry", kit.GEOMETRY,
                 (-0.03, -PIVOT_UP, -0.005), (0.03, 0.09 - PIVOT_UP, 0.005),
                 MASS, (("autocenter", "0"),), pose=pose())
    kit.save("hand")
    if kit.wants_preview():
        # Two of them for the picture: on the left as the others see it, the backs; on the
        # right the same fan turned round, as its holder sees it.
        import bpy
        fan = bpy.data.objects["ozg_hand 1"]
        twin = fan.copy()
        twin.rotation_euler = (0.0, 0.0, math.pi)
        twin.location = kit.P((-0.11, 0.0, 0.0))
        fan.location = kit.P((0.11, 0.0, 0.0))
        bpy.context.scene.collection.objects.link(twin)
        kit.preview("hand", (0.0, 0.022, 0.0), yaw=180.0, pitch=16.0, distance=0.9, floor_y=-0.12)


main()
