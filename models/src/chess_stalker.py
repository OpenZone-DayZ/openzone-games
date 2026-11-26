"""The twelve figurines of the stalker chess set (boards and dice design §15): stalkers
against mutants, posed from GSC Game World's models of the original trilogy and baked into
one material each by the research tools (models/src/chess_stalker_prep.py brings them here).

    blender -b -t 4 --python models/src/chess_stalker.py [-- --no-preview] [-- <face> ...]

Builds models/src/chess_stalker_<face>.blend for every face (wk wq wr wb wn wp bk bq br bb bn
bp) and renders models/preview/chess_stalker_<face>.png. The models they become are
OpenZone_Games\\models\\chess_stalker\\ozg_chess_stalker_<face>.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. A figurine STANDS in its
model as on the board, the middle of its base at the origin, looking towards +Z (as the
classic knight); rotationFlags 0 keeps it so on the ground. Each has its own picture, the
hidden selection `camo`. Every figurine stays inside its square of 35 mm; the kings are 56
mm high (design §15, "Heights").
"""
import math
import os
import sys

from mathutils import Vector

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

MATERIAL = kit.MOD + r"\models\chess_stalker\data\ozg_chess_stalker.rvmat"
SOURCE = os.path.join(kit.SRC, "chess_stalker.obj")
FACES = ("wk", "wq", "wr", "wb", "wn", "wp", "bk", "bq", "br", "bb", "bn", "bp")
MASS = {"p": 0.015, "r": 0.025, "b": 0.022, "q": 0.03, "k": 0.035, "n": 0.022}
SHARP = 50.0


def read_figurines(path):
    """{face: [(three Blender points, three uvs), ...]} from chess_stalker.obj."""
    points, uvs, figures = [], [], {}
    face = None
    with open(path, encoding="utf-8") as f:
        for line in f:
            word, _, rest = line.strip().partition(" ")
            if word == "o":
                face = rest
                figures[face] = []
            elif word == "v":
                points.append(Vector([float(x) for x in rest.split()]))
            elif word == "vt":
                uvs.append(tuple(float(x) for x in rest.split()[:2]))
            elif word == "f":
                # a face of more corners than three (the bake leaves a few) is split into a fan
                corners = [c.split("/") for c in rest.split()]
                for i in range(1, len(corners) - 1):
                    tri = (corners[0], corners[i], corners[i + 1])
                    figures[face].append(([points[int(c[0]) - 1] for c in tri],
                                          [uvs[int(c[1]) - 1] for c in tri]))
    return figures


def figurine(face, faces):
    texture = kit.MOD + r"\models\chess_stalker\data\ozg_chess_stalker_%s_co.paa" % face
    mat = kit.material("figurine", texture, MATERIAL,
                       image=kit.source("chess_stalker", "data", "ozg_chess_stalker_%s_co.png" % face),
                       colour=(0.6, 0.58, 0.52), rough=0.6)
    lod = kit.Lod()
    dropped = 0
    for pts, uvs in faces:
        if len({tuple(round(c, 6) for c in p) for p in pts}) < 3:
            dropped += 1
            continue
        a, b, c = pts
        try:
            made = lod.face("body", [kit.G(p) for p in pts], uvs, kit.G((b - a).cross(c - a)), mat)
        except ValueError:                   # the same face twice
            dropped += 1
            continue
        made.smooth = True
    lod.bm.normal_update()
    sharp = 0
    for edge in lod.bm.edges:
        if len(edge.link_faces) == 2 and edge.calc_face_angle(0.0) > math.radians(SHARP):
            edge.smooth = False
            sharp += 1
    print("%s: %d faces, %d dropped, %d sharp edges" % (face, len(faces) - dropped, dropped, sharp))
    lod.select("camo")
    lod.finish("ozg_chess_stalker_%s 1" % face, kit.VISUAL, 1)


def bounds(faces):
    """The box round the figurine in the game's axes, its bottom at Y = 0."""
    pts = [kit.G(p) for f in faces for p in f[0]]
    lo = [min(p[i] for p in pts) for i in range(3)]
    hi = [max(p[i] for p in pts) for i in range(3)]
    lo[1] = 0.0
    return tuple(lo), tuple(hi)


def wanted():
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    named = [a for a in args if a in FACES]
    return named or list(FACES)


def main():
    figures = read_figurines(SOURCE)
    for face in wanted():
        kind = face[1]
        name = "ozg_chess_stalker_%s" % face
        kit.start()
        figurine(face, figures[face])
        lo, hi = bounds(figures[face])
        h = hi[1]
        kit.geometry(name + " geometry", kit.GEOMETRY, lo, hi, MASS[kind], (("autocenter", "0"),))
        kit.geometry(name + " view", kit.VIEW_GEOMETRY, lo, hi)
        kit.geometry(name + " fire", kit.FIRE_GEOMETRY, lo, hi,
                     mat=kit.penetration_material(r"dz\data\data\penetration\plastic.rvmat"))
        m = 0.002
        # The inventory looks at a figurine from in front, a little to its right.
        kit.memory(name + " memory", (
            ("boundingbox_min", (lo[0] - m, -0.001, lo[2] - m)),
            ("boundingbox_max", (hi[0] + m, h + 0.001, hi[2] + m)),
            ("invView", (0.03, h * 0.7, 0.16)),
            ("ce_center", (0.0, h / 2.0, 0.0)),
            ("ce_radius", (h / 2.0 + 0.01, 0.0, 0.0)),
        ))
        kit.save("chess_stalker_%s" % face)
        if kit.wants_preview():
            kit.preview("chess_stalker_%s" % face, (0.0, h / 2.0, 0.0), yaw=200.0, pitch=12.0, distance=0.26)


main()
