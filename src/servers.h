#ifndef SERVERS_H
#define SERVERS_H

#include "settings.h"

#include <QProcess>
#include <QMenu>

class Server: public QObject
{
    Q_OBJECT

public:
        QString lowercaseName;
        QString name;
        QIcon icon;
        QString workingDirectory;
        QStringList configFiles;
        QStringList logFiles;
        QString exe;
        QMenu *trayMenu;

        // Process Monitoring
        QProcess *process;
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
        QStringList getListOfServerNamesInstalled();
        QString getCamelCasedServerName(QString &serverName) const;
        Server *getServer(const char *serverName) const;
        QProcess *getProcess(const char *serverName) const;
        QProcess::ProcessState getProcessState(const char *serverName) const;
        QString getExecutable(QString &serverName) const;

        QStringList getLogFiles(QString &serverName) const;

        bool truncateFile(const QString &file) const;
        void clearLogFile(const QString &serverName) const;

public slots:

        void showProcessError(QProcess::ProcessError error);

        // Status Action Slots
        void updateProcessStates(QProcess::ProcessState state);

        // slot action handling the clicks on daemon commands in the tray menu
        void mapAction(QAction *action);

        // Nginx Action Slots
        void startNginx();
        void stopNginx();
        void reloadNginx();
        void restartNginx();

        // PHP Action Slots
        void startPHP();
        void stopPHP();
        void restartPHP();

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

        // PostgreSQL Action Slots
        void startPostgreSQL();
        void stopPostgreSQL();
        void restartPostgreSQL();

        void updateVersion(QString server);

signals:
        // following signal is connected to MainWindow::setLabelStatusActive()
        void signalSetLabelStatusActive(QString label, bool enabled);
        void signalEnableToolsPushButtons(bool enabled);

private:
        QList<Server *> serverList;

        QString getProcessErrorMessage(QProcess::ProcessError);
};

#endif // SERVERS_H
