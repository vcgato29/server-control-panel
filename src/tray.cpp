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
Tray::Tray(QApplication *parent) : QSystemTrayIcon(parent)
{
    // set Tray Icon
    setIcon(QIcon(":/wpnxm"));

    // @todo append status of the daemons to tooltip, e.g. "Nginx up, PHP up, MariaDB up"
    // or create seperate popup?
    setToolTip("WPN-XM");

    this->settings = new Settings;

    startMonitoringDaemonProcesses();

    createTrayMenu();

    // daemon autostart
    if(settings->get("global/autostartdaemons").toBool()) {
        autostartDaemons();
    };
}

// Destructor
Tray::~Tray()
{
    // stop all daemons, when quitting the tray application
    if(settings->get("global/stopdaemonsonquit").toBool()) {
        qDebug() << "[Stopping All Daemons on Quit]";
        stopAllDaemons();
    }
}

void Tray::startMonitoringDaemonProcesses()
{
    // the timer is used for monitoring the process state of each daemon
    timer = new QTimer(this);
    timer->setInterval(1000); // msec = 1sec

    processNginx = new QProcess();
    processNginx->setWorkingDirectory(settings->get("paths/nginx").toString());
    connect(processNginx, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(nginxStateChanged(QProcess::ProcessState)));
    connect(processNginx, SIGNAL(error(QProcess::ProcessError)), this, SLOT(nginxProcessError(QProcess::ProcessError)));

    processPhp = new QProcess();
    processPhp->setWorkingDirectory(settings->get("paths/php").toString());
    connect(processPhp, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(phpStateChanged(QProcess::ProcessState)));
    connect(processPhp, SIGNAL(error(QProcess::ProcessError)), this, SLOT(phpProcessError(QProcess::ProcessError)));

    processMariaDb = new QProcess();
    processMariaDb->setWorkingDirectory(settings->get("paths/mariadb").toString());
    connect(processMariaDb, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(mariaDbStateChanged(QProcess::ProcessState)));
    connect(processMariaDb, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mariaDbProcessError(QProcess::ProcessError)));

    processMongoDb = new QProcess();
    processMongoDb->setWorkingDirectory(settings->get("paths/mongodb").toString());
    connect(processMongoDb, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(mongoDbStateChanged(QProcess::ProcessState)));
    connect(processMongoDb, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mongoDbProcessError(QProcess::ProcessError)));

    processMemcached = new QProcess();
    processMemcached->setWorkingDirectory(settings->get("paths/memcached").toString());
    connect(processMemcached, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(memcachedStateChanged(QProcess::ProcessState)));
    connect(processMemcached, SIGNAL(error(QProcess::ProcessError)), this, SLOT(memcachedProcessError(QProcess::ProcessError)));
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

    // Nginx
    nginxStatusSubmenu = new QMenu("Nginx", trayMenu);
    nginxStatusSubmenu->setIcon(QIcon(":/status_stop"));
    nginxStatusSubmenu->addAction(QIcon(":/action_reload"), tr("Reload"), this, SLOT(reloadNginx()), QKeySequence());
    nginxStatusSubmenu->addSeparator();
    nginxStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartNginx()), QKeySequence());
    nginxStatusSubmenu->addSeparator();
    nginxStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startNginx()), QKeySequence());
    nginxStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopNginx()), QKeySequence());

    // PHP
    phpStatusSubmenu = new QMenu("PHP", trayMenu);
    phpStatusSubmenu->setIcon(QIcon(":/status_stop"));
    phpStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartPhp()), QKeySequence());
    phpStatusSubmenu->addSeparator();
    phpStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startPhp()), QKeySequence());
    phpStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopPhp()), QKeySequence());

    // MariaDB
    mariaDbStatusSubmenu = new QMenu("MariaDB", trayMenu);
    mariaDbStatusSubmenu->setIcon(QIcon(":/status_stop"));
    mariaDbStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartMariaDb()), QKeySequence());
    mariaDbStatusSubmenu->addSeparator();
    mariaDbStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startMariaDb()), QKeySequence());
    mariaDbStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopMariaDb()), QKeySequence());

    // MongoDB
    mongoDbStatusSubmenu = new QMenu("MongoDB", trayMenu);
    mongoDbStatusSubmenu->setIcon(QIcon(":/status_stop"));
    mongoDbStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartMongoDb()), QKeySequence());
    mongoDbStatusSubmenu->addSeparator();
    mongoDbStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startMongoDb()), QKeySequence());
    mongoDbStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopMongoDb()), QKeySequence());

    // Memcached
    memcachedStatusSubmenu = new QMenu("Memcached", trayMenu);
    memcachedStatusSubmenu->setIcon(QIcon(":/status_stop"));
    memcachedStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartMemcached()), QKeySequence());
    memcachedStatusSubmenu->addSeparator();
    memcachedStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startMemcached()), QKeySequence());
    memcachedStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopMemcached()), QKeySequence());

    // Add local IPs to the tray menu
    // important note: this section needs "QT += network" in the .pro file
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(const QNetworkInterface &interface, interfaces) {

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

    // Build Tray Menu

    // add title entry like for WPN-XM in KVirc style (background gray, bold, small font)
    // trayMenu->addAction("WPN-XM SCP")->setFont(QFont("Arial", 8, QFont::Bold));
    trayMenu->addAction(QIcon(":/action_run"), tr("Start All"), this, SLOT(startAllDaemons()), QKeySequence());
    trayMenu->addAction(QIcon(":/action_stop"), tr("Stop All"), this, SLOT(stopAllDaemons()), QKeySequence());
    trayMenu->addSeparator();
    trayMenu->addMenu(nginxStatusSubmenu);
    trayMenu->addMenu(phpStatusSubmenu);
    trayMenu->addMenu(mariaDbStatusSubmenu);
    trayMenu->addMenu(mongoDbStatusSubmenu);
    trayMenu->addMenu(memcachedStatusSubmenu);
    trayMenu->addSeparator();
    trayMenu->addAction(QIcon(":/gear"), tr("Manage Hosts"), this, SLOT(openHostManagerDialog()), QKeySequence());
    trayMenu->addAction(QIcon(":/gear"), tr("Webinterface"), this, SLOT(goToWebinterface()), QKeySequence());
    trayMenu->addSeparator();
    trayMenu->addAction(QIcon(":/report_bug"), tr("&Report Bug"), this, SLOT(goToReportIssue()), QKeySequence());
    trayMenu->addAction(QIcon(":/question"),tr("&Help"), this, SLOT(goToWebsiteHelp()), QKeySequence());
    trayMenu->addAction(QIcon(":/quit"),tr("&Quit"), parent(), SLOT(quit()), QKeySequence());
}

