"""The card sharp's kit: an open tin standing in its own lid, and the handmade things a card
sharp keeps in it -- Indian ink, glue, an awl on a scrap of sandpaper, a file, lead pips
for dice, a razor blade and two picks bound with tape.

    blender -b -t 4 --python models/src/sharpkit.py [-- --no-preview]

Builds models/src/sharpkit.blend from nothing and renders models/preview/sharpkit.png.
The model it becomes is OpenZone_Games\\models\\sharpkit\\ozg_sharpkit.p3d (models/README.md).

Sizes are the game's: X to the right, Y up, Z forward, metres. The tin STANDS in the model
as it stands on a table, open side up, the middle of the lid's underside at the origin --
as the game's own TunaCan stands (rotationFlags 32 keeps it so on the ground), whose pose
in the hand it borrows (OZG_Hand.c). What lies in it is laid out for somebody in front of
it, at +Z and above: that is where the inventory's camera (`invView`) is, and so his left
is +X.

Every face takes its picture from one texture. Where each part lies on it is in
sharpkit_atlas.py, which make_textures.py draws by.
"""
import math
import os
import sys

sys.dont_write_bytecode = True      # no __pycache__ beside the sources
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402
import sharpkit_atlas as atlas  # noqa: E402
from mathutils import Vector  # noqa: E402

# The tin, and the lid it stands in.
TIN_R = 0.044               # the tin's outside
TIN_IN = 0.0434             # its inside
TIN_TOP = 0.018             # its rim, above the underside of the lid
FLOOR = 0.0016              # the floor inside
LID_R = 0.0458
LID_HIGH = 0.0062
SIDES = 36                  # faces round the tin and round the lid
TOP = 0.034                 # the highest thing in it, the ink bottle's cap, near enough
INV_VIEW = (0.02, 0.17, 0.15)   # where the inventory's camera looks from (the Memory LOD)
SAND = 0.0004               # a scrap of sandpaper, thick
SAND_TOP = FLOOR + SAND
BLADE_TOP = SAND_TOP + 0.0002   # the razor blade lies with one end on the sandpaper; flat, near enough
MASS = 0.2                  # kg, all of it

TEXTURE = kit.MOD + r"\models\sharpkit\data\ozg_sharpkit_co.paa"
MATERIAL = kit.MOD + r"\models\sharpkit\data\ozg_sharpkit.rvmat"
METAL = r"dz\data\data\penetration\metalplate.rvmat"

UP = Vector((0.0, 1.0, 0.0))
FRONT = Vector((0.0, 0.0, 1.0))
RIGHT = Vector((-1.0, 0.0, 0.0))   # the right of somebody in front, looking at it


def V(x, y, z):
    return Vector((x, y, z))


def round_dir(f, s, phi):
    return f * math.cos(phi) + s * math.sin(phi)


def turned(lod, part, base, a, f, s, sections, sides, mat, shape=None, smooth=True):
    """A body turned round the axis `a` through `base`. Each section is
    (t0, r0, fy0, t1, r1, fy1, region): from t0 to t1 along the axis, its radius going from
    r0 to r1, mapped on `region` from fy0 to fy1 down it. Round the body runs along the
    region's x: -180 degrees at its left edge, 0 (the direction `f`) in its middle, +90 the
    direction `s`. `shape(t, r, d)` may squeeze the round: it gives the point's offset
    from the axis for the direction d. Each section is a part of its own, so the edge
    between two sections stays sharp while each is smooth round."""
    shape = shape or (lambda t, r, d: d * r)
    for n, (t0, r0, fy0, t1, r1, fy1, region) in enumerate(sections):
        name = "%s%d" % (part, n)
        for k in range(sides):
            p0 = -math.pi + 2.0 * math.pi * k / sides
            p1 = -math.pi + 2.0 * math.pi * (k + 1) / sides
            corners = ((t0, r0, fy0, p0, k), (t0, r0, fy0, p1, k + 1),
                       (t1, r1, fy1, p1, k + 1), (t1, r1, fy1, p0, k))
            pts, uvs, seen = [], [], set()
            for t, r, fy, p, kk in corners:
                where = base + a * t + shape(t, r, round_dir(f, s, p))
                key = tuple(round(c, 7) for c in where)
                if key in seen:
                    continue            # the point of a cone: a triangle, not a quad
                seen.add(key)
                pts.append(tuple(where))
                uvs.append(atlas.uv(region, kk / float(sides), fy))
            mid = round_dir(f, s, (p0 + p1) / 2.0)
            facing = mid * (t1 - t0) - a * (r1 - r0)
            face = lod.face(name, pts, uvs, tuple(facing), mat)
            face.smooth = smooth


