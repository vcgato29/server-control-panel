#include "downloadthread.h"

#include <QDebug>

namespace Downloader
{
    DownloadThread::DownloadThread(QObject *parent) :
        QThread(parent),
        FilesDownloadedCounter(0),
        FilesToDownloadCounter(0)
    {
        connect(this, SIGNAL(runSignal()), this, SLOT(runSlot()));
    }

    void DownloadThread::run()
    {
        emit(runSignal());
    }

    void DownloadThread::runSlot()
    {
        startNextDownload();
    }

    void DownloadThread::addURL(const QUrl &url, const QString &filename)
    {
        qDebug() << "DownloadThread::addURL \n enqueueing item:";
        qDebug() << "  URL:" << url;
        qDebug() << "  Target filename: " << QDir::toNativeSeparators(filename);

        if (!validateURL(url)) {
            return;
        }

        downloadQueue.enqueue(qMakePair(url, QDir::toNativeSeparators(filename)));
        ++FilesToDownloadCounter;
    }

    void DownloadThread::addURLs(const QStringList &urlList)
    {
        qDebug() << "DownloadThread::addURLs urlList:" << urlList;

        foreach (QUrl url, urlList) {
            addURL(url, url.toLocalFile());
        }
    }

    void DownloadThread::startDownloads()
    {
        if (downloadQueue.isEmpty())
        {
           qDebug() << "DownloadThread::startDownloads : The download queue is empty! Finished.";
           emit finished();
           return;
        }

        start(); /* Start Thread */
    }

    /*QString DownloadThread::saveFileName(const QUrl &url, const QString &location)
    {
        QDir directory(location);
        QString path = url.path();
        QString basename = QFileInfo(path).fileName();

        if (location.isEmpty() || !directory.exists()) {
            if (basename.isEmpty()) {
                basename = "download";
            }
            //basename = ConfigPaths::downloadDir() + basename;
        } else {
            basename = location + basename;
        }

        qDebug()<<"DownloadThread::saveFileName :"<<basename;

        return basename;
    }*/

    void DownloadThread::startNextDownload()
    {
        if (downloadQueue.isEmpty()) {
            qDebug() << FilesDownloadedCounter << "/" << FilesToDownloadCounter << "files downloaded successfully.";
            qDebug() << "DownloadThread::startNextDownload : The download queue is now empty!";
            //printf("%d/%d files downloaded successfully\n", FilesDownloadedCounter, FilesToDownloadCounter);
            FilesDownloadedCounter = FilesToDownloadCounter = 0;
            emit finished();
            return;
        }

        QPair<QUrl, QString> urlPair = downloadQueue.dequeue();
        QString filename = urlPair.second;
        //QString filename = saveFileName(urlPair.first, urlPair.second);
        QString url      = urlPair.first.toEncoded().constData();

        //qDebug() << "\nProcessing item: \n  URL " << url << "\n  File " << filename;

        if (filename.isEmpty()) {
            qDebug() << "File name empty for url:" << url;
            return;
        }

        /*if (QFile::exists(filename)) {
            if (QMessageBox::No == (QMessageBox::question(this->parent(), tr("HTTP"),
                tr("There already exists a file called %1 in the current directory. Overwrite?").arg(filename),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No))) {
                return;
            }
            QFile::remove(filename);
        }*/

        file.setFileName(filename);

        if (!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "Problem opening file '" << filename << "': " << file.errorString();
            startNextDownload();
            return; // error. lets skip this download
        }

        /**
         * The download request
         */
        qDebug() << "Downloading:" << url;

        emit fileStarted(file.fileName());

        netAccessManager = new QNetworkAccessManager(this);
        connect(netAccessManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));

        currentDownloadReply = netAccessManager->get( QNetworkRequest(QUrl(url)) );
        //connect(currentDownloadReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
        connect(currentDownloadReply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
        connect(currentDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    }

    /**
     * 1. Redirect Handling
     * 2. Fix file moving
     * 3. File extraction
     *
     * http://stackoverflow.com/questions/14809310/qnetworkreply-and-301-redirect
     */
    void DownloadThread::downloadFinished()
    {
        file.close();

        if (currentDownloadReply->error()) {
            qDebug() << "Download failed:"<< currentDownloadReply->errorString();
            emit fileFailed(file.fileName());
            if (file.exists()) {
                file.remove();
            }
        } else {
             qDebug()<<"Succeeded.\n";
            ++FilesDownloadedCounter;
             emit fileReceived(file.fileName());
        }

        currentDownloadReply->deleteLater();
        startNextDownload();
    }

    void DownloadThread::replyFinished(QNetworkReply* reply)
    {
        qDebug()<<"DownloadThread::replyFinished:\n";

        QUrl urlRedirectedTo = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

        // If the URL is not empty, we're being redirected.
        if(!urlRedirectedTo.isEmpty()) {
            qDebug()<<"Redirected to URL:\n "<<urlRedirectedTo.toString();

            QUrlQuery query(urlRedirectedTo);
            QString filename = query.queryItemValue("response-content-disposition", QUrl::FullyDecoded).split('=').at(1);
            qDebug()<<"Filename (response-content-disposition): "<<filename;

            // request to redirection url
            addURL(urlRedirectedTo, qApp->applicationDirPath()+ "/data/downloads/"+filename);
        } else {
            // We weren't redirected anymore, means we arrived at the final destination.
            qDebug()<<QString("Final URL (after redirection): ").append(reply->url().toString());
            urlRedirectedTo.clear();
        }
    }

    void DownloadThread::downloadReadyRead()
    {
        file.write(currentDownloadReply->readAll());
    }

    bool DownloadThread::validateURL(QUrl url)
    {
        if (!url.isValid() || url.isEmpty() || url.host().isEmpty())
        {
            qDebug() << "URL invalid:" << url;

            return false;
        }
        return true;
    }

}
