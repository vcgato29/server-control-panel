#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "downloadthread.h"
#include "filehandling.h"

#include <QString>
#include <QTimer>
#include <QDialog>
#include <QFile>
#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QQueue>
#include <QStringList>
#include <QThread>
#include <QTime>
#include <QUrl>

namespace Downloader
{
    class DownloadManager: public QObject
    {
        Q_OBJECT

        public:
            DownloadManager(QObject *parent = 0);
            ~DownloadManager();

            void addURL(const QUrl &url, const QString &downloadLocation, const QString& destinationLocation, const QString& destinationName="");
            void addURL(const QString &url, const QString &downloadLocation, const QString& destinationLocation, const QString& destinationName="");
            void addURLs(const QStringList &urlList, const QString &downloadLocation, const QString& destinationLocation);
            void startDownloads();

            struct DownloadItem
            {
                int id;
                QUrl url;
                QString name;
                QString downloadLocation;
                QString destinationLocation;
                QString destinationName;
                typedef enum {New, Started, Paused, Finished, Successful, Failed} DownloadState;
                DownloadState state;
            };

        public slots:
            void downloadStarted(const QString& filename);
            void downloadReceived(const QString& filename);
            void downloadFailed(const QString& filename);

        protected slots:
            void moveFinishedDownloads();

        signals:
            void finished();
            void fileReceived(const QString& t);
            void fileFailed(const QString& t);

        private:
            Downloader::DownloadThread *thread;
            int downloadID;
            QList <DownloadItem> fileList;
    };

}

#endif