def disc(lod, part, centre, facing, f, s, r, sides, region, mat, shape=None, spot=0.42):
    """A flat disc round `centre`, looking along `facing`; its picture is a round spot in the
    middle of `region`."""
    shape = shape or (lambda d: d * r)
    pts, uvs = [], []
    for k in range(sides):
        p = -math.pi + 2.0 * math.pi * k / sides
        pts.append(tuple(centre + shape(round_dir(f, s, p))))
        uvs.append(atlas.uv(region, 0.5 + spot * math.cos(p), 0.5 + spot * math.sin(p)))
    lod.face(part, pts, uvs, tuple(facing), mat)


def flat_disc(lod, part, y, r, facing, region, mat):
    """A disc of the tin round its axis, its picture laid on it as seen from above: +X to the
    right of the picture, +Z down it."""
    pts, uvs = [], []
    for k in range(SIDES):
        p = 2.0 * math.pi * k / SIDES
        x, z = r * math.cos(p), r * math.sin(p)
        pts.append((x, y, z))
        uvs.append(atlas.uv(region, 0.5 + 0.5 * x / r, 0.5 + 0.5 * z / r))
    lod.face(part, pts, uvs, facing, mat)


def bar(lod, part, start, end, up, width, thick, mat, top="steel", side="steel"):
    """A flat bar from `start` to `end` (the middles of its ends), its broad faces looking
    along `up` and away from it, `width` across and `thick` through. The broad faces take
    the region `top` along its x from start to end; the edges and the ends take `side`."""
    a = (end - start).normalized()
    n = (up - a * up.dot(a)).normalized()
    b = a.cross(n)
    hw, ht = width / 2.0, thick / 2.0

    def at(p, sb, sn):
        return tuple(p + b * (hw * sb) + n * (ht * sn))

    for sn in (1, -1):
        lod.face(part, [at(start, -1, sn), at(end, -1, sn), at(end, 1, sn), at(start, 1, sn)],
                 [atlas.uv(top, 0.0, 0.0), atlas.uv(top, 1.0, 0.0), atlas.uv(top, 1.0, 1.0), atlas.uv(top, 0.0, 1.0)],
                 tuple(n * sn), mat)
    for sb in (1, -1):
        lod.face(part, [at(start, sb, -1), at(end, sb, -1), at(end, sb, 1), at(start, sb, 1)],
                 [atlas.uv(side, 0.0, 0.3), atlas.uv(side, 1.0, 0.3), atlas.uv(side, 1.0, 0.7), atlas.uv(side, 0.0, 0.7)],
                 tuple(b * sb), mat)
    for p, sa in ((start, -1), (end, 1)):
        lod.face(part, [at(p, -1, -1), at(p, 1, -1), at(p, 1, 1), at(p, -1, 1)],
                 [atlas.uv(side, 0.4, 0.4), atlas.uv(side, 0.6, 0.4), atlas.uv(side, 0.6, 0.6), atlas.uv(side, 0.4, 0.6)],
                 tuple(a * sa), mat)


def frame_round(a):
    """For a thing lying along `a`: 0 degrees looking up, +90 to one side."""
    f = (UP - a * UP.dot(a)).normalized()
    return f, a.cross(f)


