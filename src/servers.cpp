#include "servers.h"
#include "settings.h"
#include "tray.h"
#include "mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QTime>

Servers::Servers(QObject *parent) : QObject(parent), settings(new Settings)
{
    // build server objects
    foreach(QString serverName, getListOfServerNamesInstalled())
    {
        Server *server = new Server();

        server->lowercaseName = serverName;
        server->name = getCamelCasedServerName(serverName);
        server->icon = QIcon(":/status_stop");
        //server->configFiles = QStringList() << "a" << "b";
        server->logFiles = getLogFiles(serverName);
        server->workingDirectory = settings->get("paths/" + serverName).toString();
        server->exe = getExecutable(server->name);

        QProcess *process = new QProcess();
        process->setObjectName(server->name);
        process->setWorkingDirectory(server->workingDirectory);

        // monitor process state changes
        connect(process, SIGNAL(stateChanged(QProcess::ProcessState)),
                this, SLOT(updateProcessStates(QProcess::ProcessState)));

        // show process errors in a MessageBox
        connect(process, SIGNAL(error(QProcess::ProcessError)),
                this, SLOT(showProcessError(QProcess::ProcessError)));

        server->process = process;

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
        connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(mapAction(QAction*)), Qt::UniqueConnection);

        server->trayMenu = menu;

        serverList << server;

        qDebug() << "[" + serverName + "] was added to ServersList.";
    }
}

void Servers::mapAction(QAction *action) {
    QMetaObject::invokeMethod(this, action->objectName().toLocal8Bit().constData() );
}

/**
 * @brief Servers::getCamelCasedServerName
 * translates lowercase server and process names to internally used camel-cased label names
 * @param serverName
 * @return
 */
QString Servers::getCamelCasedServerName(QString &serverName) const
{
    if(serverName == "nginx") { return "Nginx"; }
    if(serverName == "memcached") { return "Memcached"; }
    if(serverName == "mongodb" or serverName == "mongod") { return "MongoDb"; }
    if(serverName == "mariadb" or serverName == "mysqld") { return "MariaDb"; }
    if(serverName == "php" or serverName == "php-cgi") { return "PHP"; }
    if(serverName == "postgresql" or serverName == "postgres") { return "PostgreSQL"; }

    return QString("Unknown");
}

QStringList Servers::getLogFiles(QString &serverName) const
{
    QString s = serverName.toLower();
    QString logs = QDir(settings->get("paths/logs").toString()).absolutePath();

    QStringList logfiles;

    if(s == "nginx")     { logfiles << logs + "/error.log" << logs + "/access.log"; }
    if(s == "memcached") { logfiles << ""; }
    if(s == "mongodb")   { logfiles << logs + "/mongodb.log"; }
    if(s == "mariadb")   { logfiles << logs + "/mariadb_error.log"; }
    if(s == "php")       { logfiles << logs + "/php_error.log"; }
    if(s == "postgresql"){ logfiles << logs + "/postgresql.log"; }

    return logfiles;
}

QString Servers::getExecutable(QString &serverName) const
{
    QString s = serverName.toLower();
    QString exe;
    if(s == "nginx")     { exe = "nginx.exe"; }
    if(s == "memcached") { exe = "memcached.exe"; }
    if(s == "mongodb")   { exe = "mongod.exe"; }
    if(s == "mariadb")   { exe = "mysqld.exe"; }
    if(s == "php")       { exe = "php-cgi.exe"; }
    if(s == "postgresql"){ exe = "pg_ctl.exe"; }

    return QDir::toNativeSeparators(settings->get("paths/" + serverName).toString() + "/" + exe);
}

QStringList Servers::getListOfServerNames() const
{
    // get daemons names from .ini's autostart group
    //Settings *settings = new Settings();
    //QStringList list = settings->getKeys("autostart");

    QStringList list;
    list << "nginx" << "php" << "mariadb" << "mongodb" << "memcached" << "postgresql";

    return list;
}

