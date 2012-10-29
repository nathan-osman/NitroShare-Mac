## NitroShare for Mac OS X PPC Version

This repository exists to make it easy for Mac users to download the project files for NitroShare (and its dependencies) and  build the entire application.

### How to Build

1. Make sure Xcode is installed (I'm using version 2.5)
2. Install Qt 4.6.4 ( The last version you don't need to compile from scratch, Installer runs on ppc)
3. Run `qmake` in terminal in the qxmlrpc, qjson, nitroshare subfolders, then in the main nitroshare folder.
4. Open qxmlrpc.xcodeproj, in qxmlrpc subfolder, Go to >Project >Edit Project Settings > Cross-Develop Using Target SDK: > and select "Mac OS X 10.4 (Universial)" close out the dialog, then click "Build"
5. Open qjson.xcodeproj, in qjson subfolder, Go to >Project >Edit Project Settings > Cross-Develop Using Target SDK: > and select "Mac OS X 10.4 (Universial)" close out the dialog, then click "Build"
6. Open nitroshare.xcodeproj, in nitroshare subfolder, drag `libqxmlrpc.a` and `libqjson.a` into the project under "nitroshare" on the left( The files were made on step 4 & 5, should be located above the nitroshare source directory)  Go to >Project >Edit Project Settings > Cross-Develop Using Target SDK: > and select "Mac OS X 10.4 (Universial)" close out the dialog, then click "Build and Go"

### What's Changed

* Change subfolder directorys in build.proj to be compatible, are now `SUBDIRS = /qjson /qxmlrpc /nitroshare`
* Removed code in `CBroadcastServer` and `CBasicBroadcaster` that is incompatible.
* Added `CONFIG +=  ppc` in all project files