"""The pieces of the board games as the board window draws them (boards and dice design §7,
the design canvas's boards).

    python -I tools/make_board_art.py [--skill DIR] [--stalker-from DIR] [--discs-from DIR]

classic: models/src/sprites.py renders every piece from its model onto a clear ground,
art/pieces/classic/<face>.png, 128 px. The canvas draws them as they are, with a soft
shadow under each (CSS drop-shadow 0 5px 4px, 55% black): each picture gets its shadow
beside it, `<face>_s` -- its outline in black, blurred -- which the window lays five units
lower, under the piece.

stalker: the trilogy's figurines (design §15) as the canvas draws them "at the table": from
behind (`<face>_rear`, your own) and from the front (`<face>_front`, the other side's), at
half the pictures the canvas was given (204 x 357 -> 102 x 179: a figure stands at most
110 units high in a window of 1080 rows), each with its shadow (`_s`; CSS drop-shadow
0 4px 3px, 60%). Copied into art/pieces/stalker from the research tools' renders when
--stalker-from names them.

discs_stalker: the stalker draughts (design words 30 to 32): a rookie's face on the white
disc, a bandit's on the black, and the other side of each -- an experienced stalker (Wolf
of Clear Sky, Mace of Call of Pripyat), a bandit veteran -- at 128 px, each with its shadow.
Copied into art/discs/stalker from --discs-from.

Each set is packed with the dayz-modding skill's build_imageset.py (ImageToPAA beside) into
art/pieces/ozg_<set>_ca.png, OpenZone_Games/gui/imagesets/ozg_<set>.imageset and the .paa.
"""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile

from PIL import Image, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
SKILL = os.path.join(os.path.expanduser("~"), ".claude", "skills", "dayz-modding")
LANCZOS = Image.Resampling.LANCZOS
FIGURE = (102, 179)
VIEWS = ("front", "rear")
FACES = ("wK", "wQ", "wR", "wB", "wN", "wP", "bK", "bQ", "bR", "bB", "bN", "bP")
DISCS = {"white_piece": "disc_white_piece.png", "white_king_cs": "disc_white_king_cs.png",
         "white_king_cop": "disc_white_king_cop.png", "black_piece": "disc_black_piece.png",
         "black_king": "disc_black_king.png"}


def shadow(img, sigma_px, alpha):
    """The picture's outline in black, blurred by `sigma_px`, at `alpha` -- what a CSS
    drop-shadow lays under it (without the offset, which the window adds)."""
    a = img.getchannel("A").point(lambda v: int(v * alpha))
    pad = int(sigma_px * 3) + 1
    big = Image.new("L", (img.width + 2 * pad, img.height + 2 * pad), 0)
    big.paste(a, (pad, pad))
    big = big.filter(ImageFilter.GaussianBlur(sigma_px))
    a = big.crop((pad, pad, pad + img.width, pad + img.height))
    out = Image.new("RGBA", img.size, (0, 0, 0, 0))
    out.putalpha(a)
    return out


def pack(name, stage, skill):
    """build_imageset.py over the staged pictures, the results into the repository."""
    with tempfile.TemporaryDirectory() as tmp:
        out = os.path.join(tmp, "out")
        packer = os.path.join(skill, "scripts", "build_imageset.py")
        subprocess.run([sys.executable, "-I", packer, stage, "--name", name,
                        "--prefix", "OpenZone_Games/gui/imagesets", "--out", out], check=True)
        atlas = os.path.join(REPO, "art", "pieces", name + "_ca.png")
        shutil.copyfile(os.path.join(out, name + "_ca.png"), atlas)
        shutil.copyfile(os.path.join(out, name + ".imageset"),
                        os.path.join(REPO, "OpenZone_Games", "gui", "imagesets", name + ".imageset"))
        paa = os.path.join(out, name + "_ca.paa")
        if os.path.isfile(paa):
            shutil.copyfile(paa, os.path.join(REPO, "OpenZone_Games", "gui", "imagesets", name + "_ca.paa"))
        else:
            print("  no .paa made: convert %s yourself" % atlas)
    print("%s -> art/pieces/%s_ca.png, gui/imagesets/%s.imageset" % (name, name, name))