void Tray::goToWebinterface()
{
    QDesktopServices::openUrl(QUrl("http://localhost/webinterface/"));
}

void Tray::goToReportIssue()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/"));
}

void Tray::goToWebsiteHelp()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/wiki/Using-the-Server-Control-Panel"));
}

void Tray::autostartDaemons()
{
    if(settings->get("autostart/nginx").toBool()) startNginx();
    if(settings->get("autostart/php").toBool()) startPhp();
    if(settings->get("autostart/mariadb").toBool()) startMariaDb();
    if(settings->get("autostart/mongodb").toBool()) startMongoDb();
    if(settings->get("autostart/memcached").toBool()) startMemcached();
}

//*
//* Action slots
//*
void Tray::startAllDaemons()
{
    startNginx();
    startPhp();
    startMariaDb();
    startMongoDb();
    startMemcached();
}

void Tray::stopAllDaemons()
{
    stopMariaDb();
    stopPhp();
    stopNginx();
    stopMongoDb();
    stopMemcached();
}

void Tray::restartAll()
{
    restartNginx();
    restartPhp();
    restartMariaDb();
    restartMongoDb();
    restartMemcached();
}

/*
 * Nginx - Actions: run, stop, restart
 */
void Tray::startNginx()
{
    // already running
    if(processNginx->state() != QProcess::NotRunning)
    {
        QMessageBox::warning(0, tr("Nginx"), tr("Nginx already running."));
        return;
    }

    // start daemon
    QString const startNginx = settings->get("path/nginx").toString() + NGINX_EXEC
            + " -p " + QDir::currentPath()
            + " -c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf";

    qDebug() << "Starting Nginx" << startNginx;

    processNginx->start(startNginx);
}

