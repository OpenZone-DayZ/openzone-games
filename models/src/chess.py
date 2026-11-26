"""The six pieces of the classic chess set (boards and dice design §3.2, §15): Poly Haven's
"Chess Set" by Riley Queen (CC0), reduced by chess_polyhaven.py into chess_classic.obj.

    blender -b -t 4 --python models/src/chess.py [-- --no-preview]

Builds models/src/chess_<k>.blend for k in k q r b n p and renders models/preview/chess_<k>.png.
The models they become are OpenZone_Games\\models\\chess\\ozg_chess_<k>.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. A piece STANDS in its model
as it stands on the board, the middle of its base at the origin; rotationFlags 0 keeps it
so on the ground. One model a kind: the colour is the hidden selection `camo`, the set's
white or black picture (OZG_Chess_Classic_w*, _b*), which lie on a piece the same way. The
knight looks towards +Z and the king's cross spreads along X.

The faces are smooth, but where two meet at more than SHARP degrees the edge stays sharp:
the steps of the base, the rook's battlements, the cross.
"""
import math
import os
import sys

from mathutils import Vector

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

TEXTURE = kit.MOD + r"\models\chess\data\ozg_chess_white_co.paa"
MATERIAL = kit.MOD + r"\models\chess\data\ozg_chess.rvmat"
SOURCE = os.path.join(kit.SRC, "chess_classic.obj")
MASS = {"p": 0.015, "r": 0.025, "b": 0.022, "q": 0.03, "k": 0.035, "n": 0.022}
SHARP = 50.0


def read_pieces(path):
    """{kind: [(three Blender points, three uvs), ...]} from the OBJ chess_polyhaven.py writes."""
    points, uvs, pieces = [], [], {}
    kind = None
    with open(path) as f:
        for line in f:
            word, _, rest = line.strip().partition(" ")
            if word == "o":
                kind = rest
                pieces[kind] = []
            elif word == "v":
                points.append(Vector([float(x) for x in rest.split()]))
            elif word == "vt":
                uvs.append(tuple(float(x) for x in rest.split()))
            elif word == "f":
                corners = [c.split("/") for c in rest.split()]
                pieces[kind].append(([points[int(v) - 1] for v, _ in corners],
                                     [uvs[int(t) - 1] for _, t in corners]))
    return pieces


def piece(kind, faces):
    mat = kit.material("pieces", TEXTURE, MATERIAL,
                       image=kit.source("chess", "data", "ozg_chess_white_co.png"),
                       normal=kit.source("chess", "data", "ozg_chess_nohq.png"),
                       colour=(0.85, 0.83, 0.78), rough=0.4)
    lod = kit.Lod()
    dropped = 0
    for pts, uvs in faces:
        if len({tuple(round(c, 6) for c in p) for p in pts}) < 3:
            dropped += 1
            continue
        a, b, c = pts
        try:
            face = lod.face("body", [kit.G(p) for p in pts], uvs, kit.G((b - a).cross(c - a)), mat)
        except ValueError:                   # the same face twice
            dropped += 1
            continue
        face.smooth = True
    lod.bm.normal_update()
    sharp = 0
    for edge in lod.bm.edges:
        if len(edge.link_faces) == 2 and edge.calc_face_angle(0.0) > math.radians(SHARP):
            edge.smooth = False
            sharp += 1
    print("%s: %d faces, %d dropped, %d sharp edges" % (kind, len(faces) - dropped, dropped, sharp))
    lod.select("camo")
    lod.finish("ozg_chess_%s 1" % kind, kit.VISUAL, 1)


def bounds(faces):
    """The box round the piece in the game's axes, its bottom at Y = 0."""
    pts = [kit.G(p) for f in faces for p in f[0]]
    lo = [min(p[i] for p in pts) for i in range(3)]
    hi = [max(p[i] for p in pts) for i in range(3)]
    lo[1] = 0.0
    return tuple(lo), tuple(hi)


def main():
    pieces = read_pieces(SOURCE)
    for kind in ("k", "q", "r", "b", "n", "p"):
        kit.start()
        piece(kind, pieces[kind])
        lo, hi = bounds(pieces[kind])
        h = hi[1]
        kit.geometry("ozg_chess_%s geometry" % kind, kit.GEOMETRY, lo, hi, MASS[kind], (("autocenter", "0"),))
        kit.geometry("ozg_chess_%s view" % kind, kit.VIEW_GEOMETRY, lo, hi)
        kit.geometry("ozg_chess_%s fire" % kind, kit.FIRE_GEOMETRY, lo, hi,
                     mat=kit.penetration_material(r"dz\data\data\penetration\wood.rvmat"))
        m = 0.002
        # The inventory looks at the knight from its left, so that its muzzle points left
        # as in the table window; at the others from in front.
        view = (-0.16, h * 0.75, 0.03) if kind == "n" else (0.03, h * 0.75, 0.16)
        kit.memory("ozg_chess_%s memory" % kind, (
            ("boundingbox_min", (lo[0] - m, -0.001, lo[2] - m)),
            ("boundingbox_max", (hi[0] + m, h + 0.001, hi[2] + m)),
            ("invView", view),
            ("ce_center", (0.0, h / 2.0, 0.0)),
            ("ce_radius", (h / 2.0 + 0.01, 0.0, 0.0)),
        ))
        kit.save("chess_%s" % kind)
        if kit.wants_preview():
            kit.preview("chess_%s" % kind, (0.0, h / 2.0, 0.0), yaw=250.0 if kind == "n" else 200.0, pitch=12.0,
                        distance=0.28)


main()
