#include "servers.h"
#include "settings.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QProcess>

#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define DBGPRINT(fmt, ...) SLOT(printf(fmt, __VA_ARGS__))

Servers::Servers(QObject *parent) : QObject(parent), settings(new Settings)
{
    // build server objects
    foreach(QString daemonName, getListOfServerNames())
    {
        Server *server = new Server();

        server->name = fixName(daemonName);
        server->icon = QIcon(":/status_stop");
        server->configFiles = QStringList() << "a" << "b";
        server->logFiles = QStringList() << "a" << "b";
        server->workingDirectory = settings->get("paths/" + server->name).toString();
        server->process = new QProcess();

        QMenu *menu =  new QMenu(server->name);
        menu->setObjectName(QString("menu").append(server->name));
        menu->setIcon(server->icon);

        QAction *restartAction = new QAction(QIcon(":/action_restart"), tr("Restart"), menu);
        QAction *startAction = new QAction(QIcon(":/action_run"), tr("Start"), menu);
        QAction *stopAction = new QAction(QIcon(":/action_stop"), tr("Stop"), menu);

        restartAction->setObjectName(QString("restart").append(server->name));
        startAction->setObjectName(QString("start").append(server->name));
        stopAction->setObjectName(QString("stop").append(server->name));

        menu->addAction(restartAction);
        menu->addSeparator();
        menu->addAction(startAction);
        menu->addAction(stopAction);

        // connect ONLY ONCE
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(mySlot(QAction*)), Qt::UniqueConnection);

        server->trayMenu = menu;

        serversList << server;
    }
}

void Servers::mySlot(QAction *action) {
   action->dumpObjectInfo();
}

QString Servers::fixName(QString &serverName) const
{
    if(serverName == "nginx") { return "Nginx"; }
    if(serverName == "memcached") { return "Memcached"; }
    if(serverName == "mongodb") { return "MongoDb"; }
    if(serverName == "mariadb") { return "MariaDb"; }
    if(serverName == "php") { return "PHP"; }
    return QString();
}

QStringList Servers::getListOfServerNames() const
{
    // get daemons names from .ini's autostart group
    Settings *settings = new Settings();
    QStringList list = settings->getKeys("autostart");
    return list;
}

QList<Server*> Servers::servers() const
{
    return serversList;
}

/**
 * @brief Servers::getServer
 * @param serverName
 * @return Server
 */
Server* Servers::getServer(const char *serverName) const
{
     QString name = QString(serverName).toLocal8Bit().constData();

     foreach(Server *server, serversList) {
         if(server->name == name)
             return server;
     }

     // faked return
     Server *server = new Server();
     server->name = QString("Unknown");
     return server;
}

QProcess* Servers::getProcess(const char *serverName) const
{
    return getServer(serverName)->process;
}

QProcess::ProcessState Servers::getProcessState(const char *serverName) const
{
    return getProcess(serverName)->state();
}

// Server "Start - Stop - Restart" Methods

/*
 * Nginx - Actions: run, stop, restart
 */
void Servers::startNginx()
{
    qDebug() << "1";

    // already running
    if(getProcessState("Nginx") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("Nginx"), tr("Nginx already running."));
        return;
    }

     qDebug() << "2";

    // start daemon
    QString const startNginx = settings->get("paths/nginx").toString() + NGINX_EXEC
            + " -p " + QDir::currentPath()
            + " -c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf";

    qDebug() << "[Nginx] Starting...\n" << startNginx;

    getProcess("Nginx")->start(startNginx);
}

void Servers::stopNginx()
{
    QProcess processStopNginx;
    processStopNginx.setWorkingDirectory(settings->get("paths/nginx").toString());

    // fast shutdown
    QString stopNginx = settings->get("paths/nginx").toString() + NGINX_EXEC
            + " -p " + QDir::currentPath()
            + " -c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf"
            + " -s stop";

    qDebug() << "[Nginx] Stopping...\n" << stopNginx;

    processStopNginx.start(stopNginx);
    processStopNginx.waitForFinished();
}

void Servers::reloadNginx()
{
    QString cfgNginxDir = settings->get("paths/nginx").toString();

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

void Servers::restartNginx()
{
    stopNginx();
    startNginx();
}

/*
 * PHP - Actions: run, stop, restart
 */
void Servers::startPhp()
{
    // already running
    if(getProcessState("PHP") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("PHP"), tr("PHP is already running."));
        return;
    }

    // start daemon
    QString const startPHP = settings->get("paths/php").toString()+PHPCGI_EXEC
            + " -b " + settings->get("php/fastcgi-host").toString()+":"+settings->get("php/fastcgi-port").toString();

    qDebug() << "[PHP] Starting...\n" << startPHP;

    getProcess("PHP")->start(startPHP);
}

void Servers::stopPhp()
{
    qDebug() << "[PHP] Stopping...";

    Server *server = getServer("PHP");
    qDebug() << server->name;

    if(getProcessState("PHP") == QProcess::NotRunning) {
        qDebug() << "[PHP] Not running...";
        return;
    }

    QProcess *processPHP = getProcess("PHP");

    // 1) processPhp->terminate(); will fail because WM_CLOSE message not handled
    // 2) By killing the process, we are crashing it!
    //    The user will then get a "Process Crashed" Error MessageBox.
    //    Therefore we need to disconnect signal/sender from method/receiver.
    //    The result is, that crashing the php daemon intentionally is not shown as error.
    //disconnect(processPHP, SIGNAL(error(QProcess::ProcessError)), this, SLOT(showProcessError(QProcess::ProcessError)));

    // kill PHP daemon
    processPHP->kill();
    processPHP->waitForFinished();
}