# ----------------------------------------------------------------------- the parts
def tin(lod, mat):
    o = V(0.0, 0.0, 0.0)
    # The outside wall above the lid, its rim, the wall inside and the floor.
    turned(lod, "wall", o, UP, FRONT, RIGHT, [(LID_HIGH, TIN_R, 1.0, TIN_TOP, TIN_R, 0.0, "wall")], SIDES, mat)
    turned(lod, "rim", o, UP, FRONT, RIGHT, [(TIN_TOP, TIN_R, 0.0, TIN_TOP, TIN_IN, 1.0, "rim")], SIDES, mat)
    turned(lod, "inner", o, UP, FRONT, RIGHT, [(TIN_TOP, TIN_IN, 0.0, FLOOR, TIN_IN, 1.0, "inner")], SIDES, mat)
    flat_disc(lod, "floor", FLOOR, TIN_IN, (0, 1, 0), "floor", mat)
    # The lid under it: the edge round the tin, its wall, its underside.
    turned(lod, "lidrim", o, UP, FRONT, RIGHT, [(LID_HIGH, LID_R, 0.0, LID_HIGH, TIN_R, 1.0, "rim")], SIDES, mat)
    turned(lod, "lidwall", o, UP, FRONT, RIGHT, [(0.0, LID_R, 1.0, LID_HIGH, LID_R, 0.0, "lid")], SIDES, mat)
    flat_disc(lod, "lidbottom", 0.0, LID_R, (0, -1, 0), "bottom", mat)


def ink_bottle(lod, mat):
    """Indian ink in a squat bottle at the back on the right, its label turned towards the
    inventory's camera (INV_VIEW)."""
    base = V(-0.024, FLOOR, -0.022)
    f = V(INV_VIEW[0] - base.x, 0.0, INV_VIEW[2] - base.z).normalized()
    s = f.cross(UP)                     # round the label from its middle: to the right as it is read
    turned(lod, "ink", base, UP, f, s, [(0.0, 0.0085, 1.0, 0.0185, 0.0085, 0.0, "ink")], 16, mat)
    turned(lod, "inkneck", base, UP, f, s, [
        (0.0185, 0.0085, 0.78, 0.0215, 0.0048, 0.88, "ink_cap"),
        (0.0215, 0.0048, 0.88, 0.0240, 0.0045, 0.98, "ink_cap"),
    ], 16, mat)
    turned(lod, "inkcap", base, UP, f, s, [
        (0.0240, 0.0045, 0.72, 0.0240, 0.0057, 0.70, "ink_cap"),
        (0.0240, 0.0057, 0.70, 0.0325, 0.0057, 0.02, "ink_cap"),
    ], 16, mat)
    disc(lod, "inkcaptop", base + UP * 0.0325, UP, f, s, 0.0057, 16, "ink_cap", mat)


def glue_tube(lod, mat):
    """A tube of glue lying across the front, its flattened end on the left, its label
    turned up towards somebody in front."""
    end = V(0.024, FLOOR + 0.0062, 0.022)      # the tip of the flattened end, on the axis
    a = V(-1.0, 0.0, 0.0)
    c = math.sqrt(0.5)
    f = V(0.0, c, c)                            # the middle of the label: up and to the front
    s = V(0.0, c, -c)                           # round it from there: up and away
    wide, thin = V(0.0, 0.0, 1.0), UP

    def squeezed(t, r, d):
        # Flat 9.5 mm either way and 0.6 mm thick up to 10 mm from the tip, round 6 mm
        # further on.
        q = min(1.0, max(0.0, (t - 0.010) / 0.006))
        half_w = 0.0095 + (r - 0.0095) * q
        half_t = 0.0006 + (r - 0.0006) * q
        return wide * (d.dot(wide) * half_w) + thin * (d.dot(thin) * half_t)

    r = 0.0062
    turned(lod, "glue", end, a, f, s, [
        (0.000, r, 0.00, 0.010, r, 1.00, "crimp"),
        (0.010, r, 0.00, 0.016, r, 0.06, "glue"),
        (0.016, r, 0.06, 0.044, r, 1.00, "glue"),
    ], 12, mat, shape=squeezed)
    disc(lod, "glueend", end, -a, f, s, r, 12, "crimp", mat, shape=lambda d: squeezed(0.0, r, d))
    turned(lod, "gluecap", end, a, f, s, [
        (0.044, r, 0.0, 0.047, 0.0028, 1.0, "steel"),
        (0.047, 0.0028, 0.10, 0.047, 0.0034, 0.12, "glue_cap"),
        (0.047, 0.0034, 0.12, 0.055, 0.0034, 0.95, "glue_cap"),
    ], 12, mat)
    disc(lod, "gluecapend", end + a * 0.055, a, f, s, 0.0034, 12, "glue_cap", mat)


