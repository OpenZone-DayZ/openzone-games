"""The classic chess set from Poly Haven's "Chess Set" by Riley Queen (CC0,
https://polyhaven.com/a/chess_set): one piece of each kind, reduced, for chess.py.

    blender -b -t 4 --python models/src/chess_polyhaven.py -- <chess_set_1k.gltf>

Reads the glTF of the set's 1k download (its textures/ folder beside it) and writes

    models/src/chess_classic.obj                the six pieces, reduced to TARGET triangles
    models/chess/data/ozg_chess_white_co.png    the white pieces' picture, PICTURE px
    models/chess/data/ozg_chess_black_co.png    the black pieces' picture: the same layout
    models/chess/data/ozg_chess_nohq.png        the white pieces' normal map, its green
                                                turned over as the game reads it

The pieces are the white ones (piece_<kind>_white, the first of each); the black ones lie
on their picture the same way, which this script checks and prints. Each piece is scaled by
SCALE (the king 58.8 mm high, its base 26.6 mm across, on a square of 35 mm) and stands with
the middle of its base at the origin. The knight looks towards Blender's +Y (the game's +Z),
the king's cross spreads along X, so both show their faces from there.

The points are Blender's: X right, Y forward, Z up, metres; the game's (X, Y, Z) is the
(x, z, y) of a point here. Faces go round counter-clockwise seen from outside. The reduction
is Blender's own (Decimate, Collapse), so another Blender may cut a little differently.
"""
import math
import os
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(HERE)
OBJ = os.path.join(HERE, "chess_classic.obj")
DATA = os.path.join(MODELS, "chess", "data")
KINDS = ("k", "q", "r", "b", "n", "p")
NAMES = {"k": "piece_king_%s", "q": "piece_queen_%s", "r": "piece_rook_%s_01", "b": "piece_bishop_%s_01",
         "n": "piece_knight_%s_01", "p": "piece_pawn_%s_01"}
TARGET = {"k": 520, "q": 460, "r": 420, "b": 400, "n": 640, "p": 260}
SCALE = 0.62
BASE_BAND = 0.0025           # metres of the original above its lowest point that count as the base
PICTURE = 512


def baked(ob):
    """The object's mesh in the world, as a new mesh."""
    me = ob.data.copy()
    me.transform(ob.matrix_world)
    return me


def stand(me):
    """Moves the mesh so that the middle of its base is at the origin, and scales it."""
    zs = [v.co.z for v in me.vertices]
    low = min(zs)
    base = [v.co for v in me.vertices if v.co.z < low + BASE_BAND]
    cx = (min(c.x for c in base) + max(c.x for c in base)) / 2.0
    cy = (min(c.y for c in base) + max(c.y for c in base)) / 2.0
    me.transform(Matrix.Scale(SCALE, 4) @ Matrix.Translation((-cx, -cy, -low)))


def turn_to(me, kind):
    """The knight's muzzle towards +Y; the king's cross along X."""
    h = max(v.co.z for v in me.vertices)
    if kind == "n":
        upper = [v.co for v in me.vertices if v.co.z > 0.55 * h]
        tip = max(upper, key=lambda c: c.x * c.x + c.y * c.y)
        angle = math.atan2(tip.y, tip.x)
        me.transform(Matrix.Rotation(math.pi / 2.0 - angle, 4, 'Z'))
        print("knight: the muzzle was at %.1f degrees from +X, turned to +Y" % math.degrees(angle))
    if kind == "k":
        top = [v.co for v in me.vertices if v.co.z > 0.9 * h]
        wide_x = max(c.x for c in top) - min(c.x for c in top)
        wide_y = max(c.y for c in top) - min(c.y for c in top)
        if wide_y > wide_x:
            me.transform(Matrix.Rotation(math.pi / 2.0, 4, 'Z'))
        print("king: the top %.1f mm along X, %.1f along Y%s" % (
            wide_x * 1000, wide_y * 1000, ", turned a quarter" if wide_y > wide_x else ""))


