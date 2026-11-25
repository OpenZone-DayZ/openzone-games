"""The pictures of the board's pieces for the table window, rendered from the models
(boards and dice design §7).

    blender -b -t 4 --python models/src/sprites.py

Opens models/src/chess_<k>.blend and disc.blend (built by chess.py and disc.py), puts the
light or the dark wood on, and renders each piece from in front and a little above, with
the same scale for all, on a clear ground: art/pieces/classic/<face>.png, 128 px square
(wK .. bP, wD, bD). tools/make_board_art.py packs them into the window's imageset.
"""
import math
import os
import sys

import bpy

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ozg_kit as kit  # noqa: E402

OUT = os.path.join(kit.REPO, "art", "pieces", "classic")
SIZE = 128
SCALE = 0.072             # metres across the picture, the same for every piece
PITCH = 14.0              # degrees the camera looks down


def light(name, energy, rot):
    data = bpy.data.lights.new(name, 'SUN')
    data.energy = energy
    data.angle = math.radians(8.0)
    ob = bpy.data.objects.new(name, data)
    ob.rotation_euler = rot
    bpy.context.scene.collection.objects.link(ob)


def render(blend, image_path, out_name, centre_z, scale, pitch, from_side=False):
    bpy.ops.wm.open_mainfile(filepath=blend)
    scene = bpy.context.scene
    for ob in bpy.data.objects:
        own = ob.a3ob_properties_object
        ob.hide_render = bool(own.is_a3_lod and own.lod != kit.VISUAL)
    img = bpy.data.images.load(image_path, check_existing=True)
    for mat in bpy.data.materials:
        node = mat.node_tree.nodes.get("picture") if mat.node_tree else None
        if node:
            node.image = img
    light("key", 3.2, (math.radians(50.0), 0.0, math.radians(-30.0)))
    light("fill", 1.3, (math.radians(70.0), 0.0, math.radians(150.0)))
    world = bpy.data.worlds.new("sky")
    scene.world = world
    back = world.node_tree.nodes.get("Background")
    if back:
        back.inputs["Color"].default_value = (0.9, 0.92, 1.0, 1.0)
        back.inputs["Strength"].default_value = 0.6
    cam_data = bpy.data.cameras.new("camera")
    cam_data.type = 'ORTHO'
    cam_data.ortho_scale = scale
    cam = bpy.data.objects.new("camera", cam_data)
    p = math.radians(pitch)
    d = 0.5
    # Blender's axes: the game's Y is Blender's Z, the game's Z Blender's Y. The knight
    # is seen from its right (+X), so its head shows its profile, the muzzle to the left.
    if from_side:
        cam.location = (d * math.cos(p), 0.0, centre_z + d * math.sin(p))
        cam.rotation_euler = (math.radians(90.0) - p, 0.0, math.radians(90.0))
    else:
        cam.location = (0.0, d * math.cos(p), centre_z + d * math.sin(p))
        cam.rotation_euler = (math.radians(90.0) - p, 0.0, math.radians(180.0))
    scene.collection.objects.link(cam)
    scene.camera = cam
    scene.render.engine = 'BLENDER_EEVEE'
    scene.eevee.taa_render_samples = 64
    scene.render.film_transparent = True
    scene.render.resolution_x = SIZE
    scene.render.resolution_y = SIZE
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = 'PNG'
    scene.render.image_settings.color_mode = 'RGBA'
    scene.view_settings.view_transform = 'Standard'
    scene.render.filepath = os.path.join(OUT, out_name + ".png")
    bpy.ops.render.render(write_still=True)
    print("sprite", scene.render.filepath)


def main():
    os.makedirs(OUT, exist_ok=True)
    src = kit.SRC
    for kind in "kqrbnp":
        for side, colour in (("w", "white"), ("b", "black")):
            render(os.path.join(src, "chess_%s.blend" % kind),
                   os.path.join(kit.MODELS, "chess", "data", "ozg_chess_%s_co.png" % colour),
                   side + kind.upper(), SCALE * 0.5 - 0.003, SCALE, PITCH, from_side=kind == "n")
    for side, colour in (("w", "white"), ("b", "black")):
        render(os.path.join(src, "disc.blend"),
               os.path.join(kit.MODELS, "disc", "data", "ozg_disc_%s_co.png" % colour),
               side + "D", 0.0035, 0.036, 55.0)


main()