QStringList Servers::getListOfServerNamesInstalled()
{
    QStringList list;
    foreach(QString serverName, getListOfServerNames()) {

        // these three servers form the base package.
        // we assume that they are always installed.
        // this is also for testing, because they appear installed, even if they are not.
        if(serverName == "nginx" || serverName == "php" || serverName == "mariadb"||
           QFile().exists(getExecutable(serverName))) {
            qDebug() << "[" + serverName + "] is installed.";
            list << serverName;
        } else {
            qDebug() << "[" + serverName + "] is not installed.";
        }
    }
    return list;
}

QList<Server*> Servers::servers() const
{
    return serverList;
}

Server* Servers::getServer(const char *serverName) const
{
     QString name = QString(serverName).toLocal8Bit().constData();

     foreach(Server *server, serverList) {
         //qDebug() << server->name;
         if(server->name == name)
             return server;
     }

     // anti "control reaches end of non-void function" faked return
     Server *server = new Server();
     server->name = QString("Not Installed");
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

bool Servers::truncateFile(const QString &file) const
{
    QFile f(file);

    if(f.exists()) {
        f.open(QFile::WriteOnly|QFile::Truncate);
        f.close();
        return true;
    }
    f.close();
    return false;
}

void Servers::clearLogFile(const QString &serverName) const
{
    if(settings->get("global/clearlogsonstart").toBool()) {

        QString dirLogs = settings->get("paths/logs").toString();
        QString logfile = "";

        if(serverName == "Nginx") {
            truncateFile(dirLogs + "/access.log");
            truncateFile(dirLogs + "/error.log");
        }

        if(serverName == "PHP")        { logfile = dirLogs + "/php_error.log";}
        if(serverName == "MariaDb")    { logfile = dirLogs + "/mariadb_error.log";}
        if(serverName == "MongoDb")    { logfile = dirLogs + "/mongodb.log";}
        if(serverName == "PostgreSQL") { logfile = dirLogs + "/postgresql.log";}

        truncateFile(logfile);

        qDebug() << "[" << serverName << "] Cleared logs...\n";
    }
}

// Server "Start - Stop - Restart" Methods

/*
 * Nginx - Actions: run, stop, restart
 */
void Servers::startNginx()
{
    // already running
    if(getProcessState("Nginx") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("Nginx"), tr("Nginx already running."));
        return;
    }

    clearLogFile("Nginx");

    updateVersion("Nginx");

    // http://wiki.nginx.org/CommandLine - start daemon
    QString const startNginx = getServer("Nginx")->exe
            + " -p " + qApp->applicationDirPath() +
            + " -c " + qApp->applicationDirPath() + "/bin/nginx/conf/nginx.conf";

    qDebug() << "[Nginx] Starting...\n" << startNginx;

    getProcess("Nginx")->start(startNginx);
}

void Servers::stopNginx()
{
    // if not installed, skip
    if(!QFile().exists(getServer("Nginx")->exe)) {
        qDebug() << "[Nginx] Is not installed. Skipping stop command.";
        return;
    }

    // http://wiki.nginx.org/CommandLine - stop daemon
    QString const stopNginx = getServer("Nginx")->exe
            + " -p " + qApp->applicationDirPath()
            + " -c " + qApp->applicationDirPath() + "/bin/nginx/conf/nginx.conf"
            + " -s stop";
    qDebug() << "[Nginx] Stopping...\n" << stopNginx;

    QProcess::execute(stopNginx);
}

void Servers::reloadNginx()
{
    QProcess *process = getProcess("Nginx");

    QString const reloadNginx = getServer("Nginx")->exe
            + " -p " + qApp->applicationDirPath()
            + " -c " + qApp->applicationDirPath() + "/bin/nginx/conf/nginx.conf"
            + "-s reload";

    qDebug() << "[Nginx] Reloading...\n" << reloadNginx;

    process->start(reloadNginx);
    process->waitForFinished(1500);
}

void Servers::restartNginx()
{
    stopNginx();
    startNginx();
}

/*
 * PostgreSQL Actions: run, stop, restart
 */
