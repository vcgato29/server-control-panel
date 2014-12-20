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

// Local WPN-XM SCP includes
#include "tray.h"
#include "settings.h"
#include "hostmanager/hostmanagerdialog.h"
#include "servers.h"
#include "mainwindow.h"

// Global Qt includes
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
#include <QDebug>

#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkAddressEntry>
#include <QtNetwork/QHostAddress>

// Constructor
Tray::Tray(QApplication *parent, Servers *servers, Settings *settings) :
  QSystemTrayIcon(QIcon(":/wpnxm.png"), parent)
{
    this->servers = servers;
    this->settings = settings;

    createTrayMenu();
}

void Tray::createTrayMenu()
{
    QMenu *trayMenu = contextMenu();

    if (trayMenu) {
        trayMenu->clear();
    } else {
        trayMenu = new QMenu;
        setContextMenu(trayMenu);
    }

    //trayMenu->setToolTip("WPN-XM");

    // set tray menu icon
    trayMenu->setIcon(QIcon(QPixmap(":/wpnxm.png")));

    // add title entry like for WPN-XM in KVirc style (background gray, bold, small font)
    trayMenu->addAction("WPN-XM v" APP_VERSION)->setFont(QFont("Arial", 9, QFont::Bold));
    trayMenu->addSeparator();

    // Add local IPs to the tray menu
    // important note: this section needs "QT += network" in the .pro file
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(const QNetworkInterface &interface, interfaces)
    {
        if(interface.flags().testFlag(QNetworkInterface::IsUp) &&
           interface.flags().testFlag(QNetworkInterface::IsRunning) &&
          !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            foreach(const QNetworkAddressEntry &entry, interface.addressEntries()) {
                QHostAddress address = entry.ip();
                if(address.protocol() == QAbstractSocket::IPv4Protocol) {
                    QAction *ipAction = trayMenu->addAction("IP: " + address.toString());
                    ipAction->setFont(QFont("Arial", 9, QFont::Bold));
                }
            }
        }
    }

    trayMenu->addSeparator();

    // start and stop all daemons; the connection to these actions is made from mainwindow
    trayMenu->addAction(QIcon(":/action_run"), tr("Start All"), this, SLOT(startAllDaemons()), QKeySequence());
    trayMenu->addAction(QIcon(":/action_stop"), tr("Stop All"), this, SLOT(stopAllDaemons()), QKeySequence());
    trayMenu->addSeparator();

    // add all server submenus to the tray menu
    foreach(Server *server, servers->servers()) {
        trayMenu->addMenu(server->trayMenu);
        qDebug() << "[" + server->name + "] SubMenu was added to the TrayMenu.";
    }

    trayMenu->addSeparator();
    trayMenu->addAction(QIcon(":/gear"), tr("Manage Hosts"), this, SLOT(openHostManagerDialog()), QKeySequence());
    trayMenu->addAction(QIcon(":/gear"), tr("Webinterface"), this, SLOT(goToWebinterface()), QKeySequence());
    trayMenu->addSeparator();
    trayMenu->addAction(QIcon(":/report_bug"), tr("&Report Bug"), this, SLOT(goToReportIssue()), QKeySequence());
    trayMenu->addAction(QIcon(":/question"),tr("&Help"), this, SLOT(goToWebsiteHelp()), QKeySequence());
    trayMenu->addAction(QIcon(":/quit"),tr("&Quit"), qApp, SLOT(quit()), QKeySequence());
}


void Tray::goToWebinterface()
{
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/"));
}

void Tray::goToReportIssue()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/"));
}

void Tray::goToWebsiteHelp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/wiki/Using-the-Server-Control-Panel"));
}

/*
 * Action Slots
 */

void Tray::startAllDaemons()
{
    servers->startNginx();
    servers->startPHP();
    servers->startMariaDb();
    servers->startMongoDb();
    servers->startMemcached();
}

void Tray::stopAllDaemons()
{
    servers->stopMariaDb();
    servers->stopPHP();
    servers->stopNginx();
    servers->stopMongoDb();
    servers->stopMemcached();
}

void Tray::openHostManagerDialog()
{
    HostsManagerDialog dlg;
    dlg.exec();
}

/*
void Tray::openNginxSites()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(
        settings->get("paths/nginx").toString()+settings->get("nginx/sites").toString())
    );
    // start as own process ( not as a child process), will live after Tray terminates
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openNginxConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(settings->get("nginx/config").toString()));
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openMariaDbClient()
{
    QProcess::startDetached(
        settings->get("paths/mariadb").toString()+MARIADB_CLIENT_EXEC,
        QStringList(),
        settings->get("paths/mariadb").toString()
    );
}

void Tray::openMariaDbConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(settings->get("mariadb/config").toString()));
    QProcess::startDetached("cmd", QStringList() << "/c" << "start "+strDir);
}

void Tray::openPhpConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(settings->get("php/config").toString()));
    QProcess::startDetached("cmd", QStringList() << "/c" << "start "+strDir);
}*/
