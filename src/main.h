/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a GUI tool for managing server daemons under Windows.
    It's a fork of Easy WEMP written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Server Stack for Windows.

    WPN-XM SCP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WPN-XM SCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WPN-XM SCP. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAIN_H
#define MAIN_H

// global includes
#include <QtWidgets>

void exitIfAlreadyRunning();
void handleCommandLineArguments();
void printHelpText(const QString &errorMessage = QString());
void execDaemons(const QString &command, QCommandLineOption &clioption, QStringList args, QCommandLineParser &parser);

enum COLORS {
    BLACK = 0,
    BLUE = FOREGROUND_BLUE,
    GREEN = FOREGROUND_GREEN,
    CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
    RED = FOREGROUND_RED,
    MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
    BROWN = FOREGROUND_RED | FOREGROUND_GREEN,
    LIGHTGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    DARKGRAY = FOREGROUND_INTENSITY,
    LIGHTBLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    LIGHTGREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    LIGHTCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    LIGHTRED = FOREGROUND_RED | FOREGROUND_INTENSITY,
    LIGHTMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};

class Main : public QObject
{
    Q_OBJECT  // Enables signals and slots

public:
    explicit Main(QObject *parent = 0);
};

#endif // MAIN_H
