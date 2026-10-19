import argparse
import json
import tarfile

from pathlib import Path

from materials import materials


SPECIAL_TYPES = {"yap": "scintillator", "lyso": "scintillator"}
COMMON_META = {
    "scale": 1,
    "translation": [-355.6 / 2, -143.256 / 2, 157.48 / 2],
    "euler_rotation": [0, 0, 0],
}


def format_file(file: Path, outdir: Path) -> tuple[Path, str]:
    """Ensures the file ends with a newline and removes the
    material from the file name.
    """
    with open(file, "r+") as infile:
        if infile.read()[-1] != "\n":
            _ = infile.write("\n")
    material, new_name = file.name.split("_")
    new_name = new_name.replace(".STL", ".stl")
    new_path = outdir / new_name
    _ = file.copy(new_path)

    return new_path, material


def main():

    parser = argparse.ArgumentParser(
        description="Process STL files and their materials for importing into Geant.",
        epilog="Example of use: python part_prep.py -i ./raw-models -o ./payload",
    )
    _ = parser.add_argument(
        "-i",
        "--indir",
        help="input directory, contains STL files exported directly from SolidWorks; file names should have the format: material-name_part-name.STL",
        type=str,
        default="./raw-models",
    )
    _ = parser.add_argument(
        "-o",
        "--outdir",
        help="output directory, will contain the Geant-ready STL and materials metadata",
        type=str,
        default="./payload",
    )
    args = parser.parse_args()

    INDIR = Path(args.indir)
    OUTDIR = Path(args.outdir)
    if not OUTDIR.exists():
        OUTDIR.mkdir()

    parts = {}
    for file in sorted(INDIR.glob("*.STL")):
        path, material = format_file(file, OUTDIR)
        part_name = path.stem
        parts[part_name] = {
            "file": str(path),
            "type": SPECIAL_TYPES[material] if material in SPECIAL_TYPES else "passive",
            **materials[material],
            **COMMON_META,
        }

    with open(OUTDIR / "materials.json", "w", encoding="utf-8") as outfile:
        print(json.dumps(parts), file=outfile)
    with tarfile.open(f"{OUTDIR.name}.tar.xz", "w:xz") as tar:
        tar.add(OUTDIR, arcname=OUTDIR.name)


if __name__ == "__main__":
    main()
