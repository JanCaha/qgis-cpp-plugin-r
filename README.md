# R Console Plugin

C++ plugin for [QGIS](https://qgis.org/) that allows running [R](https://cran.r-project.org/) directly from QGIS in form of interactive R Console (in the ways similar to QGIS's native Python Console).

The plugin is highly experimental and quite likely to break!

![](https://img.shields.io/badge/lifecycle-experimental-orange)

The plugin is based on off work done by [Nyall Dawson](https://github.com/nyalldawson) in [this branch of QGIS](https://github.com/nyalldawson/QGIS/tree/r_console), where the initial steps for the implementation were proposed.

## Install

The plugin needs to be compiled from source and the path to resulting library (in form of .so or .dll file) added to the QGIS in **Options** -> **System** -> **Plugin Paths**. Then the plugin can be activated from **Plugins** -> **Manage and Install Plugins ...**.

### Requirements

To compile the plugin following tools needs to be installed:

- cmake
- qgis (version 3.30)
- qgis-dev (header files)
- R (R CMD and RScript programs must be runnable)
- Rcpp (R package)

### Compilation

Configuration and compilation is done:

```bash
cmake -S . -B build
cmake --build build --config Release --target all --
```

The folder **plugin** then contains the file **libr_plugin.so** which needs to pointed from QGIS.

## Specific R Functions in QGIS

There is `QGIS` object with several variables and functions that help with interaction between **QGIS** and **R**.

```R
QGIS$version
QGIS$activeLayer
QGIS$mapLayers
QGIS$mapLayerByName(layerName) # layerName is string variable
QGIS$projectCrs
QGIS$dfToLayer(df) # df is data.frame (or similar class), to be converted into QGIS layer
```

The functions `QGIS$activeLayer` and `QGIS$mapLayerByName(layerName)` return `QgsMapLayerWrapper` **R** object. This object again has some functions and variables bound to id.

```R
lyr$id
lyr$featureCount
lyr$toDataFrame(includeGeometries, onlySelected) # includeGeometries, onlySelected are boolean variables, creates data.frame optionally with geometry (using sf package)
lyr$toNumericVector(fieldName, onlySelected) # fieldname is string, onlySelected is boolean
lyr$readAsSf() # reads layer as sf object based on source on disk
lyr$isVectorLayer
lyr$isRasterLayer
lyr$tableToDf # converts attribute table to R data.frame (only the table, no geometries)
lyr$toSf # converts the layer into sf data.frame (with geometry column and crs set from the layer)
```
