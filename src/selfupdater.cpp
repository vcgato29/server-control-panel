#include "selfupdater.h"

#include "version.h"
#include "windowsapi.h"

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
     * 1. download new version as "wpnxm-scp.new-version.exe"
     * 2. rename running "wpnxm-scp.exe" to "wpnxm-scp.exe.old"
     * 3. rename "wpnxm-scp.new-version.exe" to "wpnxm-scp.exe" (new version replaces old one)
     * 4. indicate the need for a manual restart
     * 5. on (re)start of new exe: remove "wpnxm-scp.exe.old"
     *
     */
    SelfUpdater::SelfUpdater()
    {
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
            switchToNewExecutable();
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

    }

    void SelfUpdater::switchToNewExecutable()
    {
        QString exeName     = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        QString oldExeName	= exeName + ".old";

        QFile::copy(exeName, oldExeName); // wpnxm-scp.exe -> wpnxm-scp.exe.old
        QFile::remove(exeName);

        //QApplication::exit();
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
