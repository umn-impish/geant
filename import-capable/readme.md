# Simple way to import shapes into G4 for optical simulations

## Overview

Here we use the [CADMesh](https://github.com/christopherpoole/CADMesh/tree/master)
    library to import arbitrary 3D geometries into Geant4,
    with a focus on optical ray tracing.
We're doing this because [pvtrace](https://github.com/danieljfarrell/pvtrace/tree/master) is painfully slow.

Metadata is written in JSON and passed to `stdin` of the Geant4 executible using,
    e.g., the [`cat`](https://www.man7.org/linux/man-pages/man1/cat.1.html) utility:
```bash
cat geometry_metadata.json | ./basic-optics
```

The JSON is parsed using the [`json`](https://github.com/nlohmann/json) library listed below into C-style structs.

Some configuration data is also present in `simulation.config.file`.
This may be updated as needed.
See docs at the repository root for more info.

The current format of the JSON descriptor for each geometry element is as follows (Python notation):
```python
object_descriptor = {
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
    # 2. 
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
    "euler_rotation": [0, 0, 0]
}
```

## "Quickstart" example

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
ln -s ../scripts/rectangular .
cat rectangular/rect_meta.json | ./basic-optics
```

You should get a gui to pop up and you can run stuff interactively.

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

## Required libraries
- [Geant4]() and an appropriate Qt version
    - Read the [installation instructions](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/index.html).
    - I recommend installing from source.
    - Make sure multi-threading is enabled and Qt support is selected.
    ```bash
    # Example installation command
    # (first, download the source and cd into the G4 directory)
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX="$HOME/g4" -DGEANT4_INSTALL_DATA=ON -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_USE_QT=ON -DCMAKE_PREFIX_PATH=/usr/local/opt/qt5 ..
    make -j4
    sudo make install
    ```
- [CADMesh](https://github.com/christopherpoole/CADMesh/tree/master)
    - Can be installed like most c++ libraries:
    ```bash
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
- (if you want to run the example 3D shapes script) [CadQuery](https://github.com/CadQuery/cadquery)
    ```bash
    pip install --upgrade pip
    pip instal cadquery
    ```

