"""What the model scripts share (Blender Python; the scripts import it from their own folder).

AXES. A model is described here the way it stands in the game: X to the right, Y up,
Z forward, metres. Blender has Z up, and the exporter (Arma 3 Object Builder,
io/data_p3d.py, write_vert) writes a Blender point (x, y, z) as (x, z, y). So the game's
(X, Y, Z) is Blender's (X, Z, Y): `P` below, and every point and direction handed to
this module is the game's. Nothing is mirrored by that: both describe the same thing,
they only name the axes differently.

WHAT THE EXPORTER READS (read in the add-on's source, not guessed):
    object.a3ob_properties_object        is_a3_lod, lod (the LOD type, a number as text),
                                         resolution, properties (named properties)
    object.a3ob_properties_object_flags  vertex / face flag groups; index 0 is every point's
                                         and every face's unless a layer says otherwise
    object.vertex_groups                 the selections; a face is in a selection when all
                                         of its points are
    mesh point layer "a3ob_mass"         the mass of each point, Geometry LOD only
    material.a3ob_properties_material    texture_type, texture_path / color_raw, material_path
An object with a parent is merged into that parent, so every LOD here is one object with
none. The MCP's asset_export calls the exporter with relative_paths, and a path that is
already relative to the mod's prefix goes into the model as it is written here
(utilities/generic.py, make_relative), lowercased.
"""
import math
import os
import sys

import bmesh
import bpy
from mathutils import Vector

SRC = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(SRC)
REPO = os.path.dirname(MODELS)
MOD = "OpenZone_Games"

# The LOD types of the exporter (utilities/data.py, class LOD), as the text it stores.
VISUAL = "0"
GEOMETRY = "6"
MEMORY = "9"
VIEW_GEOMETRY = "14"
FIRE_GEOMETRY = "15"
LOD_NAMES = {VISUAL: "Resolution", GEOMETRY: "Geometry", MEMORY: "Memory", VIEW_GEOMETRY: "View Geometry",
             FIRE_GEOMETRY: "Fire Geometry"}

# What a bullet meets in the Fire Geometry: the game's own materials.
PENETRATION_CLOTH = r"dz\data\data\penetration\cloth.rvmat"

# The deck sheet (512 x 512) and the three fields on it, as (u0, v0, u1, v1) with v
# running UP as Blender has it; the exporter turns it over. Each field is drawn one
# texel inside its border so that its neighbour does not bleed in.
SHEET_TEXTURE = MOD + r"\data\cards\ozg_deck_classic_co.paa"
SHEET_MATERIAL = MOD + r"\data\cards\ozg_deck.rvmat"
_T = 1.0 / 512.0
SHEET_BACK = (0.0 + _T, 0.0 + _T, 0.5 - _T, 1.0 - _T)      # the left half
SHEET_FACE = (0.5 + _T, 0.25 + _T, 1.0 - _T, 1.0 - _T)     # the right half, its top three quarters
SHEET_EDGE = (0.5 + _T, 0.0 + _T, 1.0 - _T, 0.25 - _T)     # the right half, its bottom quarter

# A playing card, and the whole deck as a stack.
CARD_WIDE = 0.063
CARD_LONG = 0.088
DECK_HIGH = 0.018


def P(v):
    """A point or a direction of the game, as Blender has it."""
    return Vector((v[0], v[2], v[1]))


def G(v):
    """A Blender point, as the game has it."""
    return (v[0], v[2], v[1])


def start():
    """An empty scene. The preferences stay: the exporter's add-on lives in them."""
    bpy.context.preferences.use_preferences_save = False
    bpy.ops.wm.read_homefile(use_empty=True)
    if not hasattr(bpy.types.Object, "a3ob_properties_object"):
        sys.exit("the Arma 3 Object Builder add-on is not enabled in this Blender: "
                 "its properties are what makes an object a LOD")


