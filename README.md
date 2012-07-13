## NitroShare for Mac OS X

This repository exists to make it easy for Mac users to download the project files for NitroShare (and its dependencies) and  build the entire application.

### How to Build

1. Install Qt Creator
1. Open the project file (`build.pro`)
1. Select "Release"
1. Build the project

### What's Changed

* Removed some unnecessary lines from the qxmlrpc project file.
* Replaced `#include "xmlrpc...` with `#include "qxmlrpc...` in qxmlrpc source and header files.