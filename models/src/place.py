"""The laid-out game: a cloth mat with a deck of cards on it.

    blender -b -t 4 --python models/src/place.py [-- --no-preview]

Builds models/src/place.blend from nothing and renders models/preview/place.png.
The model it becomes is OpenZone_Games\\models\\place\\ozg_place.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The mat lies flat; the
origin is the middle of its underside and the underside is Y = 0.
"""
import math
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

MAT_X = 0.50                # along X
MAT_Z = 0.35                # along Z
MAT_THICK = 0.004
MAT_CORNER = 0.015          # make_textures.py draws the binding round this same corner
CORNER_STEPS = 3            # faces round each corner

DECK_AT_X = 0.12            # the deck, from the middle of the mat towards +X
DECK_TURN = 10.0            # degrees, clockwise seen from above

# What stops a body and what the cursor's ray meets: one box over the whole mat.
BOX_HIGH = 0.03
MASS = 0.4                  # kg, the whole thing

TEXTURE = kit.MOD + r"\models\place\data\ozg_place_co.paa"
MATERIAL = kit.MOD + r"\models\place\data\ozg_place.rvmat"


def outline():
    """The mat's edge seen from above, anticlockwise from the -Z end of its +X side:
    (x, z, and the direction from there towards the inside of the mat)."""
    a, b, r = MAT_X / 2.0, MAT_Z / 2.0, MAT_CORNER
    points = []
    for cx, cz, start in ((a - r, r - b, -90.0), (a - r, b - r, 0.0), (r - a, b - r, 90.0), (r - a, r - b, 180.0)):
        for i in range(CORNER_STEPS + 1):
            angle = math.radians(start + 90.0 * i / CORNER_STEPS)
            nx, nz = math.cos(angle), math.sin(angle)
            points.append((cx + r * nx, cz + r * nz, -nx, -nz))
    return points


def on_cloth(x, z):
    """Where a point of the mat is on its texture: the top face takes the whole picture,
    +Z at the picture's top."""
    return (x / MAT_X + 0.5, z / MAT_Z + 0.5)


def visual():
    cloth = kit.material("mat", TEXTURE, MATERIAL,
                         image=kit.source("place", "data", "ozg_place_co.png"),
                         normal=kit.source("place", "data", "ozg_place_nohq.png"),
                         colour=(0.05, 0.12, 0.07), rough=0.95)
    sheet = kit.sheet_material()
    lod = kit.Lod()
    edge = outline()
    lod.face("mat", [(x, MAT_THICK, z) for x, z, _, _ in edge], [on_cloth(x, z) for x, z, _, _ in edge],
             (0, 1, 0), cloth)
    lod.face("mat", [(x, 0.0, z) for x, z, _, _ in edge], [on_cloth(x, z) for x, z, _, _ in edge],
             (0, -1, 0), cloth)
    # The side wears the binding too: the picture is folded over the edge, so the strip
    # of it that lies just inside the edge is seen once more on the side.
    for i, (x0, z0, ix0, iz0) in enumerate(edge):
        x1, z1, ix1, iz1 = edge[(i + 1) % len(edge)]
        lod.face("mat",
                 [(x0, MAT_THICK, z0), (x1, MAT_THICK, z1), (x1, 0.0, z1), (x0, 0.0, z0)],
                 [on_cloth(x0, z0), on_cloth(x1, z1),
                  on_cloth(x1 + ix1 * MAT_THICK, z1 + iz1 * MAT_THICK),
                  on_cloth(x0 + ix0 * MAT_THICK, z0 + iz0 * MAT_THICK)],
                 (-(ix0 + ix1), 0.0, -(iz0 + iz1)), cloth)
    # Nobody sees the underside of a deck that lies on a mat.
    lod.stack("deck", (DECK_AT_X, MAT_THICK, 0.0), DECK_TURN, sheet, underside=False)
    lod.select("camo")
    lod.select("deck", "deck")
    return lod.finish("ozg_place 1", kit.VISUAL, 1)


def main():
    kit.start()
    visual()
    lo = (-MAT_X / 2.0, 0.0, -MAT_Z / 2.0)
    hi = (MAT_X / 2.0, BOX_HIGH, MAT_Z / 2.0)
    # autocenter = 0: the origin stays where it is drawn, under the mat, and not in the
    # middle of the box.
    kit.geometry("ozg_place geometry", kit.GEOMETRY, lo, hi, MASS, (("autocenter", "0"),))
    kit.geometry("ozg_place view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_place fire", kit.FIRE_GEOMETRY, lo, hi, mat=kit.penetration_material())
    kit.save("place")
    if kit.wants_preview():
        kit.preview("place", (0.0, 0.0, 0.0), yaw=24.0, pitch=38.0, distance=1.42)


main()
