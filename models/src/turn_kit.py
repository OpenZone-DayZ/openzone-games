"""What the models of the "3 in 1" board share (Blender Python): bodies turned
round an upright axis, flat polygons pushed out into slabs, boxes, each face
with its picture laid on. Built on ozg_kit's Lod; the game's axes: X right,
Y up, Z forward, metres.
"""
import math

from mathutils import Vector


def V(x, y, z):
    return Vector((x, y, z))


def lathe(lod, part, profile, sides, mat, v_span=(0.0, 1.0), smooth=True, close_bottom=True,
          centre=(0.0, 0.0), u_span=(0.0, 1.0)):
    """A body turned round the upright axis through `centre` (x, z): `profile` is
    [(y, r), ...] from the bottom up, in metres. Round it the picture runs along
    u from u_span[0] to u_span[1]; up it, v from v_span[0] at the lowest point
    to v_span[1] at the highest. The bottom is closed by a disc when
    `close_bottom`; a profile that ends on r = 0 closes itself at the top."""
    cx, cz = centre
    y_lo = profile[0][0]
    y_hi = profile[-1][0]
    height = max(1e-6, y_hi - y_lo)

    def vy(y):
        return v_span[0] + (v_span[1] - v_span[0]) * (y - y_lo) / height

    def uk(k):
        return u_span[0] + (u_span[1] - u_span[0]) * k / float(sides)

    for n in range(len(profile) - 1):
        y0, r0 = profile[n]
        y1, r1 = profile[n + 1]
        for k in range(sides):
            a0 = 2.0 * math.pi * k / sides
            a1 = 2.0 * math.pi * (k + 1) / sides
            corners = ((y0, r0, a0, k), (y0, r0, a1, k + 1), (y1, r1, a1, k + 1), (y1, r1, a0, k))
            pts, uvs, seen = [], [], set()
            for y, r, a, kk in corners:
                p = (cx + r * math.sin(a), y, cz + r * math.cos(a))
                key = tuple(round(c, 7) for c in p)
                if key in seen:
                    continue
                seen.add(key)
                pts.append(p)
                uvs.append((uk(kk), vy(y)))
            if len(pts) < 3:
                continue
            am = (a0 + a1) / 2.0
            radial = V(math.sin(am), 0.0, math.cos(am))
            facing = radial * (y1 - y0) - V(0.0, 1.0, 0.0) * (r1 - r0)
            face = lod.face("%s%d" % (part, n), pts, uvs, tuple(facing), mat)
            face.smooth = smooth
    if close_bottom and profile[0][1] > 0:
        disc(lod, part + "bottom", profile[0][0], profile[0][1], (0, -1, 0), sides, mat, centre)
    if profile[-1][1] > 0:
        disc(lod, part + "top", profile[-1][0], profile[-1][1], (0, 1, 0), sides, mat, centre)


def disc(lod, part, y, r, facing, sides, mat, centre=(0.0, 0.0), uv_centre=(0.5, 0.5), uv_r=0.45):
    """A flat round face at height `y`, its picture a round spot of radius uv_r."""
    cx, cz = centre
    pts, uvs = [], []
    for k in range(sides):
        a = 2.0 * math.pi * k / sides
        pts.append((cx + r * math.sin(a), y, cz + r * math.cos(a)))
        uvs.append((uv_centre[0] + uv_r * math.sin(a), uv_centre[1] + uv_r * math.cos(a)))
    lod.face(part, pts, uvs, facing, mat)


def slab(lod, part, outline, x0, x1, mat, uv_of=None, side_uv=(0.0, 0.0, 1.0, 1.0)):
    """A flat polygon of the (z, y) plane, `outline` [(z, y), ...] in order round
    it, pushed out from x0 to x1. Its two faces take `uv_of(z, y)`; its sides run
    round side_uv = (u0, v0, u1, v1)."""
    uv_of = uv_of or (lambda z, y: (0.5, 0.5))
    lod.face(part + "a", [(x1, y, z) for z, y in outline], [uv_of(z, y) for z, y in outline], (1, 0, 0), mat)
    lod.face(part + "b", [(x0, y, z) for z, y in outline], [uv_of(z, y) for z, y in outline], (-1, 0, 0), mat)
    total = 0.0
    lengths = []
    for i in range(len(outline)):
        z0, y0 = outline[i]
        z1, y1 = outline[(i + 1) % len(outline)]
        d = math.hypot(z1 - z0, y1 - y0)
        lengths.append(d)
        total += d
    # Which way the outline turns: its signed area in the (z, y) plane.
    area = 0.0
    for i in range(len(outline)):
        z0, y0 = outline[i]
        z1, y1 = outline[(i + 1) % len(outline)]
        area += z0 * y1 - z1 * y0
    turn = 1.0 if area > 0 else -1.0
    u0, v0, u1, v1 = side_uv
    run = 0.0
    for i in range(len(outline)):
        z0, y0 = outline[i]
        z1, y1 = outline[(i + 1) % len(outline)]
        s0 = u0 + (u1 - u0) * run / total
        run += lengths[i]
        s1 = u0 + (u1 - u0) * run / total
        # Outwards: the edge's direction turned a quarter away from the inside.
        facing = (0.0, -(z1 - z0) * turn, (y1 - y0) * turn)
        face = lod.face(part + "s", [(x0, y0, z0), (x0, y1, z1), (x1, y1, z1), (x1, y0, z0)],
                        [(s0, v0), (s1, v0), (s1, v1), (s0, v1)], facing, mat)
        face.smooth = False


def box(lod, part, lo, hi, mat, uvs=None):
    """A box between two corners, each face with the uv rectangle `uvs[name]`
    (names: top, bottom, front, back, right, left) or the whole picture."""
    (x0, y0, z0), (x1, y1, z1) = lo, hi
    sides = {
        "top": ([(x0, y1, z0), (x1, y1, z0), (x1, y1, z1), (x0, y1, z1)], (0, 1, 0)),
        "bottom": ([(x0, y0, z1), (x1, y0, z1), (x1, y0, z0), (x0, y0, z0)], (0, -1, 0)),
        "front": ([(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)], (0, 0, 1)),
        "back": ([(x1, y0, z0), (x0, y0, z0), (x0, y1, z0), (x1, y1, z0)], (0, 0, -1)),
        "right": ([(x1, y0, z1), (x1, y0, z0), (x1, y1, z0), (x1, y1, z1)], (1, 0, 0)),
        "left": ([(x0, y0, z0), (x0, y0, z1), (x0, y1, z1), (x0, y1, z0)], (-1, 0, 0)),
    }
    for name, (pts, facing) in sides.items():
        u0, v0, u1, v1 = (uvs or {}).get(name, (0.0, 0.0, 1.0, 1.0))
        lod.face(part + name, pts, [(u0, v0), (u1, v0), (u1, v1), (u0, v1)], facing, mat)