# ------------------------------------------------------------------- materials
def material(name, texture="", rvmat="", image="", normal="", colour=None, glow=0.0, rough=0.85):
    """A material of the model, and how Blender shows it.

    `texture` and `rvmat` go into the model: paths from the mod's prefix, or a procedural
    texture (it starts with '#'). The rest is for the eye only: `image` and `normal` are
    the PNG sources, `colour` stands in where there is no picture, `glow` makes it shine."""
    mat = bpy.data.materials.new(name)
    props = mat.a3ob_properties_material
    if texture.startswith("#"):
        props.texture_type = 'CUSTOM'
        props.color_raw = texture
    else:
        props.texture_type = 'TEX'
        props.texture_path = texture
    props.material_path = rvmat

    tree = mat.node_tree
    tree.nodes.clear()
    out = tree.nodes.new("ShaderNodeOutputMaterial")
    out.location = (400, 0)
    shader = tree.nodes.new("ShaderNodeBsdfPrincipled")
    shader.location = (100, 0)
    shader.inputs["Roughness"].default_value = rough
    tree.links.new(shader.outputs["BSDF"], out.inputs["Surface"])
    if colour:
        shader.inputs["Base Color"].default_value = (colour[0], colour[1], colour[2], 1.0)
    if image:
        tex = tree.nodes.new("ShaderNodeTexImage")
        tex.name = "picture"
        tex.location = (-300, 100)
        tex.image = bpy.data.images.load(image, check_existing=True)
        tree.links.new(tex.outputs["Color"], shader.inputs["Base Color"])
    if normal:
        tex = tree.nodes.new("ShaderNodeTexImage")
        tex.location = (-560, -220)
        tex.image = bpy.data.images.load(normal, check_existing=True)
        tex.image.colorspace_settings.name = "Non-Color"
        bump = tree.nodes.new("ShaderNodeNormalMap")
        bump.location = (-240, -220)
        bump.convention = 'DIRECTX'            # green is the slope down the picture, as the game reads it
        tree.links.new(tex.outputs["Color"], bump.inputs["Color"])
        tree.links.new(bump.outputs["Normal"], shader.inputs["Normal"])
    if glow:
        shader.inputs["Emission Color"].default_value = (colour[0], colour[1], colour[2], 1.0)
        shader.inputs["Emission Strength"].default_value = glow
    mat.diffuse_color = (colour[0], colour[1], colour[2], 1.0) if colour else (0.8, 0.8, 0.8, 1.0)
    return mat


def source(*parts):
    """A file under models/, or "" when it is not there."""
    path = os.path.join(MODELS, *parts)
    return path if os.path.isfile(path) else ""


def sheet_material():
    """The deck sheet, shared by the deck, the deck on the mat and the fan.

    In the model it names the classic deck's sheet; the config puts on the sheet of the
    deck's own style through a hidden selection. Blender shows the picture that sheet is
    converted from, which tools/make_cards.py draws."""
    picture = os.path.join(REPO, "art", "cards", "classic", "deck_sheet.png")
    return material("deck sheet", SHEET_TEXTURE, SHEET_MATERIAL,
                    image=picture if os.path.isfile(picture) else "",
                    normal=source("cards", "data", "ozg_deck_nohq.png"),
                    colour=(0.85, 0.82, 0.74), rough=0.6)


def penetration_material(rvmat=PENETRATION_CLOTH):
    return material("fire: " + os.path.splitext(os.path.basename(rvmat))[0], "", rvmat, colour=(0.8, 0.3, 0.2))


