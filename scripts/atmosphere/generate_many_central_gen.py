import argparse
import json
import os
import pathlib

import asdf
import numpy as np
import numpy.typing as npt


def main():
    p = argparse.ArgumentParser(
        description=(
            "Generate a directory structure for use with the CentralEventGenerator. "
            "Accepts a set of momentum histograms, a starting simulation configuration file, "
            "output directory, and a path to the cadmesh wrapper Geant4 executable. "
        )
    )
    p.add_argument("--output-directory", type=str)
    p.add_argument("--momentum-fn", type=str)
    p.add_argument("--config-fn", type=str)
    p.add_argument("--geant-executable", type=str)
    p.add_argument("--geometry-directory", type=str)
    args = p.parse_args()
    direc = pathlib.Path(args.output_directory).resolve()
    momentum_fn = pathlib.Path(args.momentum_fn).resolve()
    config_fn = pathlib.Path(args.config_fn).resolve()
    exe = pathlib.Path(args.geant_executable).resolve()
    geom_direc = pathlib.Path(args.geometry_directory).resolve()

    # For just the IMPISH flight model
    base_generator_meta = {
        "beam_dx_mm": 110,
        "beam_dy_mm": 60,
        "beam_dz_mm": 102,
        "radius_mm": 150,
    }

    # Configure the global settings
    config_map = read_geant_config(config_fn)
    config_map["generator-type"] = "central-generator"
    config_map["save-prefix"] = "central-gen-matrix"

    direc.mkdir(exist_ok=True)
    config_map["data-dir"] = str((direc / "data-out").resolve())
    (direc / "data-out").mkdir(exist_ok=True)
    af = asdf.open(momentum_fn)
    for k in af.keys():  # noqa: SIM118
        k = str(k)
        if not k.startswith("("):
            continue
        ea, eb = (float(x) for x in k[1:-1].split(","))
        config_map["save-prefix"] = (pfx := f"{ea:.1f}-{eb:.1f}-kev-matrix-row")
        config_map["central-generator-config"] = (cen_gen_cfg_fn := f"{pfx}-meta.json")
        cur_meta = af[k]
        with open(direc / cen_gen_cfg_fn, "w") as jf:
            json.dump(
                {
                    "momentum_bins_kev": cur_meta["bin_edges"].tolist(),
                    "momentum_hg_kev": properly_weight(
                        cur_meta["momentum_histograms"]
                    ).tolist(),
                }
                | base_generator_meta,
                jf,
            )
        write_geant_config(cfg_fn := (direc / f"{pfx}.config.file"), config_map)

        with open(direc / "macro.mac", "w") as f:
            print("/run/printProgress 10000", file=f)
            print("/run/beamOn 1000000", file=f)

        with open(script := direc / f"runme-{ea:.2f}-{eb:.2f}", "w") as f:
            print("#!/bin/bash", file=f)
            print(
                # Assumes that everything is absolute paths (including the geometry metadata)
                f"'{exe}' '{cfg_fn!s}' '{geom_direc / 'meta.json'!s}' '{(direc / 'macro.mac')!s}'",
                file=f,
            )
        # read, write, executable by anyone
        os.chmod(str(script), 0o700)


def properly_weight(hg: npt.NDArray[np.int64]) -> npt.NDArray[np.uint64]:
    """Given a 4D hypercube of momentum histograms, weight and sum constituent 3D cubes
    by total counts per histogram and return the flattened cubic result."""
    total = hg.sum()
    composite = np.zeros_like(hg[0], dtype=np.uint64)
    for subh in hg:
        # Relative unnormalized probabilities;
        # we want the resulting histogram to be unisigned 64-bit integers
        rel_prob = int(1e4 * np.nan_to_num(subh.sum() / total))
        composite += rel_prob * subh.astype(np.uint64)
    return composite.ravel().astype(np.uint64)


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
            ret[key] = cast_map[type_](value)
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
