# Simple way to import geometries into G4

## Required libraries
- (if you want to run the example 3D shapes script) [CadQuery](https://github.com/CadQuery/cadquery)
    ```bash
    # This is using uv for version management;
    # you can use conda or whatever, but I prefer uv
    # uv venv --python=3.10
    # source .venv/bin/activate

    # after setting up your Python installation, do these steps
    cd scripts
    uv pip install wheel cadquery-ocp cadquery numpy==1.24.0 astropy numpy
    ```
- [Geant4]() and an appropriate Qt version
    - Read the [installation instructions](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/index.html).
    - I recommend installing from source.
    - Make sure multi-threading is enabled and Qt support is selected.
    ```bash
    # Example installation command
    # (first, download the source and cd into the G4 directory)
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX="$HOME/g4_install" -DGEANT4_INSTALL_DATA=ON -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_USE_QT=ON -DCMAKE_PREFIX_PATH=/usr/local/opt/qt5 ..
    make -jN # N = number of cores you want to use
    sudo make install
    ```
- [CADMesh](https://github.com/christopherpoole/CADMesh/tree/master) and `tetgen`
    - Can be installed like most c++ libraries:
    ```bash
    # Prereq: tetgen
    # This is the current version in Dec 2024; might change
    sudo apt update && sudo apt install libtet1.5-dev

    # Actual installation
    git clone https://github.com/christopherpoole/CADMesh.git
    cd CADMesh
    mkdir build
    cd build
    cmake ..
    make -jN # N = number of cores you want to use
    sudo make install
    ```
- [json](https://github.com/nlohmann/json)
    - Can be installed like most c++ libraries:
    ```bash
    git clone https://github.com/nlohmann/json.git
    cd json
    mkdir build
    cd build
    cmake ..
    make -jN # N = number of cores you want to use
    sudo make install
    ```


## Overview

Here we use the [CADMesh](https://github.com/christopherpoole/CADMesh/tree/master)
    library to import arbitrary 3D geometries into Geant4,
    with a focus on optical ray tracing.

We're doing this because [pvtrace](https://github.com/danieljfarrell/pvtrace/tree/master) is painfully slow.

Metadata is written in JSON and passed to the Geant4 executable along with an optional macro
```bash
./cadmesh-wrapper metadata.json path/to/optional/macro/file.mac
```

The JSON is parsed using the [`json`](https://github.com/nlohmann/json) library listed below into nice objects.

Some configuration data is also present in `simulation.config.file`.
This may be updated as needed.
See docs at the repository root for more info.

The current format of the JSON descriptor for each geometry element is as follows (Python notation):
```python
# Example Python script to write out JSON for you

import json

meta = {
    "thing1": {
        # File name relative to the current working directory
        "file": "filename.stl",

        # material can be any defined one in Geant's predefined library,
        # https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html
        # or it can be one which is defined in src/materials
        "material": "mat",

        # the type of the thing to be parsed in.
        # currently there are only some special meanings:
        # 1. specular_reflector: makes the item a perfect specular reflector
        #    (inside and out)
        # 2. scintillator: detects X-rays
        # 3. optical_detector: detects opticals
        # 4. passive: interacts with particles but is not a detector
        # 5. roughener: specifies if optical surfaces should get roughened.
        #    for this, you also need to provide an "other_volume" key to say
        #    what this particular volume should roughen up.
        "type": "type of object",

        # How much to scale the units:
        # default Geant units are in mm, so
        # if your STL file is in micron, your scale
        # should be 1000.
        "scale": 1.,

        # How much to translate the object along (x, y, z) in mm
        "translation": [0, 0, 0],

        # Euler rotation angles for the object,
        # if it should be rotated at all
        "euler_rotation": [0, 0, 0],

        # The color you'd like to use in [R, G, B, A]
        # as floats in [0, 1]. If not specified, a random
        # color is used.
        "color": [1, 0, 0, 0.1]
    },

    # Write more metadata as needed
    "thing2": {...},
    "thing3": {...},
}

# If in a .py script, write out the metadata to a file
with open('meta.json', 'w') as f:
    f.write(json.dumps(meta))
```

## Helpful Geant4 resources
It's hard to find some information on Geant4 components, so here's a short list of references I often go back to:
- [Geant4 commands documentation](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Control/AllResources/Control/UIcommands/_.html)
- [Informal G4GeneralParticleSource documentation](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/GettingStarted/generalParticleSource.html)
- Command to run a geometry overlap check: `/geometry/test/run`
- To do a big run in an interactive window, turn off visualization: `/vis/enable 0`

## "Quickstart" example
For information on `simulation.config.file`, see `docs/settings-documentation.md`.


An example CAD file set can be built by running `scripts/rectangular.py`.
The script requires [cadquery](https://github.com/CadQuery/cadquery)
    to be installed which you can do via `pip`.
Then to run this specific example, you could do:
```bash
mkdir build
cd build
cmake ..
# Update the number to your number of cores
make -j4

cd ../scripts
python rectangular.py

cd ../build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -jN # N is number of cores to use
ln -s ../scripts/rectangular .

# Before running this, modify `simulation.config.file` as you wish
./cadmesh-wrapper rectangular/rect_meta.json
```

You should get a gui to pop up and you can run stuff interactively, e.g.,
```
/control/execute optical-diagnose.mac
/run/beamOn 100
```
should launch some opticals in the crystal,
    and hits on the SiPM block.

## Further geometries
You may define further arbitrary geometries by supplying the proper
    metadata in the JSON which is passed to the executible.

Currently scintillation is not enabled but it could be for the
    appropriate materials.

Data is saved via the classes in `src/analysis*`.
If you want to save more data,
    you can add new methods to those classes,
    and other save points in the appropriate Geant action in
    `src/actions.*`.

# Resources for simulating particle populations in LEO
From [Patrick Dunn](https://www.researchgate.net/profile/Patrick-Dunn-3),
    we got some recommendations about how to simulate particle populations in LEO and perhaps elsewhere.

- For **energetic (solar-system) particles**: we can use the [SPENVIS](https://www.spenvis.oma.be/) tool
- For **other radiation stuff (dose?)**: we can use [OLTARIS](https://oltaris.larc.nasa.gov/), which is kind of like SPENVIS.
- For **galactic cosmic rays**: we can use the [Badhwar-O'Neill](https://spaceradiation.larc.nasa.gov/nasapapers/NASA-TP-2019-220419.pdf) model