# ------------------------------------------------------------------------ a pose
class Pose:
    """How a thing drawn in its own frame stands in the model: turned about its origin,
    then moved. This is what there is to change when a thing sits wrong in a hand.

    `backwards` turns it half round the vertical axis before anything else. Then `lean`
    takes its top towards +X, `tilt` takes its top towards +Z, `yaw` turns it clockwise
    seen from above, in that order, in degrees; and `shift` moves it, in metres."""

    def __init__(self, yaw=0.0, tilt=0.0, lean=0.0, shift=(0.0, 0.0, 0.0), backwards=False):
        self.yaw, self.tilt, self.lean = (math.radians(a) for a in (yaw, tilt, lean))
        self.shift = tuple(shift)
        self.backwards = backwards

    def direction(self, v):
        x, y, z = v
        if self.backwards:
            x, z = -x, -z
        c, s = math.cos(self.lean), math.sin(self.lean)
        x, y = x * c + y * s, -x * s + y * c
        c, s = math.cos(self.tilt), math.sin(self.tilt)
        y, z = y * c - z * s, y * s + z * c
        c, s = math.cos(self.yaw), math.sin(self.yaw)
        x, z = x * c + z * s, -x * s + z * c
        return (x, y, z)

    def point(self, v):
        x, y, z = self.direction(v)
        return (x + self.shift[0], y + self.shift[1], z + self.shift[2])


