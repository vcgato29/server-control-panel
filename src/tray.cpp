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

// Local WPN-XM SCP includes
#include "tray.h"
#include "hostmanager/hostmanagerdialog.h"

// Global Qt includes
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QWidgetAction>
#include <QMessageBox>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

// Constructor
Tray::Tray(QApplication *parent) : QSystemTrayIcon(parent)
{
    // Tray Icon
    setIcon(QIcon(":/wpnxm"));

    // @todo append status of the daemons to tooltip, e.g. "Nginx up, PHP up, MariaDB up"
    // or create seperate popup?
    setToolTip("WPN-XM");

    initializeConfiguration();

    startMonitoringDaemonProcesses();

    createTrayMenu();

    // @todo make this a configuration option in user preferences dialog
    /*if(bAutostartDaemons)
    {
        startAllDaemons();
    }*/

    /* Auto-connect Slots
       The following definition isneeded:
                void on_<object name>_<signal name>(<signal parameters>);
       like:    private slots: void on_okButton_clicked();
                or  void on_xy_triggered();
    */
    // QMetaObject::connectSlotsByName(this);
}

// Destructor
Tray::~Tray()
{
    // @todo stop all daemons, when you quit the tray application?
    // add option to configure dialog
    stopAllDaemons();

    delete trayMenu;
}

void Tray::startMonitoringDaemonProcesses()
{
    // the timer is used for monitoring the process state of each daemon
    timer = new QTimer(this);
    timer->setInterval(1000); // msec = 1sec

    processNginx = new QProcess(this);
    processNginx->setWorkingDirectory(cfgNginxDir);
    connect(processNginx, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(nginxStateChanged(QProcess::ProcessState)));
    connect(processNginx, SIGNAL(error(QProcess::ProcessError)), this, SLOT(nginxProcessError(QProcess::ProcessError)));

    processPhp = new QProcess(this);
    processPhp->setWorkingDirectory(cfgPhpDir);
    connect(processPhp, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(phpStateChanged(QProcess::ProcessState)));
    connect(processPhp, SIGNAL(error(QProcess::ProcessError)), this, SLOT(phpProcessError(QProcess::ProcessError)));

    processMariaDB = new QProcess(this);
    processMariaDB->setWorkingDirectory(cfgMariaDBDir);
    connect(processMariaDB, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(mariaDBStateChanged(QProcess::ProcessState)));
    connect(processMariaDB, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mariaDBProcessError(QProcess::ProcessError)));
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
    mariaDBStatusSubmenu = new QMenu("MariaDB", trayMenu);
    mariaDBStatusSubmenu->setIcon(QIcon(":/status_stop"));
    mariaDBStatusSubmenu->addAction(QIcon(":/action_restart"), tr("Restart"), this, SLOT(restartMariaDB()), QKeySequence());
    mariaDBStatusSubmenu->addSeparator();
    mariaDBStatusSubmenu->addAction(QIcon(":/action_run"), tr("Start"), this, SLOT(startMariaDB()), QKeySequence());
    mariaDBStatusSubmenu->addAction(QIcon(":/action_stop"), tr("Stop"), this, SLOT(stopMariaDB()), QKeySequence());

    // Build Tray Menu

    // add title entry like for WPN-XM in KVirc style (background gray, bold, small font)
    // trayMenu->addAction("WPN-XM SCP")->setFont(QFont("Arial", 8, QFont::Bold));
    trayMenu->addAction(QIcon(":/action_run"), tr("Start All"), this, SLOT(startAllDaemons()), QKeySequence());
    trayMenu->addAction(QIcon(":/action_stop"), tr("Stop All"), this, SLOT(stopAllDaemons()), QKeySequence());
    trayMenu->addSeparator();
    trayMenu->addMenu(nginxStatusSubmenu);
    trayMenu->addMenu(phpStatusSubmenu);
    trayMenu->addMenu(mariaDBStatusSubmenu);
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