void Tray::stopNginx()
{
    QProcess processStopNginx;
    processStopNginx.setWorkingDirectory(settings->get("path/nginx").toString());

    // fast shutdown
    QString stopNginx = settings->get("path/nginx").toString() + NGINX_EXEC
            + " -p " + QDir::currentPath()
            + " -c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf"
            + " -s stop";

    qDebug() << "Stopping Nginx" << stopNginx;

    processStopNginx.start(stopNginx);
    processStopNginx.waitForFinished();
}

void Tray::reloadNginx()
{
    QString cfgNginxDir = settings->get("path/nginx").toString();

    QProcess processStopNginx;
    processStopNginx.setWorkingDirectory(cfgNginxDir);

    QString const reloadNginx = cfgNginxDir + NGINX_EXEC
            + " -p " + QDir::currentPath()
            + " -c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf"
            + "-s reload";

    qDebug() << reloadNginx;

    processStopNginx.start(reloadNginx);
    processStopNginx.waitForFinished();
}

void Tray::restartNginx()
{
    stopNginx();
    startNginx();
}

/*
 * PHP - Actions: run, stop, restart
 */
void Tray::startPhp()
{
    // already running
    if(processPhp->state() != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("PHP"), tr("PHP is already running."));
        return;
    }

    // start daemon
    QString const startPHP = settings->get("path/php").toString()+PHPCGI_EXEC
            + " -b " + settings->get("php/fastcgi-host").toString()+":"+settings->get("php/fastcgi-port").toString();

    qDebug() << "Starting PHP" << startPHP;

    processPhp->start(startPHP);
}

void Tray::stopPhp()
{
    // 1) processPhp->terminate(); will fail because WM_CLOSE message not handled
    // 2) By killing the process, we are crashing it!
    //    The user will then get a "Process Crashed" Error MessageBox.
    //    Therefore we need to disconnect signal/sender from method/receiver.
    //    The result is, that crashing the php daemon intentionally is not shown as error.
    disconnect(processPhp, SIGNAL(error(QProcess::ProcessError)), this, SLOT(phpProcessError(QProcess::ProcessError)));

    // kill PHP daemon
    processPhp->kill();
    processPhp->waitForFinished();
}

void Tray::restartPhp()
{
    stopPhp();
    startPhp();
}

/*
 * MariaDb Actions - run, stop, restart
 */
void Tray::startMariaDb()
{
    // already running
    if(processMariaDb->state() != QProcess::NotRunning){
        QMessageBox::warning(0, tr("MariaDB"), tr("MariaDB already running."));
        return;
    }

    // start
    QString const startMariaDb = settings->get("path/mariadb").toString() + MARIADB_EXEC;
    qDebug() << "Starting MariaDb" << startMariaDb;
    processMariaDb->start(startMariaDb);
}

void Tray::stopMariaDb()
{
    // disconnect process monitoring, before crashing the process
    disconnect(processMariaDb, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mariaDbProcessError(QProcess::ProcessError)));

    processMariaDb->kill();
    processMariaDb->waitForFinished();
}

void Tray::restartMariaDb()
{
    stopMariaDb();
    startMariaDb();
}

/*
 * MongoDB Actions - run, stop, restart
 */