def awl(lod, mat):
    """An awl across the middle, its handle on the sandpaper on the left, its point on the
    razor blade on the right."""
    tip = V(-0.034, BLADE_TOP, 0.006)
    # The handle's belly (6.2 mm across the axis, 10 mm from the butt) on the sandpaper, the
    # point on the blade.
    slope = math.atan2(SAND_TOP + 0.0058 - BLADE_TOP, 0.061)
    a = V(-math.cos(slope), -math.sin(slope), 0.0)
    butt = tip - a * 0.071
    f, s = frame_round(a)
    turned(lod, "awl", butt, a, f, s, [
        (0.000, 0.0030, 0.00, 0.003, 0.0048, 0.09, "wood"),
        (0.003, 0.0048, 0.09, 0.010, 0.0058, 0.31, "wood"),
        (0.010, 0.0058, 0.31, 0.020, 0.0056, 0.62, "wood"),
        (0.020, 0.0056, 0.62, 0.028, 0.0044, 0.87, "wood"),
        (0.028, 0.0044, 0.87, 0.032, 0.0030, 1.00, "wood"),
    ], 10, mat)
    disc(lod, "awlbutt", butt, -a, f, s, 0.0030, 10, "wood", mat)
    turned(lod, "awlferrule", butt, a, f, s, [
        (0.032, 0.0030, 0.1, 0.032, 0.0026, 0.2, "steel"),
        (0.032, 0.0026, 0.2, 0.036, 0.0026, 0.8, "steel"),
        (0.036, 0.0026, 0.8, 0.036, 0.0012, 0.9, "steel"),
    ], 10, mat)
    turned(lod, "awlspike", butt, a, f, s, [(0.036, 0.0012, 0.1, 0.071, 0.0, 0.9, "steel")], 6, mat)


def file_(lod, mat):
    """A cut-down file lying from the back on the left towards the middle, the teeth up, its
    tip on the second scrap of sandpaper."""
    bar(lod, "file", V(0.038, SAND_TOP + 0.0009, -0.002), V(0.008, SAND_TOP + 0.0009, -0.038), UP,
        0.0055, 0.0018, mat, top="file")


def lead_pips(lod, mat):
    """Lead pips for loading dice, in a little heap at the back."""
    centre = V(-0.004, FLOOR, -0.032)
    spots = ((0.0, 0.0, 0), (0.0045, 0.001, 0), (-0.004, 0.002, 0), (0.002, -0.004, 0),
             (-0.003, -0.0035, 0), (0.006, -0.003, 0), (0.001, 0.0045, 0), (0.0012, 0.0006, 1))
    for n, (dx, dz, level) in enumerate(spots):
        base = centre + V(dx, level * 0.0013, dz)
        turn = 0.9 * n
        f = V(math.sin(turn), 0.0, math.cos(turn))
        s = UP.cross(f)
        turned(lod, "pip%d" % n, base, UP, f, s, [(0.0, 0.0017, 0.1, 0.0013, 0.0017, 0.9, "lead")], 7, mat)
        disc(lod, "piptop%d" % n, base + UP * 0.0013, UP, f, s, 0.0017, 7, "lead", mat)


def sandpaper(lod, mat):
    """Two scraps of sandpaper: one under the awl's handle, one on the floor between the
    file and the awl, under the file's tip and the razor blade's end."""
    for n, (x, z, turn_deg, long, wide, region) in enumerate(((0.016, 0.010, 8.0, 0.034, 0.024, "sand"),
                                                              (0.006, -0.012, -14.0, 0.030, 0.020, "sand2"))):
        turn = math.radians(turn_deg)
        d = V(math.cos(turn), 0.0, -math.sin(turn))
        centre = V(x, FLOOR + SAND / 2.0, z)
        bar(lod, "sand%d" % n, centre - d * (long / 2.0), centre + d * (long / 2.0), UP, wide, SAND, mat,
            top=region, side=region)


