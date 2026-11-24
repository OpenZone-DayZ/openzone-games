# Builds the game window's own fonts (art/fonts/fonts.json) into
# OpenZone_Games/gui/fonts: bitmap fonts (FNT3, a .fnt with its .edds atlas)
# made from OFL typefaces, and OFL.txt beside them so the licence travels in
# the pbo.
#
# Usage: python -I tools/build_fonts.py [--check]
#
# The converter itself is OpenZone Research's (source/fonts: fontbuild.py and
# its fontkit), read from the sibling checkout E:\openzone\openzone-research
# or from the folder in OZ_FONT_CONVERTER; it is not copied here. Its README
# says how a font's "exact text size" relates to a mock-up's CSS size. Like
# that converter, the build stops when a font lacks a Ukrainian letter, and a
# converted font may carry no Reserved Font Name of its face (PT Mono's OFL
# reserves "PT Mono": hence ozg_type).
import argparse
import json
import os
import re
import sys
import tempfile

sys.dont_write_bytecode = True
HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
FACES = os.path.join(REPO, "art", "fonts")
MANIFEST = os.path.join(FACES, "fonts.json")
CONVERTER = os.environ.get("OZ_FONT_CONVERTER",
                           os.path.join(os.path.dirname(REPO), "openzone-research", "source", "fonts"))
if not os.path.isfile(os.path.join(CONVERTER, "fontbuild.py")):
    raise SystemExit("the font converter is not at %s: check out openzone-research beside this "
                     "repository, or set OZ_FONT_CONVERTER" % CONVERTER)
sys.path.insert(0, CONVERTER)

import fontbuild  # noqa: E402
from fontkit import sfnt  # noqa: E402

NOTE = (
    "The fonts in this folder (ozg_*.fnt with their ozg_*.edds atlases) are bitmap renderings\n"
    "of the typefaces below, made by tools/build_fonts.py of the OpenZone Games repository\n"
    "with the converter of OpenZone Research. They are Modified Versions under the SIL Open\n"
    "Font License 1.1 and do not use any Reserved Font Name. The original TTF files are in the\n"
    "OpenZone Games repository, art/fonts/.\n")
LICENCE_START = "This Font Software is licensed"
RFN = re.compile(r'Reserved Font Names?((?:\s*(?:,|and)?\s*"[^"]+")+)')


def face_path(face):
    return os.path.normpath(os.path.join(FACES, face))


def read_licence(face):
    with open(os.path.join(os.path.dirname(face_path(face)), "OFL.txt"), encoding="utf-8-sig") as fh:
        text = fh.read().replace("\r\n", "\n")
    head, rest = text.split(LICENCE_START, 1)
    m = RFN.search(head)
    reserved = re.findall(r'"([^"]+)"', m.group(1)) if m else []
    return head.strip(), LICENCE_START + rest, reserved


def squash(name):
    return re.sub(r"[^0-9a-z]", "", name.lower())


def licence_text(outputs):
    faces = {}
    for o in outputs:
        faces.setdefault(o["face"], []).append(o["name"])
    parts, body = [NOTE], None
    for face in sorted(faces):
        head, licence, _ = read_licence(face)
        if body is None:
            body = licence
        family = sfnt.names(face_path(face)).get(1, face)
        parts.append("%s -- %s\n%s\n" % (", ".join(sorted(faces[face])), family, head))
    return "\n".join(parts) + "\n" + body.rstrip() + "\n"


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true", help="compare with the files on disk, write nothing")
    args = ap.parse_args(argv)
    sys.stdout.reconfigure(encoding="utf-8")       # describe() names glyphs outside the console's code page
    with open(MANIFEST, encoding="utf-8") as fh:
        m = json.load(fh)
    for o in m["outputs"]:
        if not o["name"].startswith("ozg_") or not o["name"].endswith(str(o["cell"])):
            raise SystemExit("fonts.json: %s must start with ozg_ and end with its cell" % o["name"])
        for rfn in read_licence(o["face"])[2]:
            if squash(rfn) in squash(o["name"]):
                raise SystemExit("fonts.json: %s carries the Reserved Font Name %r" % (o["name"], rfn))
    out_dir = os.path.join(REPO, *m["output"].split("/"))
    built = []
    for o in m["outputs"]:
        stem = o["name"][: -len(str(o["cell"]))]
        make = fontbuild.build_sdf if o.get("kind", "bitmap") == "sdf" else fontbuild.build
        built.append(make(face_path(o["face"]), stem, o["cell"],
                          charset=m["charsets"].get(o["charset"], o["charset"]), wght=o.get("wght")))
    work = tempfile.mkdtemp(prefix="ozg_fonts_") if args.check else out_dir
    written = []
    for b in built:
        for w in b.warnings:
            print("WARNING %s: %s" % (b.name, w))
        written += list(fontbuild.write(b, work))
        print(fontbuild.describe(b))
    lic = os.path.join(work, "OFL.txt")
    with open(lic, "w", encoding="utf-8", newline="\n") as fh:
        fh.write(licence_text(m["outputs"]))
    written.append(lic)
    if args.check:
        stale = []
        for p in written:
            mine = os.path.join(out_dir, os.path.basename(p))
            same = os.path.isfile(mine) and open(p, "rb").read() == open(mine, "rb").read()
            if not same:
                stale.append(os.path.basename(p))
            os.remove(p)
        os.rmdir(work)
        print("OUT OF DATE: " + " ".join(stale) if stale else "up to date")
        return 1 if stale else 0
    print("%d fonts and OFL.txt in %s" % (len(built), m["output"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
