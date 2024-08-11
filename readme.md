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