//*
//* Action slots
//*
void Tray::startAllDaemons()
{
    startNginx();
    startPhp();
    startMariaDB();
}

void Tray::stopAllDaemons()
{
    stopMariaDB();
    stopPhp();
    stopNginx();
}

void Tray::restartAll()
{
    restartNginx();
    restartPhp();
    restartMariaDB();
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
    //qDebug() << cfgNginxDir+NGINX_EXEC << "-p" << QDir::currentPath() << "-c" << QDir::currentPath() + "/bin/nginx/conf/nginx.conf";
    processNginx->start(cfgNginxDir+NGINX_EXEC, QStringList() << "-p" << QDir::currentPath() << "-c" << QDir::currentPath() + "/bin/nginx/conf/nginx.conf");
}

void Tray::stopNginx()
{
    QProcess processStopNginx;
    processStopNginx.setWorkingDirectory(cfgNginxDir);
    //qDebug() << cfgNginxDir+NGINX_EXEC << "-s" << "stop";
    processStopNginx.start(cfgNginxDir+NGINX_EXEC, QStringList() << "-p" << QDir::currentPath() << "-c" << QDir::currentPath() + "/bin/nginx/conf/nginx.conf" << "-s" << "stop"); // fast shutdown
    processStopNginx.waitForFinished();
}

void Tray::reloadNginx()
{
    QProcess processStopNginx;
    processStopNginx.setWorkingDirectory(cfgNginxDir);
    processStopNginx.start(cfgNginxDir+NGINX_EXEC, QStringList() << "-p" << QDir::currentPath() << "-c" << QDir::currentPath() + "/bin/nginx/conf/nginx.conf" << "-s" << "reload");
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
    if(processPhp->state() != QProcess::NotRunning)
    {
        QMessageBox::warning(0, tr("PHP"), tr("PHP is already running."));
        return;
    }

    // start daemon
    processPhp->start(cfgPhpDir+PHPCGI_EXEC, QStringList() << "-b" << cfgPhpFastCgiHost+":"+cfgPhpFastCgiPort);
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
 * MariaDB Actions - run, stop, restart
 */
void Tray::startMariaDB()
{
    // already running
    if(processMariaDB->state() != QProcess::NotRunning){
        QMessageBox::warning(0, tr("MariaDB"), tr("MariaDB already running."));
        return;
    }

    // start
    qDebug() << cfgMariaDBDir+MARIADB_EXEC;
    processMariaDB->start(cfgMariaDBDir+MARIADB_EXEC);
}

void Tray::stopMariaDB()
{
    // disconnect process monitoring, before crashing the process
    disconnect(processMariaDB, SIGNAL(error(QProcess::ProcessError)), this, SLOT(mariaDBProcessError(QProcess::ProcessError)));

    processMariaDB->kill();
    processMariaDB->waitForFinished();
}

void Tray::restartMariaDB()
{
    stopMariaDB();
    startMariaDB();
}

/*
 * MongoDB Actions - run, stop, restart
 */
void Tray::startMongoDB()
{
    // already running
    if(processMongoDB->state() != QProcess::NotRunning){
        QMessageBox::warning(0, tr("MongoDB"), tr("MongoDB already running."));
        return;
    }

    // start
    qDebug() << cfgMongoDBDir+MARIADB_EXEC;
    processMongoDB->start(cfgMongoDBDir+MARIADB_EXEC);
}

void Tray::stopMongoDB()
{
    // disconnect process monitoring, before crashing the process
    disconnect(processMongoDB, SIGNAL(error(QProcess::ProcessError)), this, SLOT(MongoDBProcessError(QProcess::ProcessError)));

    processMongoDB->kill();
    processMongoDB->waitForFinished();
}

void Tray::restartMongoDB()
{
    stopMongoDB();
    startMongoDB();
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
    qDebug() << cfgMemcachedDir+MARIADB_EXEC;
    processMemcached->start(cfgMemcachedDir+MARIADB_EXEC);
}

void Tray::stopMemcached()
{
    // disconnect process monitoring, before crashing the process
    disconnect(processMemcached, SIGNAL(error(QProcess::ProcessError)), this, SLOT(MemcachedProcessError(QProcess::ProcessError)));

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
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(cfgNginxDir+cfgNginxSites));

    // start as own process ( not as a child process), will live after Tray terminates
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openNginxConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(cfgNginxConfig));
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openNginxLogs()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(cfgLogsDir));
    QProcess::startDetached("explorer", QStringList() << strDir);
}

