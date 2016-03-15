#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QtCore>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QThread>
#include <QQueue>
#include <QPair>
#include <QUrl>
#include <QFile>
#include <QFileInfo>

#include <stdio.h>

namespace Downloader
{
    class DownloadThread: public QThread
    {
        Q_OBJECT

        public:
            DownloadThread(QObject *parent = 0);
            void run();

            void addURL(const QUrl &url, const QString &filename);
            void addURLs(const QStringList &urlList);
            void startDownloads();
            //QString saveFileName(const QUrl &url, const QString &location);

        signals:
            void finished();
            void runSignal();
            void fileStarted(const QString &);
            void fileReceived(const QString &);
            void fileFailed(const QString &);

        private slots:
            void startNextDownload();
            void replyFinished(QNetworkReply* reply);
            void downloadFinished();
            void downloadReadyRead();
            void runSlot();

        private:
            QNetworkAccessManager *netAccessManager;
            QNetworkReply *currentDownloadReply;
            bool validateURL(QUrl url);
            int FilesDownloadedCounter;
            int FilesToDownloadCounter;
            QQueue<QPair<QUrl, QString>> downloadQueue;
            QStringList urlList;
            QFile file;
    };

}

#endif
