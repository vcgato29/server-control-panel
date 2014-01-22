#ifndef SERVERS_H
#define SERVERS_H

#include "settings.h"

#include <QProcess>
#include <QMenu>

class Server: public QObject
{
    Q_OBJECT

public:
        QString name;
        QIcon icon;
        QString workingDirectory;
        QStringList configFiles;
        QStringList logFiles;
        QMenu *trayMenu;

        // Process Monitoring
        QProcess *process;

        //Process onStart;
        //Process onStop;
};

class Servers : public QObject
{
    Q_OBJECT

public:
        Settings *settings;

        // default ctor
        Servers(QObject *parent = 0);

        QList<Server*> servers() const;
        QStringList getListOfServerNames() const;
        QString fixName(QString &serverName) const;
        Server *getServer(const char *serverName) const;
        QProcess* getProcess(const char *serverName) const;
        QProcess::ProcessState getProcessState(const char *serverName) const;

public slots:

        void showProcessError(QProcess::ProcessError error);

        void mySlot(QAction *action);

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
        void startMariaDb();
        void stopMariaDb();
        void restartMariaDb();

        // MongoDB Action Slots
        void startMongoDb();
        void stopMongoDb();
        void restartMongoDb();

        // Memcached Action Slots
        void startMemcached();
        void stopMemcached();
        void restartMemcached();

private:

        QList<Server*> serversList;

        QString getProcessErrorMessage(QProcess::ProcessError);

        // Executables
        #define PHPCGI_EXEC "/php-cgi.exe"
        #define NGINX_EXEC "/nginx.exe"
        #define MARIADB_EXEC "/mysqld.exe"
        #define MARIADB_CLIENT_EXEC "/mysql.exe"
        #define MONGODB_EXEC "/mongod.exe"
        #define MEMCACHED_EXEC "/memcached.exe"

};

#endif // SERVERS_H
