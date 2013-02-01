WPN-XM Server Control Panel
===========================

The WPN-XM Server Control Panel is a Tray Application written in Qt.

## Build Dependencies

* Qt >= 4.8

### Downloading Qt

Install Qt binaries from either the Qt SDK or standalone binary package.

You should be able to find everything at http://qt-project.org/downloads or ftp://ftp.qt-project.org/

* Qt libraries 4.8.4 for Windows (minGW 4.4, 317 MB)

  http://releases.qt-project.org/qt4/source/qt-win-opensource-4.8.4-mingw.exe

* Qt Creator 2.6.1 for Windows (52 MB)

  http://releases.qt-project.org/qtcreator/2.6.1/qt-creator-windows-opensource-2.6.1.exe

* MinGW Extension "make" for w32 (mingw32-make.exe, 1 MB)

  http://sourceforge.net/projects/mingw/files/MinGW/Extension/make/make-3.82-mingw32/

### Installation Steps for Qt

http://qt-project.org/doc/qt-4.8/install-win.html

#### Add Qt folder to PATH

In order to build and use Qt, the PATH environment variable needs to be extended
by adding the Qt installation folder, e.g. "c:\Qt-4.8.0".

* Control Panel -> System -> Advanced -> Environment variables

#### Build the Qt Library

Go to the installation folder, e.g. "c:\Qt-4.8.0" and run "configure" followed by "mingw32-make".

Type configure -help to get a list of all available options.
The Configuration Options for Qt page gives a brief overview of these.

See http://qt-project.org/doc/qt-4.8/configure-options.html

#### Build Qt Debug Libraries

You have to build the libraries to be able to link successfully.

* Start -> Programs -> Qt -> Qt Build Debug Libraries

## Build Instructions

### 1 The "/build" directory must contain the following files:

* \wpnxm.ini
* \libs\libgcc_s_dw2-1.dll
* \libs\mingwm10.dll
* \libs\QtCore4.dll
* \libs\QtGui4.dll

### 2 Run environment

The run environment should include paths to Qt libraries.

### 3 Build

* qmake wpnxm-servercontrolpanel.pro
* mingw32-make

## Bugs

If you find a bug in the software, please let us know about it.

Please post the issue to the main project via https://github.com/WPN-XM/WPN-XM/issues/new
