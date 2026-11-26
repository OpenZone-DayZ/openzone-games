"""The stalker set's sources (boards and dice design §15, words 23 to 36): the trilogy's
figurines and the stalker draughts' faces, as the research tools left them, brought into
this folder for the Blender scripts.

    python -I models/src/chess_stalker_prep.py <research tools' out folder>

The research tools (gsc-assets\\tools under the series' research notes, not in this
repository) pose GSC Game World's models of the original trilogy, cut half their
triangles and the faces no eye above the board sees, and bake each figurine into one
material: out/figurines/set_v2000_cut/baked/ozg_chess_stalker_<face>.obj (Blender's axes,
Z up, metres, the middle of its base at the origin, facing +Y) and _co.png (1024 px).
The draughts' faces are the trilogy's portraits on the disc's cap:
out/draughts/discs/face_<role>.png (512 px).

Writes
  models/src/chess_stalker.obj                       the twelve figurines, "o <face>" each
  models/chess_stalker/data/ozg_chess_stalker_<face>_co.png   their pictures at 512 px
  models/disc/data/ozg_disc_stalker_<side>_co.png    a stalker disc: the man's face on its
                                                      top (the picture's top left), the
                                                      king's on its underside (top right),
                                                      the side's colour below for the edge

The assets are GSC Game World's (the mod's NOTICE; GSC's terms for DayZ mods: free,
marked unofficial, linking to GSC's store).
"""
import os
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
MODELS = os.path.dirname(HERE)
FACES =("wk", "wq", "wr", "wb", "wn", "wp", "bk", "bq", "br", "bb", "bn", "bp")
LANCZOS = Image.Resampling.LANCZOS
# The edge of a disc: the side's colour, as the chess bases (research draughts_discs.py).
EDGE = {"white": (219, 207, 179), "black": (9, 9, 10)}
# The white king is Wolf of Clear Sky (the canvas's tweak whiteKing: its first choice);
# Mace of Call of Pripyat is the portrait of the same name beside it.
DISCS = {"white": ("face_white_piece.png", "face_white_king_cs.png"),
         "black": ("face_black_piece.png", "face_black_king.png")}


def figurines(src):
    baked = os.path.join(src, "figurines", "set_v2000_cut", "baked")
    lines = ["# The stalker chess set: figurines posed from GSC Game World's models of the original",
             "# S.T.A.L.K.E.R. trilogy (the mod's NOTICE), each baked into one material by the",
             "# research tools. Blender's axes: X right, Y forward (the way a figurine looks), Z",
             "# up, metres; the middle of each base at the origin. models/src/chess_stalker_prep.py"]
    base_v = 0
    base_t = 0
    for face in FACES:
        verts, uvs, tris = [], [], []
        with open(os.path.join(baked, "ozg_chess_stalker_%s.obj" % face), encoding="utf-8") as f:
            for line in f:
                word, _, rest = line.strip().partition(" ")
                if word == "v":
                    verts.append(rest)
                elif word == "vt":
                    uvs.append(rest)
                elif word == "f":
                    corners = []
                    for c in rest.split():
                        parts = c.split("/")
                        corners.append((int(parts[0]), int(parts[1])))
                    tris.append(corners)
        lines.append("o %s" % face)
        lines += ["v " + v for v in verts]
        lines += ["vt " + t for t in uvs]
        for corners in tris:
            lines.append("f " + " ".join("%d/%d" % (v + base_v, t + base_t) for v, t in corners))
        base_v += len(verts)
        base_t += len(uvs)
        print("%s: %d points, %d triangles" % (face, len(verts), len(tris)))
        img = Image.open(os.path.join(baked, "ozg_chess_stalker_%s_co.png" % face)).convert("RGB")
        out = os.path.join(MODELS, "chess_stalker", "data")
        os.makedirs(out, exist_ok=True)
        img.resize((512, 512), LANCZOS).save(os.path.join(out, "ozg_chess_stalker_%s_co.png" % face))
    with open(os.path.join(HERE, "chess_stalker.obj"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    print("wrote models/src/chess_stalker.obj")


def discs(src):
    faces = os.path.join(src, "draughts", "discs")
    out = os.path.join(MODELS, "disc", "data")
    for side, (man, king) in DISCS.items():
        sheet = Image.new("RGB", (512, 512), EDGE[side])
        for i, name in enumerate((man, king)):
            face = Image.open(os.path.join(faces, name)).convert("RGB").resize((256, 256), LANCZOS)
            sheet.paste(face, (i * 256, 0))
        # a thin rim of the edge's colour round each cap, as the canvas's disc shows it
        d = ImageDraw.Draw(sheet)
        for i in range(2):
            cx = i * 256 + 128
            d.ellipse((cx - 127, 1, cx + 127, 255), outline=EDGE[side], width=3)
        sheet.save(os.path.join(out, "ozg_disc_stalker_%s_co.png" % side))
        print("wrote models/disc/data/ozg_disc_stalker_%s_co.png" % side)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    src = sys.argv[1]
    figurines(src)
    discs(src)


if __name__ == "__main__":
    main()
