#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QObject>

#include <QNetworkInterface>
#include <QNetworkAddressEntry>

class NetworkUtils
{
    public:
        NetworkUtils();

        static QList<QHostAddress> getLocalHostIPs();

        QString getIpAddress();
        void ping(QString hostname);
        void traceroute(QString hostname);
};

#endif // NETWORKUTILS_H