def classic(skill):
    src = os.path.join(REPO, "art", "pieces", "classic")
    with tempfile.TemporaryDirectory() as tmp:
        for f in sorted(os.listdir(src)):
            if not f.endswith(".png"):
                continue
            img = Image.open(os.path.join(src, f)).convert("RGBA")
            img.save(os.path.join(tmp, f))
            # 2 units of blur on a 106-unit piece of 128 px
            shadow(img, 2.0 * 128 / 106, 0.55).save(os.path.join(tmp, f[:-4] + "_s.png"))
        pack("ozg_pieces_classic", tmp, skill)


def stalker(skill, take_from):
    dst = os.path.join(REPO, "art", "pieces", "stalker")
    os.makedirs(dst, exist_ok=True)
    if take_from:
        for face in FACES:
            for view in VIEWS:
                img = Image.open(os.path.join(take_from, "%s_%s.png" % (face, view))).convert("RGBA")
                img.resize(FIGURE, LANCZOS).save(os.path.join(dst, "%s_%s.png" % (face, view)))
    with tempfile.TemporaryDirectory() as tmp:
        for face in FACES:
            for view in VIEWS:
                name = "%s_%s" % (face, view)
                img = Image.open(os.path.join(dst, name + ".png")).convert("RGBA")
                img.save(os.path.join(tmp, name + ".png"))
                # 1.5 units of blur on a figure about 50 units wide drawn from 102 px
                shadow(img, 1.5 * 102 / 50, 0.6).save(os.path.join(tmp, name + "_s.png"))
        pack("ozg_pieces_stalker", tmp, skill)


def fit_script():
    """art/pieces/stalker/fit.json -- where each figurine stands inscribed in its classic
    piece, per view, in units from its square's corner (the research tools'
    shadow_fit.py) -- as OZG_StalkerFit.c, which the board window reads."""
    fit = json.load(open(os.path.join(REPO, "art", "pieces", "stalker", "fit.json"), encoding="utf-8"))
    lines = ["// GENERATED by tools/make_board_art.py from art/pieces/stalker/fit.json -- edit those.",
             "// Where each stalker figurine stands inscribed in its classic piece (boards and dice",
             "// design §15, the owner's word 36): \"<face>:<view>:x,y,w,h;\" in units from its",
             "// square's top-left corner, the view \"front\" or \"rear\".",
             "class OZG_StalkerFit",
             "{",
             "    static string Table()",
             "    {",
             '        string s = "";']
    for face in sorted(fit):
        for view in VIEWS:
            x, y, w, h = fit[face][view]
            lines.append('        s = s + "%s:%s:%g,%g,%g,%g;";' % (face, view, x, y, w, h))
    lines += ["        return s;", "    }", "}", ""]
    out = os.path.join(REPO, "OpenZone_Games", "scripts", "5_Mission", "OpenZone_Games", "OZG_StalkerFit.c")
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    print("wrote", os.path.relpath(out, REPO))


