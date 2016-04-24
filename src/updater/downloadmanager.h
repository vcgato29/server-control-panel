#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QElapsedTimer>

namespace Downloader
{
    class TransferItem : public QObject
    {
            Q_OBJECT
        public:
            TransferItem(const QNetworkRequest &r, QNetworkAccessManager &n);
            void startRequest();
        signals:
            void downloadFinished(TransferItem *self);
            void downloadProgress(QMap<QString, QVariant>);
            void finished(TransferItem *self);
        public slots:
            void updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        private slots:
            void finished();
        public:
            QNetworkRequest request;
            QNetworkReply *reply;
            QNetworkAccessManager &nam;
            QFile *inputFile;
            QFile *outputFile;
            QList<QUrl> redirects;
            QElapsedTimer timer;
            QMap<QString, QVariant> progress;

        private:
            QString getSizeHumanReadable(qint64 bytes);
    };

    class DownloadItem : public TransferItem
    {
            Q_OBJECT
        public:
            DownloadItem(const QNetworkRequest &r, QNetworkAccessManager &nam);
            ~DownloadItem();           
        private slots:
            void readyRead();
            void finished();
    };

    class DownloadManager : public QObject
    {
            Q_OBJECT
        public:
            DownloadManager();
            ~DownloadManager();
            void get(const QNetworkRequest &request);
            enum QueueMode { Parallel, Serial };
            void setQueueMode(QueueMode mode);
            TransferItem *findTransfer(QUrl url);

        public slots:
            void checkForAllDone();

        private slots:
            void finished(QNetworkReply *reply);
        #ifndef QT_NO_SSL
            void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
        #endif
            void downloadFinished(TransferItem *item);
        private:
            TransferItem *findTransfer(QNetworkReply *reply);

            QNetworkAccessManager nam;
            QList<TransferItem*> transfers;
            QueueMode queueMode;           

        public:
            int FilesDownloadedCounter;
            int FilesToDownloadCounter;
    };
}

#endif // DOWNLOADMANAGER_H
