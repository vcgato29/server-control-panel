#include "downloadmanager.h"

#include <QDebug>

namespace Downloader
{

    DownloadManager::DownloadManager(QObject *parent)
            : QObject(parent)
    {
        downloadID = 0;
        thread     = new DownloadThread();

        connect(thread, SIGNAL(fileReceived(const QString &)), this, SLOT(downloadReceived(const QString&)));
        connect(thread, SIGNAL(fileStarted(const QString &)), this, SLOT(downloadStarted(const QString &)));
        connect(thread, SIGNAL(fileFailed(const QString &)), this, SLOT(downloadFailed(const QString&)));
        connect(thread, SIGNAL(finished()), this, SLOT(moveFinishedDownloads()));
    }

    DownloadManager::~DownloadManager()
    {
        // make sure thread is no longer running
        // deleting a running thread would result in a program crash

        if (thread && !thread->isRunning()) {
            delete thread;
        }
    }

    void DownloadManager::addURL(const QUrl &url, const QString& downloadLocation, const QString& destinationLocation, const QString& destinationName)
    {
        DownloadItem d;
        d.id                    =   downloadID++;
        d.name                  =   url.fileName();
        d.url                   =   url;
        d.downloadLocation      =   downloadLocation;
        d.destinationLocation   =   destinationLocation;
        d.destinationName       =   destinationName;
        d.state                 =   DownloadItem::New;
        fileList.append(d);

        thread->addURL(url, d.downloadLocation + "/" + d.name);
    }

    void DownloadManager::addURL(const QString &url, const QString &downloadLocation, const QString& destinationLocation, const QString& destinationName)
    {
        DownloadItem d;
        d.id                    =   downloadID++;
        d.name                  =   QUrl(url).fileName();
        d.url                   =   url;
        d.downloadLocation      =   downloadLocation;
        d.destinationLocation   =   destinationLocation;
        d.destinationName       =   destinationName;
        d.state                 =   DownloadItem::New;
        fileList.append(d);

        thread->addURL(QUrl(url), d.downloadLocation + "/" + d.name);
    }

    void DownloadManager::addURLs(const QStringList &urlList, const QString &downloadLocation, const QString &destinationLocation)
    {
        foreach(QString s, urlList)
        {
            DownloadItem d;
            d.id                    =   downloadID++;
            d.name                  =   QUrl(s).fileName();
            d.url                   =   s;
            d.downloadLocation      =   downloadLocation;
            d.destinationLocation   =   destinationLocation;
            d.destinationName       =   "";
            d.state                 =   DownloadItem::New;
            fileList.append(d);
        }

        thread->addURLs(urlList);

    }

    void DownloadManager::startDownloads()
    {
        qDebug() << "DownloadManager::startDownloads() Starting downloads...";
        thread->startDownloads();
    }

    void DownloadManager::downloadStarted(const QString &filename)
    {
        qDebug()<<"Download started for filename:"<<filename;
        QMutableListIterator<DownloadItem> i(fileList);
        while (i.hasNext())
        {
            i.next();
            if (i.value().state!=DownloadItem::Successful && i.value().state!=DownloadItem::Failed && i.value().downloadLocation+i.value().name==filename)
            {
                qDebug()<<"starting"<<i.value().downloadLocation+i.value().name<<filename;
                i.value().state=DownloadItem::Started;
                break;
            }
        }
    }

    void DownloadManager::downloadReceived(const QString &filename)
    {
        emit fileReceived(filename);
        qDebug()<<"File Received:"<<filename;
        QMutableListIterator<DownloadItem> i(fileList);
        while (i.hasNext())
        {
            i.next();
            if (i.value().state==DownloadItem::Started && i.value().downloadLocation+i.value().name==filename)
            {
                qDebug()<<"success"<<i.value().downloadLocation+i.value().name<<filename;
                i.value().state=DownloadItem::Successful;
                break;
            }
        }
    }

    void DownloadManager::downloadFailed(const QString &filename)
    {
        emit fileFailed(filename);
        qDebug() << "File download failed:" << filename;
        QMutableListIterator<DownloadItem> i(fileList);
        while (i.hasNext())
        {
            i.next();
            if (i.value().state == DownloadItem::Started && i.value().downloadLocation + i.value().name == filename)
            {
                qDebug() << "fail" << i.value().downloadLocation + i.value().name << filename;
                i.value().state = DownloadItem::Failed;
                break;
            }
        }
    }

    void DownloadManager::moveFinishedDownloads()
    {
        QMutableListIterator<DownloadItem> i(fileList);
        while (i.hasNext())
        {
            i.next();
            qDebug()<<"DownloadManager::moveFinishedDownloads"<<i.value().name<<i.value().url;
            switch (i.value().state)
            {
                case DownloadItem::Successful:
                    {
                        if (i.value().downloadLocation == i.value().destinationLocation) {
                                ;
                            qDebug()<<i.value().name<<"is in"<<i.value().downloadLocation<<"which is the same as"<<i.value().destinationLocation;
                        } else {
                            qDebug()<<"Need to move"<<i.value().name<<"from"<<i.value().downloadLocation<<"to"<<i.value().destinationLocation;
                            QString newName = i.value().name;
                            if (!i.value().destinationName.isEmpty()) {
                               newName = i.value().destinationName;
                            }
                            File::moveFile(i.value().downloadLocation + i.value().name, i.value().destinationLocation+newName);
                        }
                    }
                    break;
                case DownloadItem::Failed:
                    qDebug()<<i.value().name<<"failed :(";
                    break;
                default:
                    //qDebug()<<"State Default for URL:"<<i.value().url;
                    break;
            }
        }
        emit finished();
    }
}
