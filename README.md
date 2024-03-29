WPN-XM Server Control Panel [![Build Status](https://travis-ci.org/WPN-XM/server-control-panel.svg)](https://travis-ci.org/WPN-XM/server-control-panel)
===========================

The WPN-XM Server Control Panel is written in Qt v5.2+.

![WPN-XM Server Control Panel v0.8.0](https://cloud.githubusercontent.com/assets/85608/4353472/9dfe4d10-4233-11e4-96bd-939f82b82869.jpg)

![WPN-XM Server Control Panel v0.8.0 - Main Application Window](https://cloud.githubusercontent.com/assets/85608/4353466/85a395c2-4233-11e4-9ff3-5d7d975e7396.jpg)

## Build Dependencies

* Qt >= 5.2
* QuaZIP
  * zlib

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package or just use the Qt Online Installer.

You should be able to find everything at http://www.qt.io/download-open-source/ or ftp://ftp.qt-project.org/

* Qt Online Installer

  http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe

* Qt 5.6.x

  http://download.qt.io/official_releases/qt/5.6/5.6.0/qt-opensource-windows-x86-mingw492-5.6.0.exe

* Qt Creator

  http://download.qt.io/official_releases/qtcreator/3.6/3.6.1/qt-creator-opensource-windows-x86-3.6.1.exe

### Installation Steps and Requirements for Qt

-  http://doc.qt.io/qt-5/windows-support.html
-  http://doc.qt.io/qt-5/windows-requirements.html
-  http://doc.qt.io/qt-5/index.html

#### Add Qt folder to PATH

In order to build and use Qt, the PATH environment variable needs to be extended
by adding the Qt installation folder, e.g. "c:\Qt".

* Control Panel -> System -> Advanced -> Environment variables

#### Build the Qt Library

Go to the installation folder, e.g. "c:\Qt" and run "configure" followed by "mingw32-make".

Type configure -help to get a list of all available options.
The Configuration Options for Qt page gives a brief overview of these.

See http://qt-project.org/doc/qt-4.8/configure-options.html

#### Build Qt Debug Libraries

You have to build the libraries to be able to link successfully.

* Start -> Programs -> Qt -> Qt Build Debug Libraries

## Build and Deployment Instructions

### 1 The "/build" directory must contain the Qt dependencies.

We are using the Qt command `windeployqt` to automatically detect and copy the dependencies to the debug/release folder.
You find the list of the files to copy at the end of the `.pro` file, in case you need to copy dependencies manually. 

For deployment we use our [Qt Minimal Deployment Kit](https://github.com/WPN-XM/qt-mini-deploy/). This is a packaged bundle of dependencies and allows a basic drop-in of dependencies, next to the executable.

### 2 Run environment

The run environment should include paths to Qt libraries.

### 3 Build

The easiest way to build the "WPN-XM SCP" is to build it from Qt Creator.

Launch the following commands to build it from command line:
* `qmake wpnxm-servercontrolpanel.pro`
* `mingw32-make -j4`

### How to push a new tag?

	git commit -m "This is the commit message. Tagging v1.2.3"
	git tag 1.2.3
	git push --tags
	git push

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
