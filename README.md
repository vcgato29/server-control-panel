WPN-XM Server Control Panel [![Build Status](https://travis-ci.org/WPN-XM/server-control-panel.svg)](https://travis-ci.org/WPN-XM/server-control-panel)
===========================

The WPN-XM Server Control Panel is a Tray Application written in Qt v5.2+.

![WPN-XM Server Control Panel v0.8.0](https://cloud.githubusercontent.com/assets/85608/4353472/9dfe4d10-4233-11e4-96bd-939f82b82869.jpg)

![WPN-XM Server Control Panel v0.8.0 - Main Application Window](https://cloud.githubusercontent.com/assets/85608/4353466/85a395c2-4233-11e4-9ff3-5d7d975e7396.jpg)

## Build Dependencies

* Qt >= 5.2

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package or just use the Qt Online Installer.

You should be able to find everything at http://qt-project.org/downloads or ftp://ftp.qt-project.org/

* Qt Online Installer

  http://download.qt-project.org/official_releases/online_installers/

* Qt 5.4.x

  http://download.qt-project.org/official_releases/qt/5.4/5.4.0/qt-opensource-windows-x86-mingw491_opengl-5.4.0.exe

* Qt Creator

  http://download.qt-project.org/official_releases/qtcreator/3.3/3.3.0/qt-creator-opensource-windows-x86-3.3.0.exe

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

### 1 The "/build" directory must contain the following files:

* \platforms\qwindows.dll
* \plugins\imageformats\qico.dll
* ~~\plugins\imageformats\qwbmp.dll~~
* ~~\icudt51.dll~~
* ~~\icuin51.dll~~
* ~~\icuuc51.dll~~
* \libgcc_s_dw2-1.dll
* \libstdc++-6.dll
* \libwinphtread-1.dll
* \Qt5Core.dll
* \Qt5Network.dll
* \Qt5Gui.dll
* \Qt5Widgets.dll
* \wpn-xm.exe
* \wpn-xm.ini (auto-created, if not existant)

For deployment use the [Qt Minimal Deployment Kit](https://github.com/WPN-XM/qt-mini-deploy/).

### 2 Run environment

The run environment should include paths to Qt libraries.

### 3 Build

The easiest way to build the "WPN-XM SCP" is to build it from Qt Creator.

Launch the following commands to build it from command line:
* `qmake wpnxm-servercontrolpanel.pro`
* `mingw32-make -j4`

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
