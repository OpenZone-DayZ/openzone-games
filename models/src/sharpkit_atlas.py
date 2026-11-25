"""Where each part of the card sharp's kit lies on its one picture.

make_textures.py draws every part there and sharpkit.py maps the model's faces there, so
both read this one table. Plain Python: it runs in Blender and outside it alike.

The picture is drawn on a canvas of CANVAS x CANVAS pixels and halved into the texture.
A region is (x0, y0, x1, y1) in canvas pixels, origin at the top left; a face is mapped
EDGE pixels inside its region's border, so that a neighbour never bleeds in.
"""
CANVAS = 1024
EDGE = 3

REGIONS = {
    # The tin and the lid it stands in.
    "floor": (0, 0, 384, 384),          # the floor inside: the tin seen from above, +X to the
    #                                     right, +Z down, the middle of the tin in the middle
    "bottom": (384, 0, 640, 256),       # the underside of the lid, as "floor"
    "wall": (0, 384, 1024, 480),        # the tin's outside wall: round it along x, its top at y0
    "inner": (0, 480, 1024, 528),       # the tin's inside wall: round it along x, its top at y0
    "lid": (0, 528, 1024, 560),         # the lid's wall under the tin: round it along x, top at y0
    "rim": (0, 560, 1024, 576),         # the rolled edges: bare steel
    # The glue.
    "glue": (640, 0, 896, 256),         # the tube: round it along x, its length along y, the
    #                                     flattened end at y0, the shoulder at y1
    "crimp": (896, 0, 1024, 128),       # the flattened end: its width along x, its length along y
    "glue_cap": (896, 128, 1024, 256),  # the cap: round it along x
    # The ink.
    "ink": (384, 256, 640, 384),        # the bottle: round it along x, its foot at y1
    "ink_cap": (640, 256, 768, 384),    # the cap, round it along x, and the glass of the neck
    #                                     in its lowest quarter
    # The tools.
    "wood": (768, 256, 1024, 384),      # the awl's handle: round it along x, its length along y,
    #                                     the butt at y0
    "file": (0, 576, 1024, 640),        # the file's two cut faces: its length along x, the
    #                                     handle's end at x0
    "steel": (0, 640, 512, 704),        # plain steel: the awl's spike and ferrule, the picks,
    #                                     the file's edges
    "tape": (512, 640, 1024, 704),      # black tape round the picks' handles
    "lead": (0, 704, 128, 832),         # the lead pips
    "sand": (128, 704, 384, 960),       # a scrap of sandpaper
    "sand2": (640, 704, 896, 960),      # a second scrap, of finer and darker paper
    "blade": (384, 704, 640, 832),      # a razor blade, its long side along x
}


def uv(region, fx, fy):
    """A point of a region, (fx, fy) from 0 to 1 across it (fy downwards), as a Blender UV:
    v runs up, and the exporter turns it over for the game."""
    x0, y0, x1, y1 = REGIONS[region]
    x = x0 + EDGE + fx * (x1 - x0 - 2 * EDGE)
    y = y0 + EDGE + fy * (y1 - y0 - 2 * EDGE)
    return (x / CANVAS, 1.0 - y / CANVAS)
