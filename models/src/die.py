"""The die: one for the board and the can of dice alike (boards and dice design §3.2).

    blender -b -t 4 --python models/src/die.py [-- --no-preview]

Builds models/src/die.blend and renders models/preview/die.png; the model it becomes is
OpenZone_Games\\models\\die\\ozg_die.p3d. A cube of 16 mm with its edges and corners cut
off, the middle of its underside at the origin. Opposite faces add up to seven: 1 up, 6
down, 2 to +Z, 5 to -Z, 3 to +X, 4 to -X. The faces take their cells of the picture
(make_textures.py, DIE_CELLS: 64 px a face on 256 x 128); the cuts take the plain bone of
the last cell.
"""
import itertools
import os
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

TEXTURE = kit.MOD + r"\models\die\data\ozg_die_co.paa"
MATERIAL = kit.MOD + r"\models\die\data\ozg_die.rvmat"
S = 0.016            # the side
C = 0.0016           # how much each edge is cut
CELLS = {1: (0, 0), 2: (1, 0), 3: (2, 0), 4: (3, 0), 5: (0, 1), 6: (1, 1)}
PLAIN = (3, 1)


def cell_uv(cell, fx, fy):
    """A point of a cell (fx, fy from 0 to 1 across it, fy downwards) as a UV."""
    cx, cy = cell
    x = (cx + 0.04 + 0.92 * fx) / 4.0
    y = (cy + 0.04 + 0.92 * fy) / 2.0
    return (x, 1.0 - y)


def main():
    kit.start()
    mat = kit.material("die", TEXTURE, MATERIAL, image=kit.source("die", "data", "ozg_die_co.png"),
                       colour=(0.93, 0.91, 0.85), rough=0.35)
    lod = kit.Lod()
    h = S / 2.0
    a = h - C
    mid = (0.0, h, 0.0)

    def at(x, y, z):
        return (mid[0] + x, mid[1] + y, mid[2] + z)

    # The six faces: each a square inset by the cut, its picture the face's cell, seen
    # from outside with its "up" as given.
    faces = (
        (1, (0, 1, 0), (1, 0, 0), (0, 0, -1)),
        (6, (0, -1, 0), (1, 0, 0), (0, 0, 1)),
        (2, (0, 0, 1), (1, 0, 0), (0, 1, 0)),
        (5, (0, 0, -1), (-1, 0, 0), (0, 1, 0)),
        (3, (1, 0, 0), (0, 0, -1), (0, 1, 0)),
        (4, (-1, 0, 0), (0, 0, 1), (0, 1, 0)),
    )
    for number, n, r, u in faces:
        pts, uvs = [], []
        for fx, fy in ((0, 1), (1, 1), (1, 0), (0, 0)):
            sx = (fx * 2 - 1) * a
            sy = (1 - fy * 2) * a
            pts.append(at(n[0] * h + r[0] * sx + u[0] * sy, n[1] * h + r[1] * sx + u[1] * sy,
                          n[2] * h + r[2] * sx + u[2] * sy))
            uvs.append(cell_uv(CELLS[number], fx, fy))
        lod.face("face%d" % number, pts, uvs, n, mat)
    # The twelve cut edges and the eight cut corners, in plain bone.
    plain = [cell_uv(PLAIN, 0.2, 0.2), cell_uv(PLAIN, 0.8, 0.2), cell_uv(PLAIN, 0.8, 0.8), cell_uv(PLAIN, 0.2, 0.8)]
    for axis in range(3):
        for s1, s2 in itertools.product((-1, 1), repeat=2):
            # An edge along `axis`, at the other two axes' signs s1, s2.
            others = [i for i in range(3) if i != axis]
            pts = []
            for along in (-a, a):
                for first in (True, False):
                    p = [0.0, 0.0, 0.0]
                    p[axis] = along
                    p[others[0]] = s1 * (h if first else a)
                    p[others[1]] = s2 * (a if first else h)
                    pts.append(p)
            quad = [pts[0], pts[1], pts[3], pts[2]]
            facing = [0.0, 0.0, 0.0]
            facing[others[0]] = s1
            facing[others[1]] = s2
            lod.face("edge%d%d%d" % (axis, s1 + 1, s2 + 1), [at(*p) for p in quad], plain, tuple(facing), mat)
    for sx, sy, sz in itertools.product((-1, 1), repeat=3):
        pts = [at(sx * h, sy * a, sz * a), at(sx * a, sy * h, sz * a), at(sx * a, sy * a, sz * h)]
        lod.face("corner%d%d%d" % (sx + 1, sy + 1, sz + 1), pts, plain[:3], (sx, sy, sz), mat)
    lod.select("camo")
    lod.finish("ozg_die 1", kit.VISUAL, 1)
    lo = (-h, 0.0, -h)
    hi = (h, S, h)
    kit.geometry("ozg_die geometry", kit.GEOMETRY, lo, hi, 0.005, (("autocenter", "0"),))
    kit.geometry("ozg_die view", kit.VIEW_GEOMETRY, lo, hi)
    kit.geometry("ozg_die fire", kit.FIRE_GEOMETRY, lo, hi,
                 mat=kit.penetration_material(r"dz\data\data\penetration\plastic.rvmat"))
    kit.memory("ozg_die memory", (
        ("boundingbox_min", (-h - 0.002, -0.001, -h - 0.002)),
        ("boundingbox_max", (h + 0.002, S + 0.002, h + 0.002)),
        ("invView", (0.035, 0.05, 0.06)),
        ("ce_center", (0.0, h, 0.0)),
        ("ce_radius", (0.015, 0.0, 0.0)),
    ))
    kit.save("die")
    if kit.wants_preview():
        kit.preview("die", (0.0, h, 0.0), yaw=215.0, pitch=30.0, distance=0.12)


main()
