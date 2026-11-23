"""Read what the table games probe wrote and say which tops a hologram can use.

    python -I tools/analyze.py area    <profile dir>
    python -I tools/analyze.py catalog <profile dir>

A "top" is a patch of cells that the eye or the body meets (view or geometry
kind) at one height, 0.35 to 1.25 m over the walkable surface under it, with
free space above. The question asked of each top is how much of it the
hologram's own ray (fire kind) meets at the same height.
"""

import csv
import sys
from collections import Counter, defaultdict
from pathlib import Path

FIRE, GEOM, VIEW = 0, 1, 2
RISE_MIN, RISE_MAX = 0.35, 1.25
GAP_MIN = 0.6        # free space over the top
SAME_Y = 0.08        # two kinds agree on a height within this
STEP_Y = 0.05        # neighbouring cells of one top differ by less


def rows_of(path):
    with open(path, newline="", encoding="utf-8", errors="replace") as f:
        return list(csv.DictReader(f, delimiter=";"))


def load_hits(path):
    """label -> kind -> (ix, iz) -> [(y, rise, gap, obj)]"""
    out = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))
    for r in rows_of(path):
        out[int(r["label"])][int(r["kind"])][(int(r["ix"]), int(r["iz"]))].append(
            (float(r["y"]), float(r["rise"]), float(r["gap"]), int(r["obj"]))
        )
    return out


def load_objs(path):
    out = {}
    for r in rows_of(path):
        out[(int(r["label"]), int(r["obj"]))] = r
    return out


def tops_of(kinds, pitch, min_cells, min_side_cells):
    """Clusters of physical top cells, each with its coverage by kind."""
    # A physical cell: a view or geometry hit in range with room above it.
    phys = {}
    for kind in (VIEW,):
        for cell, hits in kinds.get(kind, {}).items():
            for y, rise, gap, obj in hits:
                if not (RISE_MIN <= rise <= RISE_MAX) or gap < GAP_MIN:
                    continue
                slot = phys.setdefault(cell, [])
                for known in slot:
                    if abs(known["y"] - y) <= SAME_Y:
                        known["kinds"].add(kind)
                        break
                else:
                    slot.append({"y": y, "rise": rise, "obj": obj, "kinds": {kind}})

    # Which of them the fire ray, and the body, meet at the same height.
    fire = kinds.get(FIRE, {})
    geom = kinds.get(GEOM, {})
    for cell, slot in phys.items():
        for top in slot:
            top["fire"] = any(abs(h[0] - top["y"]) <= SAME_Y for h in fire.get(cell, []))
            if any(abs(h[0] - top["y"]) <= SAME_Y for h in geom.get(cell, [])):
                top["kinds"].add(GEOM)

    # Flood fill over neighbours at nearly the same height.
    seen = set()
    clusters = []
    for cell, slot in phys.items():
        for i, top in enumerate(slot):
            if (cell, i) in seen:
                continue
            stack = [(cell, i)]
            seen.add((cell, i))
            members = []
            while stack:
                c, k = stack.pop()
                t = phys[c][k]
                members.append((c, t))
                for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    nc = (c[0] + dx, c[1] + dz)
                    for j, nt in enumerate(phys.get(nc, [])):
                        if (nc, j) not in seen and abs(nt["y"] - t["y"]) <= STEP_Y:
                            seen.add((nc, j))
                            stack.append((nc, j))
            xs = [c[0] for c, _ in members]
            zs = [c[1] for c, _ in members]
            wx = max(xs) - min(xs) + 1
            wz = max(zs) - min(zs) + 1
            if len(members) < min_cells or min(wx, wz) < min_side_cells:
                continue
            n = len(members)
            clusters.append(
                {
                    "cells": n,
                    "area": n * pitch * pitch,
                    "wx": wx * pitch,
                    "wz": wz * pitch,
                    "rise": sum(t["rise"] for _, t in members) / n,
                    "y": sum(t["y"] for _, t in members) / n,
                    "fire": sum(1 for _, t in members if t["fire"]) / n,
                    "view": sum(1 for _, t in members if VIEW in t["kinds"]) / n,
                    "geom": sum(1 for _, t in members if GEOM in t["kinds"]) / n,
                    "obj": Counter(t["obj"] for _, t in members).most_common(1)[0][0],
                    "cx": sum(xs) / n,
                    "cz": sum(zs) / n,
                }
            )
    return clusters


def holder_name(o):
    if o["type"] and o["type"] != "UNKNOWN_P3D_FILE":
        return o["type"]
    return "p3d:" + (o["model"] or "?")


def grade(fire):
    if fire >= 0.9:
        return "caught"
    if fire >= 0.2:
        return "partly"
    return "missed"


