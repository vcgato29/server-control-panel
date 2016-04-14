#include "downloadmanager.h"

#include <QCoreApplication>
#include <QSslError>

#include <QDebug>

namespace Downloader
{
    DownloadManager::DownloadManager()
        : queueMode (Parallel),
          FilesDownloadedCounter(0),
          FilesToDownloadCounter(0)
    {
        connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));

    #ifndef QT_NO_SSL
        connect(&nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    #endif
    }

    DownloadManager::~DownloadManager()
    {
    }

    void DownloadManager::get(const QNetworkRequest &request)
    {
        qDebug() << "DownloadManager::get()" << "Download enqueued.";
        DownloadItem *dl = new DownloadItem(request, nam);
        transfers.append(dl);
        ++FilesToDownloadCounter;

        connect(dl, SIGNAL(downloadProgress(TransferItem*)), SLOT(downloadProgress(TransferItem*)));
        connect(dl, SIGNAL(downloadFinished(TransferItem*)), SLOT(downloadFinished(TransferItem*)));
    }

    // Signal/Slots for the Download Progress:
    // 1. TransferItem::downloadProgress    -> TransferItem::updateDownloadProgress (get progress and do calculations)
    // 2. DownloadItem::downloadProgress    -> DownloadManager::downloadProgress    (move data from item to manager)
    // 3. DownloadManager::downloadProgress -> DownloadManager::signalProgress      (signal data from manager)
    // 4. UpdaterDialog::signalProgress     -> updateProgressBar                    (grab data from manager in dialog)
    void DownloadManager::downloadProgress(TransferItem *item)
    {
        emit signalProgress(item->progress);
    }

    void DownloadManager::finished(QNetworkReply *)
    {
        qDebug() << "DownloadManager::finished()";
    }

    void DownloadManager::downloadFinished(TransferItem *item)
    {
        qDebug() << "Download finished " << item->reply->url() << " with HTTP Status: " << item->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (item->reply->error() != QNetworkReply::NoError) {
            qDebug() << "and error: " << item->reply->error() << item->reply->errorString();
        }
        transfers.removeOne(item);
        ++FilesDownloadedCounter;
        item->deleteLater();
        checkForAllDone();
    }

    void DownloadManager::checkForAllDone()
    {
        if (transfers.isEmpty()) {
            qDebug() << "[Downloader] Download queue is now empty! All Done.";
            FilesDownloadedCounter = FilesToDownloadCounter = 0;
            return;
        }

        foreach (TransferItem *item, transfers) {
            if (!item->reply) {
                item->startRequest();
                // by default multiple downloads are processed in parallel.
                // but in serial mode, only one transfer starts at a time.
                if (queueMode == Serial) {
                    break;
                }
            }
        }
    }

    #ifndef QT_NO_SSL
    void DownloadManager::sslErrors(QNetworkReply *, const QList<QSslError> &errors)
    {
        qDebug() << "sslErrors";
        foreach (const QSslError &error, errors) {
            qDebug() << error.errorString();
            qDebug() << error.certificate().toPem();
        }
    }
    #endif

    TransferItem *DownloadManager::findTransfer(QNetworkReply *reply)
    {
        foreach (TransferItem *item, transfers) {
            if (item->reply == reply) {
                return item;
            }
        }
        return 0;
    }

    void DownloadManager::setQueueMode(QueueMode mode)
    {
        queueMode = mode;
    }
}
