#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include "version.h"
#include "windowsapi.h"
#include "updater/downloadmanager.h"
#include "settings.h"

#include "third-party/quazip/quazip/JlCompress.h"

#include <QJsonObject>
#include <QObject>
#include <QFileInfo>
#include <QCoreApplication>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QIcon>

namespace Updater
{
    class SelfUpdater : public QObject
    {
        Q_OBJECT

        public:
            SelfUpdater();
            ~SelfUpdater();

            void run();
            bool updateAvailable();
            void doUpdate();

            void downloadNewVersion();
            void renameExecutable();            

            void askForRestart();

        public slots:
            void extract();
            void askForUpdate();

        private:
            QString getUpdateCheckURL();
            QJsonObject getVersionInfo();
            QJsonObject versionInfo;
            QString downloadFolder;
            bool userRequestedUpdate;

        protected:
            Downloader::DownloadManager  downloadManager;
            QNetworkAccessManager        network;
            Settings::SettingsManager    *settings;

        signals:
            QJsonObject notifyUpdateAvailable(QJsonObject versionInfo);
            QJsonObject notifyRestartNeeded(QJsonObject versionInfo);

    };

}

#endif // SELF_UPDATE_H
