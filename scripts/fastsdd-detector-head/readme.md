# Scripts to generate the X-123 detector head and model for use in Geant4

We tore down an Amptek X-123 FastSDD detector head and,
with some help from a caliper and documentation,
built a simplified model up using [CadQuery](https://github.com/CadQuery/cadquery),
and the model can be imported into [Geant4](https://geant4.web.cern.ch/).

## To run the scripts, starting from "scratch"
1. Download and install the `uv` tool for managing packages and Python `venv`s:
```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```
2. `cd` into a working directory and set up a virtual environment running Python 3.10; install the packages you need
```bash
# Set up the venv
uv python install 3.10
uv venv --python=3.10
source .venv/bin/activate

# Install some packages you need
uv pip install numpy==1.24.0
uv pip install cadquery numpy
```

3. Run the provided scripts to generate some .STL files and metadata that Geant needs.

```bash
python composite.py
python metadata.py
```

There should be a directory created called `x123-parts`. Move that folder to the folder which has the Geant executible. Then you can load the geometry in as such:
```bash
./cadmesh-wrapper x123-parts/fast_sdd_meta.json
```

Alternatively, you can just open the .STL files in something like [Freecad](https://www.freecad.org/) to see if everything generated correctly.