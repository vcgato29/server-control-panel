WPN-XM Server Control Panel
===========================

The WPN-XM Server Control Panel is a Tray Application written in Qt v5.2+.

## Build Dependencies

* Qt >= 5.2

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package or just use the Qt Online Installer.

You should be able to find everything at http://qt-project.org/downloads or ftp://ftp.qt-project.org/

* Qt Online Installer

  http://download.qt-project.org/official_releases/online_installers/1.5/qt-windows-opensource-1.5.0-x86-online.exe

* Qt 5.2.0 for Windows 32-bit (MinGW 4.8, OpenGL, 689 MB)

  http://download.qt-project.org/official_releases/qt/5.2/5.2.0/qt-windows-opensource-5.2.0-mingw48_opengl-x86-offline.exe

* Qt Creator 3.0.0 for Windows (66 MB)

  http://download.qt-project.org/official_releases/qtcreator/3.0/3.0.0/qt-creator-opensource-windows-x86-3.0.0.exe

### Installation Steps and Requirements for Qt

  http://qt-project.org/doc/qt-5.0/qtdoc/install-win.html
  http://qt-project.org/doc/qt-5.0/qtdoc/requirements-win.html

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
* \plugins\imageformats\qwbmp.dll
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

For deployment use the [Qt v5.2.2 Minimal Deployment Kit](https://github.com/jakoch/qt-mini-deploy/tree/master/5.2.2).

### 2 Run environment

The run environment should include paths to Qt libraries.

### 3 Build

The easiest way to build the "WPN-XM SCP" is to build it from Qt Creator.

Launch the following commands to build it from command line:
* qmake wpnxm-servercontrolpanel.pro
* mingw32-make -j4

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