void Servers::startPostgreSQL()
{
    // if not installed, skip
    if(!QFile().exists(qApp->applicationDirPath() + "/bin/pgsql/bin/pg_ctl.exe")) {
        qDebug() << "[PostgreSQL] Is not installed. Skipping start command.";
        return;
    }

    // already running
    if(QFile().exists(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid")) {
        QMessageBox::warning(0, tr("PostgreSQL"), tr("PostgreSQL is already running."));
        return;
    }

    clearLogFile("PostgreSQL");

    updateVersion("PostgreSQL");

    // start daemon
    QString const startCmd = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/bin/pg_ctl.exe")
            + " --pgdata " + QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data")
            + " --log " + QDir::toNativeSeparators(qApp->applicationDirPath() + "/logs/postgresql.log")
            + " start";

    qDebug() << "[PostgreSQL] Starting...\n" << startCmd;

    getProcess("PostgreSQL")->start(startCmd);
}

void Servers::delay(int millisecondsToWait) const
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void Servers::stopPostgreSQL()
{
    Server *server = getServer("PostgreSQL");

    // if not installed, skip
    if(!QFile().exists(server->exe)) {
        qDebug() << "[PostgreSQL] Is not installed. Skipping stop command.";
        return;
    }

    // running?
    QString file = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
    if(!QFile().exists(file)) {
        qDebug() << "[PostgreSQL] NO PID file found. Postgres is not running. No need to stop it.";
        server->trayMenu->setIcon(QIcon(":/status_stop"));
        emit signalSetLabelStatusActive("postgresql", false);
        return;
    }

    qDebug() << "[PostgreSQL] Stopping...";

    // disconnect process monitoring, before crashing the process
    //disconnect(getProcess("PostgreSQL"), SIGNAL(error(QProcess::ProcessError)),
              // this, SLOT(showProcessError(QProcess::ProcessError)));

    QString stopCommand = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/bin/pg_ctl.exe")
            + " stop "
            + " --pgdata " + QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data")
            + " --log " + QDir::toNativeSeparators(qApp->applicationDirPath() + "/logs/postgresql.log")
            + " --mode=fast"
            + " -W";

    getProcess("PostgreSQL")->execute(stopCommand);

    // do we have a failed shutdown? if so, delete PID file, to allow a restart
    if(QFile().exists(file)) {
        QFile().remove(file);
    }

    server->trayMenu->setIcon(QIcon(":/status_stop"));
    emit signalSetLabelStatusActive("postgresql", false);
}

void Servers::restartPostgreSQL()
{
    stopPostgreSQL();
    startPostgreSQL();
}

/*
 * PHP - Actions: run, stop, restart
 */
void Servers::startPHP()
{
    // already running
    if(getProcessState("PHP") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("PHP"), tr("PHP is already running."));
        return;
    }

    clearLogFile("PHP");

    updateVersion("PHP");

    // start daemon
    QString const startPHP = getServer("PHP")->exe
            + " -b " + settings->get("php/fastcgi-host").toString()
            + ":" + settings->get("php/fastcgi-port").toString();

    // disable PHP_FCGI_MAX_REQUESTS to go beyond the default request limit of 500 requests
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PHP_FCGI_MAX_REQUESTS", "0");

    qDebug() << "[PHP] Set PHP_FCGI_MAX_REQUESTS \"0\" (disabled).";
    qDebug() << "[PHP] Starting...\n" << startPHP;

    QProcess* process = getProcess("PHP");
    process->setEnvironment(env.toStringList());
    process->start(startPHP);
}

void Servers::stopPHP()
{
    qDebug() << "[PHP] Stopping...";

    // if not installed, skip
    if(!QFile().exists(getServer("PHP")->exe)) {
        qDebug() << "[PHP] Is not installed. Skipping stop command.";
        return;
    }

    if(getProcessState("PHP") == QProcess::NotRunning) {
        qDebug() << "[PHP] Not running... Skipping stop command.";
        return;
    }

    QProcess *process = getProcess("PHP");

    /**
     * There are two ways to kill the PHP daemon:
     * 1) processPhp->terminate();
     *    This will fail, because the WM_CLOSE message is not handled.
     * 2) By killing the process with kill(). So we are crashing it!
     *
     * But before we kill/crash the PHP daemon intentionally, we need to disconnect
     * the QProcess Error Monitoring (signal/sender from method/receiver),
     * else a "Process Crashed" Error MessageBox would appear.
     */
    disconnect(process, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(showProcessError(QProcess::ProcessError)));

    // kill PHP daemon
    process->kill();
    process->waitForFinished(2000);
}

