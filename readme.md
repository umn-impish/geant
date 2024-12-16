# Geant4 simulations for IMPISH mission

Here are two Geant4 projects (similar, but with different goals)
    for the IMPISH mission.

See project `readme`s for more info.

## `import-capable`
A Geant4 application employing the [`CADMesh`](https://github.com/christopherpoole/CADMesh/tree/master)
    package to import arbitrary shapes.
Metadata is described with a small JSON structure per-shape,
    and each shape is imported into the geometry as a [`G4TesselatedSolid`](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html#tessellated-solids).

## `original-geant`
A playground of sorts to build up some simulations
    comparing different rectangular prism geometries.
The shapes are all coded in the Geant4 API which is not fun
    to work with.

# Resources for simulating particle populations in LEO
From [Patrick Dunn](https://www.researchgate.net/profile/Patrick-Dunn-3),
    we got some recommendations about how to simulate particle populations in LEO and perhaps elsewhere.

- For **energetic (solar-system) particles**: we can use the [SPENVIS](https://www.spenvis.oma.be/) tool
- For **other radiation stuff (dose?)**: we can use [OLTARIS](https://oltaris.larc.nasa.gov/), which is kind of like SPENVIS.
- For **galactic cosmic rays**: we can use the [Badhwar-O'Neill](https://spaceradiation.larc.nasa.gov/nasapapers/NASA-TP-2019-220419.pdf) model