# ------------------------------------------------------------------------ a LOD
class Lod:
    """One LOD being put together: points, faces, their pictures and their selections.

    Points belong to a PART, and a part shares its points with nobody else. That is what
    keeps a selection exact: the exporter puts a face into a selection when all of the
    face's points are in it.

    With a `pose`, everything is described in the thing's own frame and stands in the
    model as the pose says."""

    def __init__(self, pose=None):
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new("UVMap")
        self.pose = pose
        self.parts = {}
        self.materials = []
        self.selections = []

    def _point(self, part, where):
        co = P(self.pose.point(where) if self.pose else where)
        key = (round(co.x, 6), round(co.y, 6), round(co.z, 6))
        points = self.parts.setdefault(part, {})
        if key not in points:
            points[key] = self.bm.verts.new(co)
        return points[key]

    def _slot(self, mat):
        if mat is None:
            return 0
        if mat not in self.materials:
            self.materials.append(mat)
        return self.materials.index(mat)

    def point(self, part, where):
        """A point with no face on it. Nothing is drawn there; it only widens the box round the LOD."""
        self._point(part, where)

    def face(self, part, points, uvs=None, facing=(0.0, 1.0, 0.0), mat=None):
        """A flat face through `points`, turned so that it looks towards `facing`.
        (Which way round its corners go is settled here, in the thing's own frame: a pose
        turns the whole thing and changes nothing about that.)"""
        points = list(points)
        uvs = list(uvs) if uvs else [(0.0, 0.0)] * len(points)
        normal = Vector((0.0, 0.0, 0.0))
        cos = [P(p) for p in points]
        for i, a in enumerate(cos):
            b = cos[(i + 1) % len(cos)]
            normal += Vector(((a.y - b.y) * (a.z + b.z), (a.z - b.z) * (a.x + b.x), (a.x - b.x) * (a.y + b.y)))
        if normal.dot(P(facing)) < 0.0:
            points.reverse()
            uvs.reverse()
        face = self.bm.faces.new([self._point(part, p) for p in points])
        face.material_index = self._slot(mat)
        face.smooth = False
        for loop, uv in zip(face.loops, uvs):
            loop[self.uv].uv = uv
        return face

    def rect(self, part, corner, right, up, width, height, uv=None, mat=None):
        """A rectangle as somebody standing in front of it sees it: `corner` is its bottom
        left, `right` and `up` are his right and his up, and it faces him. The picture
        `uv` = (u0, v0, u1, v1) lies on it the right way round for him."""
        c, r, u = Vector(corner), Vector(right).normalized(), Vector(up).normalized()
        points = [c, c + r * width, c + r * width + u * height, c + u * height]
        u0, v0, u1, v1 = uv or (0.0, 0.0, 0.0, 0.0)
        uvs = [(u0, v0), (u1, v0), (u1, v1), (u0, v1)]
        # He stands where Blender's right x up points. The game's axes are Blender's with
        # two of them swapped, so the same product of the game's vectors points away from him.
        facing = G(P(r).cross(P(u)))
        return self.face(part, points, uvs, facing, mat)

    def box(self, part, lo, hi, mat=None):
        """A closed box between two corners. No picture on it: this is for the geometries."""
        (x0, y0, z0), (x1, y1, z1) = lo, hi
        sides = (
            ([(x0, y1, z0), (x1, y1, z0), (x1, y1, z1), (x0, y1, z1)], (0, 1, 0)),
            ([(x0, y0, z0), (x1, y0, z0), (x1, y0, z1), (x0, y0, z1)], (0, -1, 0)),
            ([(x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)], (0, 0, 1)),
            ([(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0)], (0, 0, -1)),
            ([(x1, y0, z0), (x1, y0, z1), (x1, y1, z1), (x1, y1, z0)], (1, 0, 0)),
            ([(x0, y0, z0), (x0, y0, z1), (x0, y1, z1), (x0, y1, z0)], (-1, 0, 0)),
        )
        for points, facing in sides:
            self.face(part, points, None, facing, mat)

    def stack(self, part, middle, turn_deg, mat, underside=True, back_under=False):
        """The deck as a stack of cards lying face down, CARD_WIDE along its own right and
        CARD_LONG along its own forward, DECK_HIGH tall. `middle` is the middle of its
        underside; `turn_deg` turns it clockwise seen from above. The top is the card
        back, the underside the blank face (the back too with `back_under`), the four
        sides the edge band."""
        t = math.radians(turn_deg)
        right = Vector((math.cos(t), 0.0, -math.sin(t)))
        ahead = Vector((math.sin(t), 0.0, math.cos(t)))
        up = Vector((0.0, 1.0, 0.0))
        c = Vector(middle)
        w, d = CARD_WIDE / 2.0, CARD_LONG / 2.0
        # Each face as somebody in front of it sees it: his bottom left, his right, his up.
        self.rect(part, c - right * w - ahead * d + up * DECK_HIGH, right, ahead, CARD_WIDE, CARD_LONG,
                  SHEET_BACK, mat)
        if underside:
            self.rect(part, c + right * w - ahead * d, -right, ahead, CARD_WIDE, CARD_LONG,
                      SHEET_BACK if back_under else SHEET_FACE, mat)
        self.rect(part, c - right * w - ahead * d, right, up, CARD_WIDE, DECK_HIGH, SHEET_EDGE, mat)
        self.rect(part, c + right * w + ahead * d, -right, up, CARD_WIDE, DECK_HIGH, SHEET_EDGE, mat)
        self.rect(part, c + right * w - ahead * d, ahead, up, CARD_LONG, DECK_HIGH, SHEET_EDGE, mat)
        self.rect(part, c - right * w + ahead * d, -ahead, up, CARD_LONG, DECK_HIGH, SHEET_EDGE, mat)

    def select(self, name, *parts):
        """A selection: every point of the named parts, or of all parts when none is named."""
        self.selections.append((name, parts))

    def finish(self, name, lod, resolution=0, props=(), mass=0.0, shadow=True):
        """Make the object the exporter takes for a LOD."""
        bm = self.bm
        bmesh.ops.triangulate(bm, faces=bm.faces[:])
        bm.verts.index_update()
        members = {part: [v.index for v in points.values()] for part, points in self.parts.items()}
        self.parts = None                    # a new layer moves the points: no reference is kept
        if mass:
            layer = bm.verts.layers.float.new("a3ob_mass")
            for vert in bm.verts:
                vert[layer] = mass / len(bm.verts)
        bm.normal_update()
        mesh = bpy.data.meshes.new(name)
        bm.to_mesh(mesh)
        bm.free()
        for mat in self.materials:
            mesh.materials.append(mat)
        if lod != VISUAL and len(mesh.uv_layers):
            # The exporter drops the pictures of every LOD that is not looked at.
            mesh.uv_layers.remove(mesh.uv_layers[0])

        ob = bpy.data.objects.new(name, mesh)
        bpy.context.scene.collection.objects.link(ob)
        own = ob.a3ob_properties_object
        own.is_a3_lod = True
        own.lod = lod
        own.resolution = resolution
        for key, value in props:
            item = own.properties.add()
            item.name = key
            item.value = value
        # The normals are the ones Blender shows, not ones the game works out again; and
        # the flags are said here, so that the add-on's preferences decide nothing.
        flags = ob.a3ob_properties_object_flags
        point_flag = flags.vertex.add()
        point_flag.name = "normals as exported"
        point_flag.normals = 'FIXED'
        face_flag = flags.face.add()
        face_flag.name = "casts a shadow" if shadow else "casts no shadow"
        face_flag.shadow = shadow
        for selection, parts in self.selections:
            group = ob.vertex_groups.new(name=selection)
            wanted = parts or tuple(members)
            group.add([i for part in wanted for i in members[part]], 1.0, 'REPLACE')
        return ob


