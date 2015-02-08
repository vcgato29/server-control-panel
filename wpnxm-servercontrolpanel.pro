#
#    WPN-XM Server Control Panel
#
#    WPN-XM SCP is a GUI tool for managing server daemons under Windows.
#    It's a fork of Easy WEMP written by Yann Le Moigne and (c) 2010.
#    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.
#
#    This file is part of WPN-XM Server Stack for Windows.
#
#    WPN-XM SCP is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    WPN-XM SCP is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with WPN-XM SCP. If not, see <http://www.gnu.org/licenses/>.
#

message("You are running qmake on wpnxm-servercontrolpanel.pro file.")

VERSION = 0.000
QMAKE_TARGET_COMPANY = company
QMAKE_TARGET_PRODUCT = product
QMAKE_TARGET_DESCRIPTION = description
QMAKE_TARGET_COPYRIGHT = copyright

DEPLOYMENT.display_name = WPN-XM Server Control Panel

CONFIG += qt console c++11 #warn-on static

QT += core network widgets

# needed for "createShellLink", see ConfigurationDialog
LIBS += -luuid -lole32

# this define disables qDebug() output to console
#DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_TRANSLATION

QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport

HEADERS += \
    src/version.h \
    src/main.h \
    src/tray.h \
    src/mainwindow.h \
    src/hostmanager/host.h \
    src/hostmanager/hosttablemodel.h \
    src/hostmanager/adddialog.h \
    src/hostmanager/hostmanagerdialog.h \
    src/configurationdialog.h \
    src/settings.h \
    src/splashscreen.h \
    src/windowsapi.h \
    src/servers.h \
    src/cli.h

SOURCES += \
    src/main.cpp \
    src/tray.cpp \
    src/mainwindow.cpp \
    src/hostmanager/host.cpp \
    src/hostmanager/hosttablemodel.cpp \
    src/hostmanager/adddialog.cpp \
    src/hostmanager/hostmanagerdialog.cpp \
    src/configurationdialog.cpp \
    src/settings.cpp \
    src/splashscreen.cpp \
    src/windowsapi.cpp \
    src/servers.cpp \
    src/cli.cpp

RESOURCES += \
    src/resources/resources.qrc

FORMS += \
    src/mainwindow.ui \
    src/configurationdialog.ui

# WINDOWS RC-FILE (sets the executable attributes)
win32: RC_FILE = src/resources/application.rc

# Build destination and binary name
CONFIG(debug, debug|release) {
    TARGET = wpn-xm-debug
} else {
    TARGET = wpn-xm
}

# if using Shadow build, you need to get the output folder
CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug

# if using normal build (non-shadow) that would have worked as well.
CONFIG(release, debug|release): DESTDIR = release
CONFIG(debug, debug|release): DESTDIR = debug

static {                                      # everything below takes effect with CONFIG += static
    message("~~~ Static Build ~~~")           # this is for information, that a static build is done

    CONFIG += static staticlib
    DEFINES += STATIC

    win32: TARGET = $$join(TARGET,,,-static)  # this appends -static to the exe, so you can seperate static build from non static build

    QMAKE_LFLAGS += -static -static-libgcc

    # https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
    QMAKE_CXXFLAGS += -O3 -std=c++11 -pedantic -Wextra -fvisibility=hidden -fvisibility-inlines-hidden -mstackrealign

    # for extra security on Windows: enable ASLR and DEP via GCC linker flags
    QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat

    QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector
}


message($$QMAKESPEC) # Determine the platform we are on

linux-g++ {
    message("Running on Linux")
}

win32-g++ {
    message("The project is build on Travis: $$(TRAVIS)")
    message("Running on Windows")
}

# Deployment - Automatically Copy Dependencies to Build Folder

win32 {

    TARGET_CUSTOM_EXT = .exe
    DEPLOY_COMMAND = windeployqt

    CONFIG( debug, debug|release ) {
        # debug
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
    } else {
        # release
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
    }

    # Uncomment the following line to help debug the deploy command when running qmake
    warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

    QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
}

# Deployment - Copy Dependencies to Build Folder

#dlls.path  =  $${DESTDIR}
#dlls.files += $$[QT_INSTALL_BINS]/icudt51.dll
#dlls.files += $$[QT_INSTALL_BINS]/icuin51.dll
#dlls.files += $$[QT_INSTALL_BINS]/icuuc51.dll
#dlls.files += $$[QT_INSTALL_BINS]/libgcc_s_dw2-1.dll
#dlls.files += $$[QT_INSTALL_BINS]/libstdc++-6.dll
#dlls.files += $$[QT_INSTALL_BINS]/libwinpthread-1.dll
#dlls.files += $$[QT_INSTALL_BINS]/Qt5Core.dll
#dlls.files += $$[QT_INSTALL_BINS]/Qt5Network.dll
#dlls.files += $$[QT_INSTALL_BINS]/Qt5Gui.dll
#dlls.files += $$[QT_INSTALL_BINS]/Qt5Widgets.dll
#dllA.path   += $${DESTDIR}/platforms
#dllA.files  += $$[QT_INSTALL_PLUGINS]/platforms/qwindows.dll
#dllB.path   += $${DESTDIR}/plugins/imageformats/
#dllB.files  += $$[QT_INSTALL_PLUGINS]/imageformats/qico.dll
#dllB.files  += $$[QT_INSTALL_PLUGINS]/imageformats/qwbmp.dll
#INSTALLS   += dlls dllA dllB
