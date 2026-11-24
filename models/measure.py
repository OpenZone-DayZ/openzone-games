"""What is in a model: an exported one in full, a built one as far as its header says.

    python -I models/measure.py                     every export under models/ and every
                                                    built model under OpenZone_Games/models/
    python -I models/measure.py path/to/model.p3d   these ones

So that the numbers in models/README.md can be read off the files again at any time.
Coordinates are the game's: X to the right, Y up, Z forward, metres.

AN EXPORT (MLOD) is read whole: its LODs, their triangles, size, selections, named
properties, mass and materials. The layout walked, per LOD: "P3DM", two version words,
the counts of points, normals and faces, a flag word; the points (x, y, z, flags); the
normals; the faces (a corner count, four corners of point, normal, u, v, a flag word,
two zero-ended names: texture and material); "TAGG" and the taggs (a byte, a zero-ended
name, a length, the data) up to "#EndOfFile#"; then the resolution, a float that says
what kind of LOD it is.

A BUILT MODEL (ODOL) is mostly packed, and only its header is read: the box round the
model and the bounding centre, which is how far the engine has moved the origin (0, 0, 0
when a Geometry LOD says autocenter = 0). Where those lie was read off models whose
numbers are known, of versions 55 and 56: after "ODOL", the version, the LOD count and
one float for each LOD, the box's two corners are 48 bytes on and the centre 96.
"""
import glob
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

KINDS = {1.0e13: "Geometry", 1.0e15: "Memory", 6.0e15: "View Geometry", 7.0e15: "Fire Geometry"}


def kind(resolution):
    for value, name in KINDS.items():
        if abs(resolution - value) < value * 1e-6:
            return name
    return "Resolution %g" % resolution


def text(buf, at):
    end = buf.index(b"\0", at)
    return buf[at:end].decode("ascii", "replace"), end + 1


def lods(path):
    with open(path, "rb") as fh:
        buf = fh.read()
    if buf[:4] != b"MLOD":
        sys.exit("%s is not an MLOD: it starts %r" % (path, buf[:4]))
    count = struct.unpack_from("<I", buf, 8)[0]
    at = 12
    out = []
    for _ in range(count):
        if buf[at:at + 4] != b"P3DM":
            sys.exit("%s: no P3DM at %d" % (path, at))
        n_points, n_normals, n_faces = struct.unpack_from("<3I", buf, at + 12)
        at += 28
        points = [struct.unpack_from("<3fI", buf, at + 16 * i) for i in range(n_points)]
        at += 16 * n_points + 12 * n_normals
        faces = []
        for _ in range(n_faces):
            corners = struct.unpack_from("<I", buf, at)[0]
            flags = struct.unpack_from("<I", buf, at + 68)[0]
            texture, nxt = text(buf, at + 72)
            material, nxt = text(buf, nxt)
            faces.append((corners, flags, texture, material))
            at = nxt
        at += 4                                      # "TAGG"
        taggs = []
        while True:
            name, nxt = text(buf, at + 1)
            size = struct.unpack_from("<I", buf, nxt)[0]
            data = buf[nxt + 4:nxt + 4 + size]
            at = nxt + 4 + size
            if name == "#EndOfFile#":
                break
            taggs.append((name, data))
        resolution = struct.unpack_from("<f", buf, at)[0]
        at += 4
        out.append((resolution, points, faces, taggs))
    return out


def show_built(path, buf):
    version, count = struct.unpack_from("<II", buf, 4)
    at = 12 + 4 * count
    lo = struct.unpack_from("<3f", buf, at + 48)
    hi = struct.unpack_from("<3f", buf, at + 60)
    centre = struct.unpack_from("<3f", buf, at + 96)
    print("  built, ODOL v%d, %d LODs, %d bytes" % (version, count, len(buf)))
    print("      box x %+.4f..%+.4f, y %+.4f..%+.4f, z %+.4f..%+.4f" % (lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]))
    print("      bounding centre %.4f, %.4f, %.4f" % centre)


def show(path):
    print(os.path.relpath(path, os.path.dirname(HERE)))
    with open(path, "rb") as fh:
        buf = fh.read()
    if buf[:4] == b"ODOL":
        show_built(path, buf)
        return
    for resolution, points, faces, taggs in lods(path):
        triangles = sum(corners - 2 for corners, _, _, _ in faces)
        lo = [min(p[i] for p in points) for i in range(3)]
        hi = [max(p[i] for p in points) for i in range(3)]
        print("  %-14s %3d triangles, %3d points; %.4f x %.4f x %.4f m (x %+.4f..%+.4f, y %+.4f..%+.4f, "
              "z %+.4f..%+.4f)" % (kind(resolution), triangles, len(points), hi[0] - lo[0], hi[1] - lo[1],
                                   hi[2] - lo[2], lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]))
        pictures = {}
        for _, _, texture, material in faces:
            pictures[(texture, material)] = pictures.get((texture, material), 0) + 1
        for (texture, material), n in pictures.items():
            if texture or material:
                print("      %3d faces: texture %s, material %s" % (n, texture or "none", material or "none"))
        flags = sorted({"0x%08x" % f for _, f, _, _ in faces})
        if flags != ["0x00000000"]:
            print("      face flags: %s" % ", ".join(flags))
        for name, data in taggs:
            if name == "#Property#":
                print("      property %s = %s" % (data[:64].split(b"\0")[0].decode(), data[64:].split(b"\0")[0].decode()))
            elif name == "#Mass#":
                print("      mass %.3f kg" % sum(struct.unpack("<%df" % (len(data) // 4), data)))
            elif not name.startswith("#"):
                in_points = sum(1 for b in data[:len(points)] if b)
                in_faces = sum(1 for b in data[len(points):] if b)
                print("      selection %-12s %3d of %3d faces, %3d of %3d points" % (
                    name, in_faces, len(faces), in_points, len(points)))


def main():
    paths = sys.argv[1:]
    if not paths:
        paths = sorted(glob.glob(os.path.join(HERE, "*", "*.p3d")))
        paths += sorted(glob.glob(os.path.join(os.path.dirname(HERE), "OpenZone_Games", "models", "*", "*.p3d")))
    if not paths:
        sys.exit("no model under models/ or OpenZone_Games/models/")
    for path in paths:
        show(path)


if __name__ == "__main__":
    main()
