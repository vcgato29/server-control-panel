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

#ifndef Tray_H
#define Tray_H

// global includes
#include <QSystemTrayIcon>
#include <QProcess>
#include <QDebug>

// local includes
#include "settings.h"
#include "version.h"
#include "servers.h"

QT_BEGIN_NAMESPACE
class QAction;
class QApplication;
class QMenu;
QT_END_NAMESPACE

/// Implements a tray menu with icons.
/*!
    This class creates a tray menu with icons.
    The tray menu and the main window might be used to control the daemons.
    Daemon processes are monitored and their process state is displayed.
*/
class Tray : public QSystemTrayIcon
{
        Q_OBJECT // Enables signals and slots

public:       
       explicit Tray(QApplication *parent = 0);
        ~Tray();

public slots:

        // General Action Slots
        void startAllDaemons();
        void stopAllDaemons();
        void restartAll();

        void goToWebsiteHelp();
        void goToReportIssue();
        void goToWebinterface();

        // Config Action Slots
        void openHostManagerDialog();
        //void openConfigurationDialog();
        //void openAboutDialog();

        //void openNginxSites();
        //void openNginxConfig();
        //void openNginxLogs();

        //void openPhpConfig();

        //void openMariaDbClient();
        //void openMariaDbConfig();

        // Status Action Slots
        void updateProcessStates();

signals:
        // following signal is connected to MainWindow::setLabelStatusActive()
        void signalSetLabelStatusActive(QString label, bool enabled);
        void signalEnableToolsPushButtons(bool enabled);

private:
        QTimer* timer;

        void autostartDaemons();

        Settings *settings;
        Servers *servers;

        // Tray Menu
        void createTrayMenu();

        QMenu* trayMenu;

        void startMonitoringDaemonProcesses();
};

#endif // Tray_H