def tops(skill):
    """The nardy board's checkers, seen from above (the canvas's boards of nardy): the disc's
    top face in a circle, a little darker at its bottom edge (CSS inset 0 -2px 0, 25%
    black); classic ivory and jet from the disc model's pictures, the stalker ones with
    their faces; one round shadow for all (0 3px 4px, 60%)."""
    size = 128
    sources = {
        "classic_w": os.path.join(REPO, "models", "disc", "data", "ozg_disc_white_co.png"),
        "classic_b": os.path.join(REPO, "models", "disc", "data", "ozg_disc_black_co.png"),
        "stalker_w": (os.path.join(REPO, "models", "disc", "data", "ozg_disc_stalker_white_co.png"), 0),
        "stalker_b": (os.path.join(REPO, "models", "disc", "data", "ozg_disc_stalker_black_co.png"), 0),
    }
    ss = 4
    mask = Image.new("L", (size * ss, size * ss), 0)
    from PIL import ImageDraw
    ImageDraw.Draw(mask).ellipse((0, 0, size * ss - 1, size * ss - 1), fill=255)
    mask = mask.resize((size, size), LANCZOS)
    # the inset shade: the circle less itself moved 2/54 of the size up, darkened
    lift = int(round(size * 2 / 54.0))
    moved = Image.new("L", (size, size), 0)
    moved.paste(mask, (0, -lift))
    crescent = Image.eval(Image.merge("L", [mask]), lambda v: v)
    crescent = Image.composite(Image.new("L", (size, size), 0), crescent, moved)
    with tempfile.TemporaryDirectory() as tmp:
        for name, src in sources.items():
            if isinstance(src, tuple):
                sheet = Image.open(src[0]).convert("RGB")
                face = sheet.crop((src[1] * 256, 0, src[1] * 256 + 256, 256))
            else:
                face = Image.open(src).convert("RGB")
            face = face.resize((size, size), LANCZOS).convert("RGBA")
            shade = Image.new("RGBA", (size, size), (0, 0, 0, 0))
            shade.putalpha(crescent.point(lambda v: int(v * 0.25)))
            face = Image.alpha_composite(face, shade)
            face.putalpha(mask)
            face.save(os.path.join(tmp, "top_%s.png" % name))
        ring = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        ring.putalpha(mask)
        shadow(ring, 2.0 * size / 54, 0.6).save(os.path.join(tmp, "top_s.png"))
        pack("ozg_discs_top", tmp, skill)


def dice(skill):
    """The dice as the windows draw them (the canvas's nardy and dice boards): each face of
    each look, square on with round corners (models/src/die_looks.py writes
    art/dice/<look>_<n>.png), and one shadow (0 6px 12px, 55%)."""
    src = os.path.join(REPO, "art", "dice")
    with tempfile.TemporaryDirectory() as tmp:
        first = None
        for f in sorted(os.listdir(src)):
            if not f.endswith(".png"):
                continue
            img = Image.open(os.path.join(src, f)).convert("RGBA")
            img.save(os.path.join(tmp, f))
            first = first or img
        if first:
            shadow(first, 6.0 * first.width / 62, 0.55).save(os.path.join(tmp, "die_s.png"))
        pack("ozg_dice", tmp, skill)


def discs(skill, take_from):
    dst = os.path.join(REPO, "art", "discs", "stalker")
    os.makedirs(dst, exist_ok=True)
    if take_from:
        for role, f in DISCS.items():
            img = Image.open(os.path.join(take_from, f)).convert("RGBA")
            img.resize((128, 128), LANCZOS).save(os.path.join(dst, role + ".png"))
    with tempfile.TemporaryDirectory() as tmp:
        for role in DISCS:
            img = Image.open(os.path.join(dst, role + ".png")).convert("RGBA")
            img.save(os.path.join(tmp, role + ".png"))
            # 1.5 units of blur on a 92-unit disc of 128 px
            shadow(img, 1.5 * 128 / 92, 0.55).save(os.path.join(tmp, role + "_s.png"))
        pack("ozg_discs_stalker", tmp, skill)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--skill", default=os.environ.get("DAYZ_MODDING_SKILL", SKILL),
                        help="the dayz-modding skill's folder (default: %(default)s)")
    parser.add_argument("--stalker-from", default="", help="the figurines' renders, <face>_<view>.png")
    parser.add_argument("--discs-from", default="", help="the stalker discs' renders, disc_<role>.png")
    parser.add_argument("--only", default="", help="classic, stalker or discs")
    args = parser.parse_args()
    if args.only in ("", "classic"):
        classic(args.skill)
    if args.only in ("", "stalker"):
        stalker(args.skill, args.stalker_from)
    if args.only in ("", "stalker", "fit"):
        fit_script()
    if args.only in ("", "discs"):
        discs(args.skill, args.discs_from)
    if args.only in ("", "tops"):
        tops(args.skill)
    if args.only in ("", "dice"):
        dice(args.skill)


if __name__ == "__main__":
    main()
