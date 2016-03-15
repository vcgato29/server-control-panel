#ifndef Tray_H
#define Tray_H

#include <QSystemTrayIcon>
#include <QProcess>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QWidgetAction>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QHostAddress>

#include "version.h"
#include "servers.h"
#include "hostmanager/hostmanagerdialog.h"
#include "settings.h"

namespace ServerControlPanel
{
    /// Implements a tray menu with icons.
    /*!
        This class creates a tray menu with icons.
        The tray menu and the main window might be used to control the daemons.
        Daemon processes are monitored and their process state is displayed.
    */
    class Tray : public QSystemTrayIcon
    {
        Q_OBJECT

        public:
            explicit Tray(QApplication *parent); //, Servers::Servers *servers, Settings::SettingsManager *settings);

        public slots:
            void startAllDaemons();
            void stopAllDaemons();
            void goToWebsiteHelp();
            void goToReportIssue();
            void goToWebinterface();
            void openHostManagerDialog();
            //void openNginxSites();
            //void openNginxConfig();
            //void openPhpConfig();
            //void openMariaDbClient();
            //void openMariaDbConfig();

        private:
            void createTrayMenu();
            QList<QHostAddress> getLocalHostIPs();

            Settings::SettingsManager *settings;
            Servers::Servers          *servers;
    };
}

#endif // Tray_H
