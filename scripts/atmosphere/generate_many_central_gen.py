import argparse
import os
import pathlib
import zlib

import asdf
import msgpack


def main():
    p = argparse.ArgumentParser(
        description=(
            "Generate a directory structure for use with the CentralEventGenerator. "
            "Accepts a set of momentum histograms, a starting simulation configuration file, "
            "output directory, and a path to the cadmesh wrapper Geant4 executable. "
        )
    )
    p.add_argument("--output-directory", type=str)
    p.add_argument("--momentum-hg-direc", type=str)
    p.add_argument("--config-fn", type=str)
    p.add_argument("--geant-executable", type=str)
    p.add_argument("--geometry-directory", type=str)
    args = p.parse_args()
    direc = pathlib.Path(args.output_directory).resolve()
    momentum_direc = pathlib.Path(args.momentum_hg_direc).resolve()
    config_fn = pathlib.Path(args.config_fn).resolve()
    exe = pathlib.Path(args.geant_executable).resolve()
    geom_direc = pathlib.Path(args.geometry_directory).resolve()

    # For just the IMPISH flight model
    base_generator_meta = {
        "beam_dx_mm": 14 * 25.4,
        "beam_dy_mm": 5.64 * 25.4,
        "beam_dz_mm": 6.2 * 25.4,
        "radius_mm": 200,
    }

    # Configure the global settings
    config_map = read_geant_config(config_fn)
    config_map["generator-type"] = "central-generator"
    config_map["save-prefix"] = "central-gen-matrix"

    direc.mkdir(exist_ok=True)
    config_map["data-dir"] = str((direc / "data-out").resolve())
    (direc / "data-out").mkdir(exist_ok=True)
    for file in momentum_direc.iterdir():
        print("working on", file)
        if not file.name.endswith(".asdf"):
            continue
        config_map["save-prefix"] = (pfx := f"{file.stem}-kev-matrix-row")
        config_map["central-generator-config"] = str(
            cen_gen_cfg_fn := (direc / f"{pfx}-meta.msgpk")
        )
        cur_meta = asdf.open(file)
        out = base_generator_meta | {
            # Flatten and compress the histogram data using zlib
            "momentum_hg_kev": zlib.compress(
                cur_meta["histogram"].ravel().astype("<u2"), level=6
            ),
            "num_momentum_entries": cur_meta["histogram"].size,
            "momentum_bins_kev": cur_meta["bin_edges"].tolist(),
        }
        with open(direc / cen_gen_cfg_fn, "wb") as f:
            msgpack.pack(out, f)

        write_geant_config(cfg_fn := (direc / f"{pfx}.config.file"), config_map)

        with open(direc / "macro.mac", "w") as f:
            print("/run/printProgress 10000", file=f)
            print("/run/beamOn 1000000", file=f)

        with open(script := direc / f"runme-{file.stem}", "w") as f:
            print("#!/bin/bash", file=f)
            print(
                # Assumes that everything is absolute paths (including the geometry metadata)
                f"'{exe}' '{cfg_fn!s}' '{geom_direc / 'meta.json'!s}' '{(direc / 'macro.mac')!s}'",
                file=f,
            )
        # read, write, executable by anyone
        os.chmod(str(script), 0o700)
        print("done", file)


def read_geant_config(fn: pathlib.Path) -> dict[str, object]:
    cast_map = {"string": str, "double": float, "bool": bool}
    ret = {}
    with open(fn) as f:
        for line in f:
            line = line.strip()
            if line.startswith("#"):
                continue
            if not line:
                continue
            key, type_, value = line.split()
            if type_ != "string":
                ret[key] = cast_map[type_](float(value))
            else:
                ret[key] = value
    return ret


def write_geant_config(fn: pathlib.Path, config: dict[str, object]) -> None:
    cast_map = {str: "string", float: "double", bool: "bool"}
    # Convert bools to floats, basically
    fudge_map = {str: str, float: float, bool: float}
    with open(fn, "w") as f:
        for k, v in config.items():
            print(k, cast_map[t := type(v)], fudge_map[t](v), file=f)


if __name__ == "__main__":
    main()