def reduced(me, kind):
    """A new mesh: `me` decimated to about TARGET[kind] triangles, all of them triangles."""
    ob = bpy.data.objects.new("low_" + kind, me)
    bpy.context.scene.collection.objects.link(ob)
    before = sum(len(p.vertices) - 2 for p in me.polygons)
    mod = ob.modifiers.new("reduce", 'DECIMATE')
    mod.decimate_type = 'COLLAPSE'
    mod.ratio = min(1.0, TARGET[kind] / float(before))
    mod.use_collapse_triangulate = True
    low = bpy.data.meshes.new_from_object(ob.evaluated_get(bpy.context.evaluated_depsgraph_get()))
    bm = bmesh.new()
    bm.from_mesh(low)
    bmesh.ops.triangulate(bm, faces=bm.faces[:])
    bm.to_mesh(low)
    bm.free()
    print("%s: %d triangles reduced to %d" % (kind, before, len(low.polygons)))
    return low


def uv_box(me):
    uv = me.uv_layers.active.data
    us = [d.uv.x for d in uv]
    vs = [d.uv.y for d in uv]
    return min(us), min(vs), max(us), max(vs)


def write_obj(meshes):
    lines = ["# The classic chess set: Poly Haven's \"Chess Set\" by Riley Queen (CC0,",
             "# https://polyhaven.com/a/chess_set), one piece of each kind, reduced by",
             "# models/src/chess_polyhaven.py. Blender's axes: X right, Y forward, Z up, metres;",
             "# the game's (X, Y, Z) is (x, z, y). Faces counter-clockwise seen from outside.",
             "# The pictures: models/chess/data/ozg_chess_white_co.png and ..._black_co.png."]
    v_base = 1
    t_base = 1
    for kind in KINDS:
        me = meshes[kind]
        uv = me.uv_layers.active.data
        lines.append("o %s" % kind)
        for v in me.vertices:
            lines.append("v %.6f %.6f %.6f" % tuple(v.co))
        uvs = {}
        order = []
        for p in me.polygons:
            for li in p.loop_indices:
                key = (round(uv[li].uv.x, 5), round(uv[li].uv.y, 5))
                if key not in uvs:
                    uvs[key] = len(order)
                    order.append(key)
        for u, v in order:
            lines.append("vt %.5f %.5f" % (u, v))
        for p in me.polygons:
            corners = []
            for vi, li in zip(p.vertices, p.loop_indices):
                key = (round(uv[li].uv.x, 5), round(uv[li].uv.y, 5))
                corners.append("%d/%d" % (v_base + vi, t_base + uvs[key]))
            lines.append("f " + " ".join(corners))
        v_base += len(me.vertices)
        t_base += len(order)
    with open(OBJ, "w", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote", OBJ)


def write_picture(src, dst, normal=False):
    """`src` at PICTURE px as a PNG; a normal map gets its green turned over."""
    img = bpy.data.images.load(src)
    if normal:
        img.colorspace_settings.name = 'Non-Color'
    img.scale(PICTURE, PICTURE)
    if normal:
        n = PICTURE * PICTURE * 4
        px = [0.0] * n
        img.pixels.foreach_get(px)
        for i in range(1, n, 4):
            px[i] = 1.0 - px[i]
        img.pixels.foreach_set(px)
    img.filepath_raw = dst
    img.file_format = 'PNG'
    img.save()
    print("wrote", dst)


def main():
    src = sys.argv[sys.argv.index("--") + 1]
    textures = os.path.join(os.path.dirname(os.path.abspath(src)), "textures")
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.gltf(filepath=src)
    meshes = {}
    for kind in KINDS:
        white, black = (bpy.data.objects[NAMES[kind] % c] for c in ("white", "black"))
        print("%s: the picture's box, white %s, black %s" % (
            kind, tuple(round(x, 4) for x in uv_box(white.data)), tuple(round(x, 4) for x in uv_box(black.data))))
        me = baked(white)
        stand(me)
        turn_to(me, kind)
        low = reduced(me, kind)
        h = max(v.co.z for v in low.vertices)
        r = max(math.hypot(v.co.x, v.co.y) for v in low.vertices)
        print("%s: %.1f mm high, %.1f mm round the axis at most" % (kind, h * 1000, r * 1000))
        meshes[kind] = low
    write_obj(meshes)
    os.makedirs(DATA, exist_ok=True)
    write_picture(os.path.join(textures, "chess_set_pieces_white_diff_1k.jpg"),
                  os.path.join(DATA, "ozg_chess_white_co.png"))
    write_picture(os.path.join(textures, "chess_set_pieces_black_diff_1k.jpg"),
                  os.path.join(DATA, "ozg_chess_black_co.png"))
    write_picture(os.path.join(textures, "chess_set_pieces_white_nor_gl_1k.jpg"),
                  os.path.join(DATA, "ozg_chess_nohq.png"), normal=True)


main()
