#
#    WPN-XM Server Control Panel
#
#    WPN-XM SCP is a tool to manage Nginx, PHP and MariaDb daemons under windows.
#    It's a fork of Easy WEMP originally written by Yann Le Moigne and (c) 2010.
#    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.
#
#    This file is part of WPN-XM Serverpack for Windows.
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
DEPLOYMENT.display_name = WPN-XM Server Control Panel
# this define disables qDebug() output to console
#DEFINES += QT_NO_DEBUG_OUTPUT
# Header files
HEADERS += src/version.h \
           src/main.h \
           src/tray.h \
           src/mainwindow.h \
           src/hostmanager/host.h \
           src/hostmanager/hosttablemodel.h \
           src/hostmanager/adddialog.h \
           src/hostmanager/hostmanagerdialog.h \
           src/configurationdialog.h \
           src/settings.h \
           src/settingsTable.h
# Source files
SOURCES += src/main.cpp \
           src/tray.cpp \
           src/mainwindow.cpp \
           src/hostmanager/host.cpp \
           src/hostmanager/hosttablemodel.cpp \
           src/hostmanager/adddialog.cpp \
           src/hostmanager/hostmanagerdialog.cpp \
           src/configurationdialog.cpp \
           src/settings.cpp \
           src/settingsTable.cpp
# Resource file(s)
RESOURCES += src/resources/Resources.qrc
RC_FILE = src/resources/appico.rc
OTHER_FILES += appico.rc
FORMS += src/mainwindow.ui \
         src/configurationdialog.ui

DESTDIR = bin
release:TARGET = wpnxm-scp
# change the name of the binary, if it is build in debug mode
build_pass:CONFIG(debug, debug|release):TARGET = wpnxm-scp-debug

CONFIG += qt console warn_on static staticlib

static {                                      # everything below takes effect with CONFIG += static
    message("~~~ static build ~~~")           # this is for information, that the static build is done
    CONFIG += static
    CONFIG += staticlib                       # this is needed if you create a static library, not a static executable
    DEFINES += STATIC
    win32: TARGET = $$join(TARGET,,,_static)  # this adds an s in the end, so you can seperate static build from non static build
    QMAKE_LFLAGS *= -static -static-libgcc -enable-stdcall-fixup -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc
}