void Tray::openMariaDBClient()
{
    QProcess::startDetached(cfgMariaDBDir+MARIADB_CLIENT_EXEC, QStringList(), cfgMariaDBDir);
}

void Tray::openMariaDBConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(cfgMariaDBConfig));
    QProcess::startDetached("cmd", QStringList() << "/c" << "start "+strDir);
}

void Tray::openPhpConfig()
{
    QDir dir(QDir::currentPath());
    QString strDir = QDir::toNativeSeparators(dir.absoluteFilePath(cfgPhpConfig));
    QProcess::startDetached("cmd", QStringList() << "/c" << "start "+strDir);
}

/*
 * State slots
 */
void Tray::globalStateChanged()
{
    QProcess::ProcessState stateNginx = QProcess::Running;
    QProcess::ProcessState statePhp = QProcess::Running;
    QProcess::ProcessState stateMariaDB = QProcess::Running;
    QProcess::ProcessState stateMongoDB = QProcess::Running;
    QProcess::ProcessState stateMemcached = QProcess::Running;

    stateNginx = processNginx->state();
    statePhp = processPhp->state();
    stateMariaDB = processMariaDB->state();
    stateMongoDB = processMongoDB->state();
    stateMemcached = processMemcached->state();

    if(stateNginx==QProcess::Starting || statePhp==QProcess::Starting || stateMariaDB==QProcess::Starting
            || stateMongoDB ==QProcess::Starting || stateMemcached ==QProcess::Starting)
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

void Tray::mariaDBStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            mariaDBStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("mariadb", false);
            break;
        case QProcess::Running:
            mariaDBStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("mariadb", true);
            break;
        case QProcess::Starting:
            mariaDBStatusSubmenu->setIcon(QIcon(":/status_reload"));
            break;
    }
    globalStateChanged();
}

void Tray::mongoDBStateChanged(QProcess::ProcessState state)
{
    switch(state)
    {
        case QProcess::NotRunning:
            mongoDBStatusSubmenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("mongodb", false);
            break;
        case QProcess::Running:
            mongoDBStatusSubmenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("mongodb", true);
            break;
        case QProcess::Starting:
            mongoDBStatusSubmenu->setIcon(QIcon(":/status_reload"));
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

void Tray::mariaDBProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "MariaDB Error. "+getProcessErrorMessage(error));
}

void Tray::mongoDBProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "MongoDB Error. "+getProcessErrorMessage(error));
}

void Tray::memcachedProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, APP_NAME " - Error", "Memcached Error. "+getProcessErrorMessage(error));
}

QString Tray::getProcessErrorMessage(QProcess::ProcessError error){
    QString ret;
    switch(error){
        case QProcess::FailedToStart:
            ret = "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
            break;
        case QProcess::Crashed:
            ret = "The process crashed some time after starting successfully.";
            break;
        case QProcess::Timedout:
            ret = "The process timed out.";
            break;
        case QProcess::WriteError:
            ret = "An error occurred when attempting to write to the process. For example, the process may not be running, or it may have closed its input channel.";
            break;
        case QProcess::ReadError:
            ret = "An error occurred when attempting to read from the process. For example, the process may not be running.";
            break;
        case QProcess::UnknownError:
            ret = "An unknown error occurred.";
            break;
    }
    return ret;
}