void Tray::startMongoDb()
{
    // already running
    if(processMongoDb->state() != QProcess::NotRunning){
        QMessageBox::warning(0, tr("MongoDB"), tr("MongoDB already running."));
        return;
    }

    // mongodb doesn't start, when data dir is missing...
    QString const mongoDbDataDir = qApp->applicationDirPath() + "/bin/mongodb/data/db";
    if(QDir().exists(qApp->applicationDirPath() + "/bin/mongodb") && !QDir().exists(mongoDbDataDir)) {
        qDebug() << "Creating Directory for Mongo's Database... " << mongoDbDataDir;
        QDir().mkpath(mongoDbDataDir);
    }

    // mongodb doesn't start, when logfile is missing...
    QFile f(qApp->applicationDirPath() + "/logs/mongodb.log");
    if(!f.exists()) {
        qDebug() << "Creating empty logfile... " << qApp->applicationDirPath() + "/logs/mongodb.log";
        f.open(QIODevice::ReadWrite);
        f.close();
    }

    // build mongo start command
    QString const mongoStartCommand = settings->get("paths/mongodb").toString()+MONGODB_EXEC
             + " --config " + qApp->applicationDirPath() + "/bin/mongodb/mongodb.conf"
             + " --dbpath " + qApp->applicationDirPath() + "/bin/mongodb/data/db"
             + " --logpath " + qApp->applicationDirPath() + "/logs/mongodb.log";

    qDebug() << mongoStartCommand;

    // start
    processMongoDb->start(mongoStartCommand);
}

void Tray::stopMongoDb()
{
    // build mongo stop command
    QString const mongoStopCommand = settings->get("paths/mongodb").toString() + "/mongo.exe"
             + " --eval \"db.getSiblingDB('admin').shutdownServer()\"";

    qDebug() << "Shutting down MongoDb...\n" << mongoStopCommand;

    if(QProcess::execute(mongoStopCommand))
    {
        // disconnect process monitoring, if shutdown command successfully send
        disconnect(processMongoDb, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mongoDbProcessError(QProcess::ProcessError)));
    }
}

void Tray::restartMongoDb()
{
    stopMongoDb();
    startMongoDb();
}

/*
 * Memcached Actions - run, stop, restart
 */
void Tray::startMemcached()
{
    // already running
    if(processMemcached->state() != QProcess::NotRunning){
        QMessageBox::warning(0, tr("Memcached"), tr("Memcached already running."));
        return;
    }

    // start
    qDebug() << "Starting Memcached...\n" << settings->get("paths/memcached").toString()+MEMCACHED_EXEC;

    processMemcached->start(settings->get("paths/memcached").toString()+MEMCACHED_EXEC);
}

void Tray::stopMemcached()
{
    // disconnect process monitoring, before crashing the process
    disconnect(processMemcached, SIGNAL(error(QProcess::ProcessError)), this, SLOT(memcachedProcessError(QProcess::ProcessError)));

    qDebug() << "Stopping Memcached...\n";

    processMemcached->kill();
    processMemcached->waitForFinished();
}

void Tray::restartMemcached()
{
    stopMemcached();
    startMemcached();
}

/*
 * Config slots
 */
void Tray::openHostManagerDialog()
{
    HostsManagerDialog dlg;
    dlg.exec();
}

void Tray::openAboutDialog()
{
    //AboutDialog dlg;
    //dlg.exec();
}

void Tray::openConfigurationDialog()
{
    //ConfigDialog dlg;
    //dlg.exec();
}

void Tray::openNginxSite()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(
        settings->get("path/nginx").toString()+settings->get("nginx/sites").toString())
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

