/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a tool to manage Nginx, PHP and MariaDB daemons under windows.
    It's a fork of Easy WEMP originally written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Serverpack for Windows.

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

QT_BEGIN_NAMESPACE
class QAction;
class QApplication;
class QMenu;
class QSettings;
QT_END_NAMESPACE

/// Implements a tray menu with icons.
/*!
    This class creates a tray menu with icons.
    The tray menu might be used to control the daemons
    and tray menu icons show their state.
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

        // Nginx Action Slots
        void startNginx();
        void stopNginx();
        void reloadNginx();
        void restartNginx();

        // PHP Action Slots
        void startPhp();
        void stopPhp();
        void restartPhp();

        // MySQL Action Slots
        void startMariaDB();
        void stopMariaDB();
        void restartMariaDB();

        // MongoDB Action Slots
        void startMongoDB();
        void stopMongoDB();
        void restartMongoDB();

        // Memcached Action Slots
        void startMemcached();
        void stopMemcached();
        void restartMemcached();

        // Config Action Slots
        void openHostManagerDialog();
        void openConfigurationDialog();
        void openAboutDialog();

        void openNginxSite();
        void openNginxConfig();
        void openNginxLogs();

        void openPhpConfig();

        void openMariaDBClient();
        void openMariaDBConfig();

        // Status Action Slots
        void globalStateChanged();

        void nginxStateChanged(QProcess::ProcessState state);
        void phpStateChanged(QProcess::ProcessState state);
        void mariaDBStateChanged(QProcess::ProcessState state);
        void mongoDBStateChanged(QProcess::ProcessState state);
        void memcachedStateChanged(QProcess::ProcessState state);

        void nginxProcessError(QProcess::ProcessError error);
        void phpProcessError(QProcess::ProcessError error);
        void mariaDBProcessError(QProcess::ProcessError error);
        void mongoDBProcessError(QProcess::ProcessError error);
        void memcachedProcessError(QProcess::ProcessError error);

signals:
        // following signal is connected to MainWindow::setLabelStatusActive()
        void signalSetLabelStatusActive(QString label, bool enabled);
        void signalEnableToolsPushButtons(bool enabled);

private:
        QTimer* timer;

        // The settings manager.
        Settings m_settings;

        bool bAutostartDaemons;

        // Global
        QString cfgLogsDir;

        // PHP
        QString cfgPhpDir;
#define PHPCGI_EXEC "/php-cgi.exe"
        QString cfgPhpConfig;
        QString cfgPhpFastCgiHost;
        QString cfgPhpFastCgiPort;

        // NGINX
        QString cfgNginxDir;
#define NGINX_EXEC "/nginx.exe"
        QString cfgNginxConfig;
        QString cfgNginxSites;

        // MySQL
        QString cfgMariaDBDir;
#define MARIADB_EXEC "/mysqld.exe"
#define MARIADB_CLIENT_EXEC "/mysql.exe"
        QString cfgMariaDBConfig;
        QString cfgMariaDBClientExec;

        // MongoDB
        QString cfgMongoDBDir;
#define MONGODB_EXEC "/mongod.exe"

        // Memcached
        QString cfgMemcachedDir;
#define MEMCACHED_EXEC "/memcached.exe"

        // Process Monitoring
        QProcess* processNginx;
        QProcess* processPhp;
        QProcess* processMariaDB;
        QProcess* processMongoDB;
        QProcess* processMemcached;

        // The Tray Menu
        void createTrayMenu();
        QMenu* trayMenu;
         // The Submenus of the Tray Menu
        QMenu* nginxStatusSubmenu;
        QMenu* phpStatusSubmenu;
        QMenu* mariaDBStatusSubmenu;
        QMenu* mongoDBStatusSubmenu;
        QMenu* memcachedStatusSubmenu;

        void initializeConfiguration();
        void startMonitoringDaemonProcesses();

        QString getProcessErrorMessage(QProcess::ProcessError);
};

#endif // Tray_H
