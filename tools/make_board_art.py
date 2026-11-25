"""The pieces of the board games as the table window draws them (boards and dice design §7).

    python -I tools/make_board_art.py [--skill DIR]

models/src/sprites.py renders every piece from its model onto a clear ground:
art/pieces/<look>/<face>.png, 128 px. This rings each with a thin line of dark wood, so
that a light piece reads on a light square, and packs them with the dayz-modding skill's
build_imageset.py (--no-paa) into art/pieces/ozg_pieces_<look>_ca.png and
OpenZone_Games/gui/imagesets/ozg_pieces_<look>.imageset. The atlas becomes the .paa beside
the .imageset through the dayz MCP's asset_convert (models/README.md).

The window's two pictures of the field are drawn by models/src/make_textures.py (board).
"""
import argparse
import os
import shutil
import subprocess
import sys
import tempfile

from PIL import Image, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
LOOKS = ("classic",)
LINE = (34, 24, 16)      # the ring: dark wood
LINE_PX = 2              # its width on a 128 px picture
LINE_ALPHA = 0.8
SKILL = os.path.join(os.path.expanduser("~"), ".claude", "skills", "dayz-modding")


def ringed(img):
    """The piece with a dark ring round its outline."""
    alpha = img.getchannel("A")
    grown = alpha.filter(ImageFilter.MaxFilter(2 * LINE_PX + 1)).filter(ImageFilter.GaussianBlur(0.6))
    ring = Image.new("RGBA", img.size, LINE + (0,))
    ring.putalpha(grown.point(lambda v: int(v * LINE_ALPHA)))
    return Image.alpha_composite(ring, img)


def pack(look, skill):
    src = os.path.join(REPO, "art", "pieces", look)
    name = "ozg_pieces_" + look
    with tempfile.TemporaryDirectory() as tmp:
        stage = os.path.join(tmp, "faces")
        os.makedirs(stage)
        faces = sorted(f for f in os.listdir(src) if f.endswith(".png"))
        if not faces:
            sys.exit("no pieces in %s: run models/src/sprites.py first" % src)
        for face in faces:
            ringed(Image.open(os.path.join(src, face)).convert("RGBA")).save(os.path.join(stage, face))
        out = os.path.join(tmp, "out")
        packer = os.path.join(skill, "scripts", "build_imageset.py")
        subprocess.run([sys.executable, "-I", packer, stage, "--name", name,
                        "--prefix", "OpenZone_Games/gui/imagesets", "--out", out, "--no-paa"], check=True)
        atlas = os.path.join(REPO, "art", "pieces", name + "_ca.png")
        imageset = os.path.join(REPO, "OpenZone_Games", "gui", "imagesets", name + ".imageset")
        shutil.copyfile(os.path.join(out, name + "_ca.png"), atlas)
        shutil.copyfile(os.path.join(out, name + ".imageset"), imageset)
    print("%s: %d pieces -> %s and %s" % (look, len(faces), os.path.relpath(atlas, REPO),
                                            os.path.relpath(imageset, REPO)))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--skill", default=os.environ.get("DAYZ_MODDING_SKILL", SKILL),
                        help="the dayz-modding skill's folder (default: %(default)s)")
    args = parser.parse_args()
    for look in LOOKS:
        pack(look, args.skill)


if __name__ == "__main__":
    main()