void Servers::restartPHP()
{
    stopPHP();
    startPHP();
}

/*
 * MariaDb Actions - run, stop, restart
 */
void Servers::startMariaDb()
{
    // already running
    if(getProcessState("MariaDb") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("MariaDB"), tr("MariaDB already running."));
        return;
    }

    clearLogFile("MariaDb");

    updateVersion("MariaDb");

    // start
    QString const startMariaDb = getServer("MariaDb")->exe
            + " --defaults-file=" + QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/mariadb/my.ini");

    qDebug() << "[MariaDB] Starting...\n" << startMariaDb;

    getServer("MariaDb")->process->start(startMariaDb);
}

void Servers::stopMariaDb()
{
    qDebug() << "[MariaDB] Stopping...";

    // if not installed, skip
    if(!QFile().exists(getServer("MariaDb")->exe)) {
        qDebug() << "[MariaDb] Is not installed. Skipping stop command.";
        return;
    }

    QProcess *process = getProcess("MariaDb");

    // disconnect process monitoring, before crashing the process
    disconnect(process, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(showProcessError(QProcess::ProcessError)));

    process->kill();
    process->waitForFinished(2000);
}

void Servers::restartMariaDb()
{
    stopMariaDb();
    startMariaDb();
}

/*
 * MongoDb Actions - run, stop, restart
 */
void Servers::startMongoDb()
{
    // if not installed, skip
    if(!QFile().exists(getServer("MongoDb")->exe)) {
        qDebug() << "[MongoDb] Is not installed. Skipping start command.";
        return;
    }

    // already running
    if(getProcessState("MongoDb") != QProcess::NotRunning) {
        QMessageBox::warning(0, tr("MongoDb"), tr("MongoDb already running."));
        return;
    }

    clearLogFile("MongoDb");

    updateVersion("MongoDb");

    // MongoDb doesn't start, when data dir is missing...
    QString const mongoDbDataDir = qApp->applicationDirPath() + "/bin/mongodb/data/db";
    if(QDir().exists(qApp->applicationDirPath() + "/bin/mongodb") && !QDir().exists(mongoDbDataDir)) {
        qDebug() << "[MongoDb] Creating Directory for Mongo's Database...\n" << mongoDbDataDir;
        QDir().mkpath(mongoDbDataDir);
    }

    // MongoDb doesn't start, when logfile is missing...
    QFile f(qApp->applicationDirPath() + "/logs/mongodb.log");
    if(!f.exists()) {
        qDebug() << "[MongoDb] Creating empty logfile...\n" << qApp->applicationDirPath() + "/logs/mongodb.log";
        f.open(QIODevice::ReadWrite);
        f.close();
    }

    // build mongo start command
    QString const mongoStartCommand = getServer("MongoDb")->exe
             + " --config " + qApp->applicationDirPath() + "/bin/mongodb/mongodb.conf"
             + " --dbpath " + qApp->applicationDirPath() + "/bin/mongodb/data/db"
             + " --logpath " + qApp->applicationDirPath() + "/logs/mongodb.log"
             + " --rest";

    qDebug() << "[MongoDb] Starting...\n"<< mongoStartCommand;

    // start
    getProcess("MongoDb")->start(mongoStartCommand);
}

void Servers::stopMongoDb()
{
    // if not installed, skip
    if(!QFile().exists(getServer("MongoDb")->exe)) {
        qDebug() << "[MongoDb] Is not installed. Skipping stop command.";
        return;
    }

    // build mongo stop command based on CLI evaluation
    // mongodb is stopped via "mongo.exe --eval", not "mongodb.exe"
    QString const mongoStopCommand = qApp->applicationDirPath() + "/bin/mongodb/bin/mongo.exe"
             + " --eval \"db.getSiblingDB('admin').shutdownServer()\"";

    qDebug() << "[MongoDb] Stopping...\n" << mongoStopCommand;

    // disconnect process monitoring before sending shutdown
    disconnect(getProcess("MongoDb"), SIGNAL(error(QProcess::ProcessError)),
                   this, SLOT(showProcessError(QProcess::ProcessError)));

    QProcess::execute(mongoStopCommand);
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
    QString const memcachedStartCommand = getServer("Memcached")->exe;

    // if not installed, skip
    if(!QFile().exists(memcachedStartCommand)) {
        qDebug() << "[Memcached] Is not installed. Skipping start command.";
        return;
    }

    // already running
    if(getProcessState("Memcached") != QProcess::NotRunning){
        QMessageBox::warning(0, tr("Memcached"), tr("Memcached already running."));
        return;
    }

    updateVersion("Memcached");


    // start
    qDebug() << "[Memcached] Starting...\n" << memcachedStartCommand;

    getProcess("Memcached")->start(memcachedStartCommand);
}

