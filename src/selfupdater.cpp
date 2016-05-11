#include "selfupdater.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>

namespace Updater
{

    /**
     * @brief self_update::self_update
     *
     * Self_Update implements a self-update strategy for this executable.
     *
     * 1. check if new version available
     * 2. download new version
     * 3. remove .old file
     * 4. rename running "wpn-xm.exe" to "wpn-xm.exe.old"
     * 5. extract "wpn-xm.exe" (new version replaces old one)
     * 6. indicate the need for a manual restart
     * 7. on (re)start of new exe: remove "wpnxm-scp.exe.old"
     *
     */
    SelfUpdater::SelfUpdater()
    {
        qDebug() << "[SelfUpdater] Started...";
    }

    SelfUpdater::~SelfUpdater()
    {
    }

    void SelfUpdater::run()
    {
        if(updateAvailable())
        {
            qDebug() << "[SelfUpdater] Update available \n VersionInfo:" << versionInfo;
            emit notifyUpdateAvailable(versionInfo);
            downloadNewVersion();
            renameExecutable();
            extract();
            indicateNeedForRestart();
        }
    }

    bool SelfUpdater::updateAvailable()
    {
        versionInfo = getVersionInfo();
        return versionInfo["update_available"].toBool();
    }

    void SelfUpdater::downloadNewVersion()
    {
        QString downloadURL(versionInfo["url"].toString());
        QNetworkRequest request(downloadURL);
        downloadManager.setQueueMode(Downloader::DownloadManager::QueueMode::Serial);
        downloadManager.get(request);
        // finally: invoke downloading
        downloadManager.checkForAllDone();
        //QMetaObject::invokeMethod(&downloadManager, "checkForAllDone", Qt::QueuedConnection);
    }

    void SelfUpdater::extract()
    {
        QUrl url(versionInfo["url"].toString());
        QString zipName(url.fileName());
        QString fileToExtract("wpn-xm.exe");
        QString targetPath(QCoreApplication::applicationDirPath());

        qDebug() << "[SelfUpdater] Extracting " << fileToExtract << "from" << zipName << "to" << targetPath;

        //qDebug() << "[SelfUpdater] Filelist:" << JlCompress::getFileList(zipName);

        JlCompress::extractFile(zipName, fileToExtract, targetPath);
    }

    void SelfUpdater::renameExecutable()
    {
        QString dirPath        = QCoreApplication::applicationDirPath();
        QString exeFilePath    = QCoreApplication::applicationFilePath();
        QString exeName        = QFileInfo(exeFilePath).fileName();
        QString oldExeName	   = exeName + ".old";
        QString oldExeFilePath = dirPath + QDir::separator() + oldExeName;

        qDebug() << "[SelfUpdater] Renaming" << exeFilePath << oldExeFilePath;
        // delete the destination file first (old .old file)
        qDebug() << "[SelfUpdater] delete target first:" << QFile::remove(oldExeFilePath);
        qDebug() << "[SelfUpdater] copy:" << QFile::copy(exeFilePath, oldExeFilePath);     // wpn-xm.exe -> wpn-xm.exe.old
        qDebug() << "[SelfUpdater] remove:" << QFile::remove(exeFilePath);
    }

    void SelfUpdater::indicateNeedForRestart() {

    }

    // ----------------------------------------------------------------

    QJsonObject SelfUpdater::getVersionInfo()
    {
        QString url = getUpdateCheckURL();

        QNetworkAccessManager network;

        // QNAM is non-blocking / non-synchronous, but we want to wait until reply has been received
        // create custom temporary event loop on stack to block the stack until finished received
        QEventLoop eventLoop;

        // "quit()" the event-loop, when the network request "finished()"
        QObject::connect(&network, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

        // the HTTP request
        QNetworkRequest req(url);
        QNetworkReply *updateCheckResponse = network.get(req);

        // run event loop, which blocks the stack, until "finished()" has been called
        eventLoop.exec();

        QJsonDocument jsonResponse;

        if (updateCheckResponse->error() == QNetworkReply::NoError) {

            // read response and parse JSON
            QString strReply = (QString) updateCheckResponse->readAll();
            jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
        }
        else {
            // QNetworkReply::HostNotFoundError
            qDebug() << "Request Failure: " << updateCheckResponse->errorString();

            QMessageBox::critical( QApplication::activeWindow(),
                "Request Failure", updateCheckResponse->errorString(), QMessageBox::Ok
            );
        }

        // cleanup
        delete updateCheckResponse;

        return jsonResponse.object();
    }

    QString SelfUpdater::getUpdateCheckURL()
    {
        QString url("http://wpn-xm.org/updatecheck.php");

        // software
        url.append("?s=");
        if (WindowsAPI::running_on_64_bits_os()) {
            url.append("wpnxmscp-x64");
        } else {
            url.append("wpnxmscp");
        }

        // version
        url.append("&v=");
        QString version(APP_VERSION_SHORT);
        if(version != "@APPVERSIONSHORT@") {
            url.append(version);
        } else {
            url.append("0.8.4"); // hardcoded for local testing (formerly version_localdev.h)
        }

        return url;
    }

}