def razor_blade(lod, mat):
    """A razor blade under the awl's point, for shaving the edges of a card; its left end
    lies on the second scrap of sandpaper."""
    turn = math.radians(-8.0)
    d = V(math.cos(turn), 0.0, -math.sin(turn))
    centre = V(-0.019, BLADE_TOP - 0.0001, -0.001)
    bar(lod, "blade", centre - d * 0.0215, centre + d * 0.0215, UP, 0.022, 0.0002, mat, top="blade")


def picks(lod, mat):
    """Two picks bent out of wire, their handles bound with tape, lying over the glue tube
    with their hooks down on the razor blade."""
    tube_top = FLOOR + 0.0124 + 0.0011          # where a taped handle rests on the tube
    low = BLADE_TOP + 0.00035                   # where a pick's end rests on the blade
    for n, (x1, z1, x2, z2, t_rest) in enumerate(((0.012, 0.028, -0.032, -0.004, 0.1875),
                                                   (0.008, 0.029, -0.035, -0.001, 0.233))):
        def y(t):
            return tube_top + (t - t_rest) * (low - tube_top) / (1.0 - t_rest)
        start, end = V(x1, y(0.0), z1), V(x2, y(1.0), z2)
        along = (end - start).normalized()
        bar(lod, "pick%d" % n, start, end, UP, 0.0012, 0.0007, mat)
        hook = (along * math.cos(math.radians(40.0)) + UP * math.sin(math.radians(40.0))).normalized()
        bar(lod, "hook%d" % n, end, end + hook * 0.004, UP, 0.0012, 0.0007, mat)
        bar(lod, "tape%d" % n, start, start + along * 0.014, UP, 0.0032, 0.0022, mat, top="tape", side="tape")


def visual():
    mat = kit.material("kit", TEXTURE, MATERIAL,
                       image=kit.source("sharpkit", "data", "ozg_sharpkit_co.png"),
                       normal=kit.source("sharpkit", "data", "ozg_sharpkit_nohq.png"),
                       colour=(0.3, 0.32, 0.25), rough=0.5)
    lod = kit.Lod()
    tin(lod, mat)
    ink_bottle(lod, mat)
    glue_tube(lod, mat)
    sandpaper(lod, mat)
    razor_blade(lod, mat)
    awl(lod, mat)
    file_(lod, mat)
    lead_pips(lod, mat)
    picks(lod, mat)
    lod.select("camo")
    return lod.finish("ozg_sharpkit 1", kit.VISUAL, 1)


def main():
    kit.start()
    visual()
    lo = (-LID_R, 0.0, -LID_R)
    # autocenter = 0: the origin stays where it is drawn, under the middle of the lid.
    kit.geometry("ozg_sharpkit geometry", kit.GEOMETRY, lo, (LID_R, TIN_TOP, LID_R), MASS, (("autocenter", "0"),))
    kit.geometry("ozg_sharpkit view", kit.VIEW_GEOMETRY, lo, (LID_R, TOP, LID_R))
    kit.geometry("ozg_sharpkit fire", kit.FIRE_GEOMETRY, lo, (LID_R, TIN_TOP, LID_R),
                 mat=kit.penetration_material(METAL))
    w = LID_R + 0.001
    kit.memory("ozg_sharpkit memory", (
        ("boundingbox_min", (-w, -0.001, -w)),
        ("boundingbox_max", (w, TOP + 0.001, w)),
        ("invView", INV_VIEW),
        ("ce_center", (0.0, TOP / 2.0, 0.0)),
        ("ce_radius", (0.05, 0.0, 0.0)),
    ))
    kit.save("sharpkit")
    if kit.wants_preview():
        # From where the inventory looks: in front, a little to the left, 48 degrees up.
        kit.preview("sharpkit", (0.0, 0.010, 0.0), yaw=172.0, pitch=48.0, distance=0.30)


main()