def memory(name, points):
    """The Memory LOD: named points with no faces, in the MODEL's frame (no pose).

    What the game reads from it for an item (measured 2026-11-24 against the game's own
    Paper, PunchedCard, Cassette and Matchbox, and read in scripts/3_game/gameplay.c and
    5_mission/gui/inventorynew/containeditems/icon.c): the inventory draws an item's picture
    with the camera at `invView`, framed by `boundingbox_min` and `boundingbox_max`
    (view 0; `invView2` with `boundingbox_min2` / `boundingbox_max2` would be view 1, for
    attachment slots and the ground). The game's own items also carry `ce_center` and
    `ce_radius`. Without these points the camera looks along Z at the model's middle."""
    lod = Lod()
    for point_name, where in points:
        lod.point(point_name, where)
        lod.select(point_name, point_name)
    return lod.finish(name, MEMORY, 0)


def card_outline(width, height, radius, steps=4):
    """A card's outline round its middle, its corners rounded: (x, y) points in order round
    it. Which way round does not matter: face() turns a face the way it is told to look."""
    out = []
    corners = ((width / 2 - radius, height / 2 - radius, 0.0),
               (-width / 2 + radius, height / 2 - radius, 90.0),
               (-width / 2 + radius, -height / 2 + radius, 180.0),
               (width / 2 - radius, -height / 2 + radius, 270.0))
    for cx, cy, start in corners:
        for i in range(steps + 1):
            a = math.radians(start + 90.0 * i / steps)
            out.append((cx + radius * math.cos(a), cy + radius * math.sin(a)))
    return out


def geometry(name, lod, lo, hi, mass=0.0, props=(), mat=None, pose=None):
    """A LOD that is one closed convex box: the Geometry, the View Geometry, the Fire Geometry.

    The box is the selection Component01: without a component the engine finds no shape
    in the LOD. The exporter would name one itself, through operators that want a window."""
    box = Lod(pose)
    box.box("Component01", lo, hi, mat)
    box.select("Component01", "Component01")
    return box.finish(name, lod, 0, props, mass)


# ---------------------------------------------------------------- what was made
def measure(ob):
    """Triangles and the box round a LOD, in the game's axes."""
    mesh = ob.data
    lo = [min(G(v.co)[i] for v in mesh.vertices) for i in range(3)]
    hi = [max(G(v.co)[i] for v in mesh.vertices) for i in range(3)]
    tris = sum(len(p.vertices) - 2 for p in mesh.polygons)
    return tris, lo, hi


def report(model):
    print("%s: the LODs, in the game's axes (X right, Y up, Z forward)" % model)
    for ob in bpy.data.objects:
        own = ob.a3ob_properties_object
        if not own.is_a3_lod:
            continue
        tris, lo, hi = measure(ob)
        kind = LOD_NAMES.get(own.lod, own.lod)
        if own.lod == VISUAL:
            kind += " %d" % own.resolution
        print("  %-14s %3d triangles  x %+.4f..%+.4f  y %+.4f..%+.4f  z %+.4f..%+.4f  selections: %s" % (
            kind, tris, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2],
            ", ".join(g.name for g in ob.vertex_groups)))


