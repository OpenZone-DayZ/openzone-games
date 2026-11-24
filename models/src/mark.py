"""The mark on the floor where a player may sit down: a ring with an arrow head in it.

    blender -b -t 4 --python models/src/mark.py [-- --no-preview]

Builds models/src/mark.blend from nothing and renders models/preview/mark.png.
The model it becomes is OpenZone_Games\\models\\mark\\ozg_mark.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The origin is on the
floor in the middle of the ring; the ring and the arrow head lie flat, LIFT above it,
and the arrow head points towards +Z. Every face looks up: the mark is seen from above.

Nothing here stops a body or takes the cursor's ray: the model has no Geometry, no
View Geometry and no Fire Geometry. Its colour is a procedural texture, so that the
config can put another one on through the selection "camo"; the glow's colour is the
material's (ozg_mark.rvmat, emmisive[]), and a mark of another colour names both.

THE ORIGIN. A model's origin is moved to the middle of the box round it unless its
Geometry LOD says autocenter = 0, and this model has no Geometry LOD to say it in:
binarize does not read the property from a visual LOD (measured: with it and without
it the built model carried the same bounding centre, 0.02 up, which puts the ring ON
the floor). So the box is made to have its middle at the origin instead: one point
with no face on it lies as far under the floor as the ring lies above it. With that
point the built model's bounding centre is 0, 0, 0 (measured), and nothing is drawn
or met there. KEEP_ORIGIN = "box" is the other way: a real Geometry LOD, a 1 cm box
on the floor in the middle of the ring, with autocenter = 0 and a mass of 10 g.
"""
import math
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

RING_OUT = 0.24
RING_IN = 0.19
SEGMENTS = 32
LIFT = 0.02

# The arrow head: two arms that meet at the tip.
TIP_Z = 0.100               # the tip
ARM_X = 0.115               # how far an arm reaches to each side
ARM_BACK = 0.115            # and how far back from the tip
ARM_THICK = 0.070           # an arm's thickness, measured along Z

KEEP_ORIGIN = "point"        # "point": a point under the floor, no geometry at all; "box": a Geometry LOD

COLOUR = "#(argb,8,8,3)color(0.25,0.9,0.35,1.0,co)"
MATERIAL = kit.MOD + r"\models\mark\data\ozg_mark.rvmat"


def flat(x, z):
    """A point of the mark, and where it is on a picture laid over the whole ring."""
    return (x, LIFT, z), (x / (2.0 * RING_OUT) + 0.5, z / (2.0 * RING_OUT) + 0.5)


def face(lod, part, corners, paint):
    points, uvs = zip(*(flat(x, z) for x, z in corners))
    lod.face(part, points, uvs, (0, 1, 0), paint)


def main():
    kit.start()
    paint = kit.material("mark", COLOUR, MATERIAL, colour=(0.25, 0.9, 0.35), glow=0.8)
    lod = kit.Lod()
    for i in range(SEGMENTS):
        a0 = 2.0 * math.pi * i / SEGMENTS
        a1 = 2.0 * math.pi * (i + 1) / SEGMENTS
        face(lod, "ring", [(RING_IN * math.sin(a0), RING_IN * math.cos(a0)),
                           (RING_OUT * math.sin(a0), RING_OUT * math.cos(a0)),
                           (RING_OUT * math.sin(a1), RING_OUT * math.cos(a1)),
                           (RING_IN * math.sin(a1), RING_IN * math.cos(a1))], paint)
    tip, notch = (0.0, TIP_Z), (0.0, TIP_Z - ARM_THICK)
    for side in (-1.0, 1.0):
        face(lod, "arrow", [tip, (side * ARM_X, TIP_Z - ARM_BACK),
                            (side * ARM_X, TIP_Z - ARM_BACK - ARM_THICK), notch], paint)
    if KEEP_ORIGIN == "point":
        lod.point("origin", (0.0, -LIFT, 0.0))
    lod.select("camo")
    # No shadow from a mark that floats a finger above the floor: neither from the LOD
    # nor from its faces.
    lod.finish("ozg_mark 1", kit.VISUAL, 1, (("lodnoshadow", "1"),), shadow=False)
    if KEEP_ORIGIN == "box":
        kit.geometry("ozg_mark geometry", kit.GEOMETRY, (-0.005, 0.0, -0.005), (0.005, 0.01, 0.005),
                     0.01, (("autocenter", "0"),))
    kit.save("mark")
    if kit.wants_preview():
        kit.preview("mark", (0.0, 0.0, 0.0), yaw=0.0, pitch=50.0, distance=1.45,
                    ground=(0.10, 0.10, 0.10), sun=0.6, sky=0.2)


main()