def area(profile):
    hits = load_hits(profile / "area_hits.csv")
    objs = load_objs(profile / "area_objs.csv")
    metas = {int(r["label"]): r for r in rows_of(profile / "area_meta.csv")}

    for label, kinds in sorted(hits.items()):
        meta = metas[label]
        pitch = float(meta["pitch"])
        x0, z0 = float(meta["x0"]), float(meta["z0"])
        # 0.25 m2 and at least half a metre both ways: room for a mat.
        min_cells = max(4, round(0.25 / (pitch * pitch)))
        min_side = max(2, round(0.5 / pitch))
        tops = tops_of(kinds, pitch, min_cells, min_side)

        def kind_of(top):
            o = objs.get((label, top["obj"]))
            if top["obj"] < 0 or o is None:
                return "ground"
            if o["plant"] == "1":
                return "plant"
            if o["item"] == "1":
                return "item"
            if o["building"] == "1" or o["plain"] == "1":
                return "static"
            return "other"

        tops = [t for t in tops if kind_of(t) != "plant" and t["area"] <= 6.0]
        print(f"area {label}: x0={x0} z0={z0} pitch={pitch} "
              f"{meta['nx']}x{meta['nz']} columns, {len(tops)} tops")
        print("  by what the hologram's ray does:", dict(Counter(grade(t["fire"]) for t in tops)))
        print("  by what holds them:", dict(Counter(kind_of(t) for t in tops)))

        per_type = defaultdict(list)
        for t in tops:
            o = objs.get((label, t["obj"]))
            name = "ground"
            if o is not None:
                name = holder_name(o)
            per_type[name].append(t)
        print("  per object class (tops, caught/partly/missed, mean rise):")
        for name, ts in sorted(per_type.items(), key=lambda kv: -len(kv[1])):
            g = Counter(grade(t["fire"]) for t in ts)
            rise = sum(t["rise"] for t in ts) / len(ts)
            print(f"    {name:45s} {len(ts):3d}  {g['caught']}/{g['partly']}/{g['missed']}  {rise:.2f}")

        print("  tops (x z y rise area wx*wz fire view geom holder):")
        for t in sorted(tops, key=lambda t: -t["area"])[:60]:
            o = objs.get((label, t["obj"]))
            name = "ground" if o is None else holder_name(o)
            print(f"    {x0 + t['cx'] * pitch:8.2f} {z0 + t['cz'] * pitch:9.2f} {t['y']:7.2f} "
                  f"{t['rise']:.2f} {t['area']:5.2f} {t['wx']:.2f}*{t['wz']:.2f} "
                  f"{t['fire']:.2f} {t['view']:.2f} {t['geom']:.2f} {name}")


def catalog(profile):
    hits = load_hits(profile / "catalog_hits.csv")
    metas = {int(r["label"]): r for r in rows_of(profile / "catalog_classes.csv")}

    print("class;verdict;tops;best_rise;best_area;best_wx;best_wz;fire;view;geom")
    tally = Counter()
    for label, meta in sorted(metas.items()):
        pitch = float(meta["pitch"])
        # A mat needs about 0.4 m by 0.3 m.
        tops = tops_of(hits.get(label, {}), pitch, 12, 3)
        if not tops:
            tally["no top"] += 1
            print(f"{meta['class']};no top;0;;;;;;;")
            continue
        best = max(tops, key=lambda t: t["area"])
        verdict = grade(best["fire"])
        tally[verdict] += 1
        print(f"{meta['class']};{verdict};{len(tops)};{best['rise']:.2f};{best['area']:.2f};"
              f"{best['wx']:.2f};{best['wz']:.2f};{best['fire']:.2f};{best['view']:.2f};{best['geom']:.2f}")
    print("# tally:", dict(tally), file=sys.stderr)


def sweep(profile):
    """The live hologram's verdict per furniture class (holo_sweep.csv)."""
    import re

    rows = rows_of(profile / "holo_sweep.csv")
    tally = Counter()
    why_tally = Counter()
    print("class;verdict;rise;cells;places;r;why;drop;box;landed_off")
    for r in rows:
        name = r["class"]
        line = r["line"] or ""
        if not line.startswith("holo:"):
            tally[line or "?"] += 1
            print(f"{name};{line};{r['rise']};{r['cells']};{r['places']};{r['r']};;;;")
            continue
        refused = "refused=1" in line
        why = ""
        m = re.search(r"why:(.*?) on=", line)
        if m:
            why = " ".join(m.group(1).split())
        drop = re.search(r"drop=([\d.]+)", line)
        box = re.search(r"box=(\S+)", line)
        at = re.search(r"at=([\d.-]+) ([\d.-]+) ([\d.-]+)", line)
        off = ""
        if at and r["ax"]:
            dx = float(at.group(1)) - float(r["ax"])
            dy = float(at.group(2)) - float(r["ay"])
            dz = float(at.group(3)) - float(r["az"])
            off = f"{(dx * dx + dy * dy + dz * dz) ** 0.5:.2f}"
        verdict = "refused" if refused else "accepted"
        tally[verdict] += 1
        if refused:
            for w in why.split():
                why_tally[w] += 1
        print(f"{name};{verdict};{r['rise']};{r['cells']};{r['places']};{r['r']};{why};"
              f"{drop.group(1) if drop else ''};{box.group(1) if box else ''};{off}")
    print("# tally:", dict(tally), file=sys.stderr)
    print("# refusals by check:", dict(why_tally), file=sys.stderr)


def main():
    what, profile = sys.argv[1], Path(sys.argv[2])
    if what == "area":
        area(profile)
    elif what == "catalog":
        catalog(profile)
    elif what == "sweep":
        sweep(profile)
    else:
        raise SystemExit("area, catalog or sweep")


if __name__ == "__main__":
    main()
