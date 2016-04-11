#include "registrymanager.h"

namespace SoftwareRegistry
{
    Manager::Manager()
    {
        download();
    }

    void Manager::download()
    {
        /**         
         * foreach registry
         *   if (not JSON file exists) and (JSON file not older than updateInterval)
         *     fetch API data as JSON
         *     write to JSON file
         */

        /**
         * Server Stack Software Registry
         */
        QString stackRegistryFile = QDir::currentPath() + "/bin/updatecheck.json";

        if(fileNotExistingOrOutdated(stackRegistryFile)) {
            downloadRegistry(QUrl("http://wpn-xm.org/updatecheck.php?s=all"), stackRegistryFile);
        } else {
            qDebug() << "[Loading from Cache] Server Stack Software Registry";
            stackSoftwareRegistry = File::JSON::load(stackRegistryFile);
        }

        // php registry
        // metadata
    }

    void Manager::downloadRegistry(QUrl url, QString file)
    {
        QNetworkAccessManager network;

        // create custom temporary event loop on stack
        QEventLoop eventLoop;

        // "quit()" the event-loop, when the network request "finished()"
        QObject::connect(&network, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

        // the HTTP request
        QNetworkRequest req(url);
        QNetworkReply *updateCheckResponse = network.get(req);

        // run event loop, which blocks the stack, until "finished()" has been called
        eventLoop.exec();

        if (updateCheckResponse->error() == QNetworkReply::NoError) {

            // read response and parse JSON
            QString strReply = (QString) updateCheckResponse->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

            // save JSON to file
            File::JSON::save(jsonResponse, file);
        }
        else {
            qDebug() << "Request Failure: " << updateCheckResponse->errorString();
        }

        // cleanup pointer
        delete updateCheckResponse;
    }

    bool Manager::fileNotExistingOrOutdated(QString fileName)
    {
        if(!QFile::exists(fileName)) {
            return false;
        }

        QDateTime fileModificationDate = QFileInfo(fileName).lastModified();
        QDateTime today                = QDateTime::currentDateTime();

        // updateInterval hardcoded to 3 days
        if(3 <= fileModificationDate.daysTo(today)) {
            return true;
        }

        return false;
    }

    QJsonObject Manager::getStackSoftwareRegistry()
    {
        return stackSoftwareRegistry.object();
    }

    /*
    QJsonObject RegistriesDownloader::getPhpSoftwareRegistry()
    {
        return phpSoftwareRegistry.object();
    }*/

}