void Servers::stopMemcached()
{
    // if not installed, skip
    if(!QFile().exists(getServer("Memcached")->exe)) {
        qDebug() << "[Memcached] Is not installed. Skipping stop command.";
        return;
    }

    QProcess *process = getProcess("Memcached");

    // disconnect process monitoring, before crashing the process
    disconnect(process, SIGNAL(error(QProcess::ProcessError)),
               this, SLOT(showProcessError(QProcess::ProcessError)));

    qDebug() << "[Memcached] Stopping...\n";

    process->kill();
    process->waitForFinished(2000);
}

void Servers::restartMemcached()
{
    stopMemcached();
    startMemcached();
}

/*
 * Process Error Slot
 */
void Servers::showProcessError(QProcess::ProcessError error)
{
    QString name = sender()->objectName();
    QString title = qApp->applicationName() + " - " + name + " Error";
    QString msg =  name + " Error. " + getProcessErrorMessage(error);

    QMessageBox::warning(0, title, msg);
}

QString Servers::getProcessErrorMessage(QProcess::ProcessError error)
{
    QString ret = " <br/> ";

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

/*
 * Process State Slot
 */
void Servers::updateProcessStates(QProcess::ProcessState state)
{
    qDebug() << "Sender: " << sender()->objectName();

    QString serverName = sender()->objectName();
    Server *server = getServer(serverName.toLocal8Bit().constData());

    qDebug() << server->name << state;

    switch(state)
    {
        case QProcess::NotRunning:
                server->trayMenu->setIcon(QIcon(":/status_stop"));
                emit signalSetLabelStatusActive(serverName, false);

                // if NGINX or PHP are not running, disable PushButtons of Tools section,
                // because target URL is not available
                if((serverName == "Nginx") or (serverName == "PHP")) {
                    qDebug() << "Signal: Disable - Tools Pushbuttons";
                    emit signalEnableToolsPushButtons(false);
                }
            break;
        case QProcess::Running:
                if(serverName == "PostgreSQL") {
                    return; // this stops the activation blink, real start is detected by PID file
                }
                server->trayMenu->setIcon(QIcon(":/status_run"));
                emit signalSetLabelStatusActive(serverName, true);
            break;
        case QProcess::Starting:
                server->trayMenu->setIcon(QIcon(":/status_reload"));
            break;
    }

    // if NGINX and PHP are running, enable PushButtons of Tools section
    if((getProcessState("Nginx") == QProcess::Running) && (getProcessState("PHP") == QProcess::Running)) {
        qDebug() << "Signal: Enable - Tools Pushbuttons";
        emit signalEnableToolsPushButtons(true);
    }

    // PostgreSQL Process Monitoring via PID file
    // we can't get process monitoring after calling "pg_ctl start" (startPostgreSQL),
    // because "pg_ctl" is only a launcher and it will shutdown.

    if(server->name == "PostgreSQL" && state == QProcess::NotRunning) {

        delay(1250); // delay PID file check, PostgreSQL must start up

        QString file = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
        qDebug() << file;

        if(QFile().exists(file)) {
            qDebug() << "[PostgreSQL] PID file found. Postgres is running.";
            server->trayMenu->setIcon(QIcon(":/status_run"));
            emit signalSetLabelStatusActive("postgresql", true);
        } else {
            qDebug() << "[PostgreSQL] PID file not found. Postgres is not running.";
            server->trayMenu->setIcon(QIcon(":/status_stop"));
            emit signalSetLabelStatusActive("postgresql", false);
        }
    }

    return;
}

void Servers::updateVersion(QString server) {
    MainWindow *mw = qobject_cast<MainWindow*>(parent());
    mw->updateVersion(server);
}
