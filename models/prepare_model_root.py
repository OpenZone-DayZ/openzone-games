"""Lay out the binarize root for the mod's models (dayz-mcp.toml, [build] project_root).

    python models/prepare_model_root.py             stage the sources
    python models/prepare_model_root.py --collect   keep the fresh exports as the sources

binarize has no root switch: the root is its working directory, and every path
baked into a model resolves against it. The models name their own textures as
OpenZone_Games\\models\\<model>\\data\\..., the shared deck sheet and its material
as OpenZone_Games\\data\\cards\\..., and their Fire Geometry materials as the
vanilla dz\\data\\data\\penetration\\*.rvmat, so the root holds all three:

    build/model-root/OpenZone_Games/models/<model>/<name>.p3d   the MLOD
    build/model-root/OpenZone_Games/models/<model>/model.cfg    copied from the mod
    build/model-root/OpenZone_Games/models/<model>/data         junction to the mod's folder
    build/model-root/OpenZone_Games/data                        junction to the mod's folder
    build/model-root/dz/data/data/penetration                   junction to the unpacked vanilla folder

A model is staged when the mod ships it: OpenZone_Games/models/<model>/model.cfg
is what says so. Its folder under the root is made even before an MLOD exists,
because that is where asset_export writes one:

    asset_export(blend="models/src/<model>.blend", mod="OpenZone_Games",
                 source="models/<model>", name="ozg_<model>.p3d")
    python models/prepare_model_root.py --collect
    asset_build(mod="OpenZone_Games", source="models/<model>")

The committed MLOD is models/<model>/<name>.p3d. Staging copies it under the
root unless a newer export already lies there; --collect copies the exports
back, so that the next clone builds the same models without Blender.

The vanilla tree is found at OZ_VANILLA_DZ, or at the first of the usual places.
Windows only: the links are NTFS junctions, which need no elevation.

Only the one vanilla folder the models name is linked, not the whole tree:
binarize walks everything under its root, links included, before it reads a
model.
"""
import _winapi
import filecmp
import glob
import os
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
MOD = "OpenZone_Games"
MOD_DIR = os.path.join(REPO, MOD)
MOD_MODELS = os.path.join(MOD_DIR, "models")
ROOT = os.path.join(REPO, "build", "model-root")
PENETRATION = os.path.join("data", "data", "penetration")
VANILLA_CANDIDATES = ["E:/pdrive/dz", "P:/dz", "D:/modding/PDrive/dz"]


def vanilla_dz():
    wanted = [os.environ.get("OZ_VANILLA_DZ", "")] + VANILLA_CANDIDATES
    for path in wanted:
        if path and os.path.isdir(os.path.join(path, PENETRATION)):
            return os.path.normpath(path)
    sys.exit("no unpacked vanilla dz with data/data/penetration found; set OZ_VANILLA_DZ")


def junction(target, link):
    if os.path.exists(link):
        return
    os.makedirs(os.path.dirname(link), exist_ok=True)
    _winapi.CreateJunction(os.path.normpath(target), os.path.normpath(link))


def shipped_models():
    """The models the mod ships: the folders that hold a model.cfg."""
    found = []
    for cfg in sorted(glob.glob(os.path.join(MOD_MODELS, "*", "model.cfg"))):
        found.append(os.path.basename(os.path.dirname(cfg)))
    return found


def is_mlod(path):
    with open(path, "rb") as fh:
        return fh.read(4) == b"MLOD"


def stage(model):
    shipped = os.path.join(MOD_MODELS, model)
    staged = os.path.join(ROOT, MOD, "models", model)
    os.makedirs(staged, exist_ok=True)
    shutil.copy2(os.path.join(shipped, "model.cfg"), staged)
    if os.path.isdir(os.path.join(shipped, "data")):
        junction(os.path.join(shipped, "data"), os.path.join(staged, "data"))
    said = []
    for mlod in sorted(glob.glob(os.path.join(HERE, model, "*.p3d"))):
        target = os.path.join(staged, os.path.basename(mlod))
        if os.path.isfile(target) and os.path.getmtime(target) > os.path.getmtime(mlod):
            if not filecmp.cmp(mlod, target, shallow=False):
                said.append("%s under the root is a newer export, left alone (--collect keeps it)"
                            % os.path.basename(mlod))
            continue
        shutil.copy2(mlod, target)
    if not glob.glob(os.path.join(staged, "*.p3d")):
        said.append("no MLOD yet: export one with asset_export")
    print("staged " + model + (" -- " + "; ".join(said) if said else ""))


def collect(model):
    staged = os.path.join(ROOT, MOD, "models", model)
    kept = []
    for export in sorted(glob.glob(os.path.join(staged, "*.p3d"))):
        if not is_mlod(export):
            sys.exit("%s is not an MLOD: the source of a model is its export, never a built one" % export)
        source_dir = os.path.join(HERE, model)
        os.makedirs(source_dir, exist_ok=True)
        target = os.path.join(source_dir, os.path.basename(export))
        if os.path.isfile(target) and filecmp.cmp(export, target, shallow=False):
            continue
        shutil.copy2(export, target)
        kept.append(os.path.basename(export))
    print("collected " + model + " -- " + (", ".join(kept) if kept else "nothing new"))


def main():
    models = shipped_models()
    if not models:
        sys.exit("no model.cfg under %s: nothing to stage" % MOD_MODELS)
    if "--collect" in sys.argv[1:]:
        for model in models:
            collect(model)
        return
    # The one vanilla folder the models name, not the whole tree.
    junction(os.path.join(vanilla_dz(), PENETRATION), os.path.join(ROOT, "dz", PENETRATION))
    # The deck sheet and its material are shared by three models.
    if os.path.isdir(os.path.join(MOD_DIR, "data")):
        junction(os.path.join(MOD_DIR, "data"), os.path.join(ROOT, MOD, "data"))
    for model in models:
        stage(model)


if __name__ == "__main__":
    main()