def save(model):
    """models/src/<model>.blend, with the pictures named relative to it."""
    path = os.path.join(SRC, model + ".blend")
    for image in bpy.data.images:
        if image.filepath and not image.filepath.startswith("//"):
            image.filepath = "//" + os.path.relpath(bpy.path.abspath(image.filepath), SRC)
    if os.path.exists(path):
        os.remove(path)                      # no <model>.blend1 beside it
    bpy.ops.wm.save_as_mainfile(filepath=path, check_existing=False, compress=True)
    report(model)
    print("saved", path)


# ------------------------------------------------------------------ the preview
def wants_preview():
    args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    return "--no-preview" not in args


def preview(model, target, yaw, pitch, distance, ground=(0.30, 0.27, 0.24), floor_y=0.0, sun=3.0,
            sky=0.45, lens=70.0, size=(800, 600)):
    """Render models/preview/<model>.png. Run AFTER save(): nothing made here is kept.

    The camera looks at `target` from `distance`, `yaw` degrees round the vertical axis
    (0 is from the model's -Z side, 90 from its +X side, 180 from its +Z side) and `pitch`
    degrees up. Only the visual LODs are shown, over a plain floor at Y = `floor_y`."""
    scene = bpy.context.scene
    for ob in bpy.data.objects:
        own = ob.a3ob_properties_object
        ob.hide_render = bool(own.is_a3_lod and own.lod != VISUAL)

    floor = bpy.data.meshes.new("floor")
    bm = bmesh.new()
    bmesh.ops.create_grid(bm, x_segments=1, y_segments=1, size=20.0)
    bm.to_mesh(floor)
    bm.free()
    floor.materials.append(material("floor", colour=ground, rough=0.9))
    under = bpy.data.objects.new("floor", floor)
    under.location = P((0.0, floor_y, 0.0))
    scene.collection.objects.link(under)

    light = bpy.data.lights.new("sun", 'SUN')
    light.energy = sun
    light.angle = math.radians(4.0)
    lamp = bpy.data.objects.new("sun", light)
    lamp.rotation_euler = (math.radians(48.0), 0.0, math.radians(yaw - 35.0))   # from behind the camera's left shoulder
    scene.collection.objects.link(lamp)
    world = bpy.data.worlds.new("sky")
    scene.world = world
    back = world.node_tree.nodes.get("Background")
    if back:
        back.inputs["Color"].default_value = (0.85, 0.9, 1.0, 1.0)
        back.inputs["Strength"].default_value = sky

    cam = bpy.data.objects.new("camera", bpy.data.cameras.new("camera"))
    cam.data.lens = lens
    cam.data.clip_start = 0.01
    cam.data.clip_end = 50.0
    aim = P(target)
    y, p = math.radians(yaw), math.radians(pitch)
    cam.location = aim + Vector((math.sin(y) * math.cos(p), -math.cos(y) * math.cos(p), math.sin(p))) * distance
    cam.rotation_euler = (aim - cam.location).to_track_quat('-Z', 'Y').to_euler()
    scene.collection.objects.link(cam)
    scene.camera = cam

    scene.render.engine = 'BLENDER_EEVEE'
    scene.eevee.taa_render_samples = 64
    scene.render.resolution_x, scene.render.resolution_y = size
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGB'
    scene.render.image_settings.compression = 100
    scene.view_settings.view_transform = 'Standard'
    scene.render.filepath = os.path.join(MODELS, "preview", model + ".png")
    os.makedirs(os.path.dirname(scene.render.filepath), exist_ok=True)
    bpy.ops.render.render(write_still=True)
    print("preview", scene.render.filepath)
