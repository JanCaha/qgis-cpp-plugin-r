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
- qgis (version 3.29 Nightly builds)
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