void Tray::openNginxLogs()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(settings->get("paths/logs").toString()));
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openMariaDbClient()
{
    QProcess::startDetached(
        settings->get("path/mariadb").toString()+MARIADB_CLIENT_EXEC,
        QStringList(),
        settings->get("path/mariadb").toString()
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
}

/*
 * State slots
 */
void Tray::globalStateChanged()
{
    QProcess::ProcessState stateNginx = QProcess::Running;
    QProcess::ProcessState statePhp = QProcess::Running;
    QProcess::ProcessState stateMariaDb = QProcess::Running;
    QProcess::ProcessState stateMongoDb = QProcess::Running;
    QProcess::ProcessState stateMemcached = QProcess::Running;

    stateNginx = processNginx->state();
    statePhp = processPhp->state();
    stateMariaDb = processMariaDb->state();
    stateMongoDb = processMongoDb->state();
    stateMemcached = processMemcached->state();

    if(stateNginx==QProcess::Starting || statePhp==QProcess::Starting || stateMariaDb==QProcess::Starting
            || stateMongoDb ==QProcess::Starting || stateMemcached ==QProcess::Starting)
    {
        timer->start();
    }
    else
    {
        timer->stop();
        setIcon(QIcon(":/wpnxm"));
    }

    // if NGINX or PHP are not running, disable PushButtons of Tools section, because target URL not available
    if(processNginx->state() == QProcess::NotRunning or processPhp->state() == QProcess::NotRunning)
    {
        emit signalEnableToolsPushButtons(false);
    }

    // if NGINX and PHP are running, enable PushButtons of Tools section
    if(processNginx->state() == QProcess::Running and processPhp->state() == QProcess::Running)
    {
        emit signalEnableToolsPushButtons(true);
    }

    return;
}

void Tray::nginxStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            nginxStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("nginx", false);
            break;
        case QProcess::Running:
            nginxStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("nginx", true);
            break;
        case QProcess::Starting:
            nginxStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

void Tray::phpStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            phpStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("php", false);
            break;
        case QProcess::Running:
            phpStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("php", true);
            break;
        case QProcess::Starting:
            phpStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

void Tray::mariaDbStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            mariaDbStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("mariadb", false);
            break;
        case QProcess::Running:
            mariaDbStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("mariadb", true);
            break;
        case QProcess::Starting:
            mariaDbStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

void Tray::mongoDbStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            mongoDbStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("mongodb", false);
            break;
        case QProcess::Running:
            mongoDbStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("mongodb", true);
            break;
        case QProcess::Starting:
            mongoDbStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

void Tray::memcachedStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            memcachedStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("memcached", false);
            break;
        case QProcess::Running:
            memcachedStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("memcached", true);
            break;
        case QProcess::Starting:
            memcachedStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

/*
 * Error slots
 */
void Tray::nginxProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "Nginx Error. "+getProcessErrorMessage(error));
}

void Tray::phpProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "PHP Error. "+getProcessErrorMessage(error));
}

void Tray::mariaDbProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "MariaDB Error. "+getProcessErrorMessage(error));
}

void Tray::mongoDbProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "MongoDB Error. "+getProcessErrorMessage(error));
}

void Tray::memcachedProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "Memcached Error. "+getProcessErrorMessage(error));
}

QString Tray::getProcessErrorMessage(QProcess::ProcessError error){
    QString ret;
    ret = ret + " <br/> ";
    switch(error){
        case QProcess::FailedToStart:
            ret = "The process failed to start. <br/> Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
            break;
        case QProcess::Crashed:
            ret = "The process crashed some time after starting successfully.";
            break;
        case QProcess::Timedout:
            ret = "The process timed out.";
            break;
        case QProcess::WriteError:
            ret = "An error occurred when attempting to write to the process. <br/> For example, the process may not be running, or it may have closed its input channel.";
            break;
        case QProcess::ReadError:
            ret = "An error occurred when attempting to read from the process. <br/> For example, the process may not be running.";
            break;
        case QProcess::UnknownError:
            ret = "An unknown error occurred.";
            break;
    }
    return ret;
}