void Servers::restartPhp()
{
    stopPhp();
    startPhp();
}

/*
 * MariaDb Actions - run, stop, restart
 */
void Servers::startMariaDb()
{
    // already running
    if(getServer("MariaDb")->process->state() != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("MariaDB"), tr("MariaDB already running."));
        return;
    }

    // start
    QString const startMariaDb = settings->get("paths/mariadb").toString() + MARIADB_EXEC;
    qDebug() << "[MariaDB] Starting...\n" << startMariaDb;
    getServer("MariaDb")->process->start(startMariaDb);
}

void Servers::stopMariaDb()
{
    qDebug() << "[MariaDB] Stopping...";

    // disconnect process monitoring, before crashing the process
    disconnect(getProcess("MariaDb"), SIGNAL(error(QProcess::ProcessError)), this, SLOT(showProcessError(QProcess::ProcessError)));

    getProcess("MariaDb")->kill();
    getProcess("MariaDb")->waitForFinished();
}

void Servers::restartMariaDb()
{
    stopMariaDb();
    startMariaDb();
}

/*
 * MongoDB Actions - run, stop, restart
 */
void Servers::startMongoDb()
{
    // already running
    if(getProcessState("MongoDb") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("MongoDB"), tr("MongoDB already running."));
        return;
    }

    // if not installed, skip
    if(!QFile().exists(settings->get("paths/mongodb").toString()+MONGODB_EXEC)) {
        qDebug() << "[MongoDB] Is not installed. Skipping start command.";
        return;
    }

    // mongodb doesn't start, when data dir is missing...
    QString const mongoDbDataDir = qApp->applicationDirPath() + "/bin/mongodb/data/db";
    if(QDir().exists(qApp->applicationDirPath() + "/bin/mongodb") && !QDir().exists(mongoDbDataDir)) {
        qDebug() << "[MongoDB] Creating Directory for Mongo's Database...\n" << mongoDbDataDir;
        QDir().mkpath(mongoDbDataDir);
    }

    // mongodb doesn't start, when logfile is missing...
    QFile f(qApp->applicationDirPath() + "/logs/mongodb.log");
    if(!f.exists()) {
        qDebug() << "[MongoDB] Creating empty logfile...\n" << qApp->applicationDirPath() + "/logs/mongodb.log";
        f.open(QIODevice::ReadWrite);
        f.close();
    }

    // build mongo start command
    QString const mongoStartCommand = settings->get("paths/mongodb").toString()+MONGODB_EXEC
             + " --config " + qApp->applicationDirPath() + "/bin/mongodb/mongodb.conf"
             + " --dbpath " + qApp->applicationDirPath() + "/bin/mongodb/data/db"
             + " --logpath " + qApp->applicationDirPath() + "/logs/mongodb.log";

    qDebug() << "[MongoDB] Starting...\n"<< mongoStartCommand;

    // start
    getProcess("MongoDb")->start(mongoStartCommand);
}

void Servers::stopMongoDb()
{
    // if not installed, skip
    if(!QFile().exists(settings->get("paths/mongodb").toString()+MONGODB_EXEC)) {
        qDebug() << "[MongoDB] Is not installed. Skipping stop command.";
        return;
    }

    // build mongo stop command
    QString const mongoStopCommand = settings->get("paths/mongodb").toString() + "/mongo.exe"
             + " --eval \"db.getSiblingDB('admin').shutdownServer()\"";

    qDebug() << "[MongoDB] Stopping...\n" << mongoStopCommand;

    if(QProcess::execute(mongoStopCommand))
    {
        // disconnect process monitoring, if shutdown command successfully send
        disconnect(getProcess("MongoDb"), SIGNAL(error(QProcess::ProcessError)), this, SLOT(showProcessError(QProcess::ProcessError)));
    }
}

void Servers::restartMongoDb()
{
    stopMongoDb();
    startMongoDb();
}

/*
 * Memcached Actions - run, stop, restart
 */
void Servers::startMemcached()
{
    // already running
    if(getProcessState("Memcached") != QProcess::NotRunning){
        QMessageBox::warning(0, tr("Memcached"), tr("Memcached already running."));
        return;
    }

    // if not installed, skip
    if(!QFile().exists(settings->get("paths/memcached").toString()+MEMCACHED_EXEC)) {
        qDebug() << "[Memcached] Is not installed. Skipping start command.";
        return;
    }

    // start
    qDebug() << "[Memcached] Starting...\n" << settings->get("paths/memcached").toString()+MEMCACHED_EXEC;

    getProcess("Memcached")->start(settings->get("paths/memcached").toString()+MEMCACHED_EXEC);
}

void Servers::stopMemcached()
{
    // if not installed, skip
    if(!QFile().exists(settings->get("paths/memcached").toString()+MEMCACHED_EXEC)) {
        qDebug() << "[Memcached] Is not installed. Skipping stop command.";
        return;
    }

    QProcess *processMemcached = getProcess("Memcached");

    // disconnect process monitoring, before crashing the process
    disconnect(processMemcached, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(showProcessError(QProcess::ProcessError)));

    qDebug() << "[Memcached] Stopping...\n";

    processMemcached->kill();
    processMemcached->waitForFinished();
}

void Servers::restartMemcached()
{
    stopMemcached();
    startMemcached();
}

/*
 * Error slots
 */
void Servers::showProcessError(QProcess::ProcessError error)
{
    QMessageBox::warning(0, qApp->applicationName() + " - Error", " Error. " + getProcessErrorMessage(error));
}

QString Servers::getProcessErrorMessage(QProcess::ProcessError error){
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

