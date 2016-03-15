#ifndef JSON_H
#define JSON_H

#include <QJsonDocument>
#include <QFile>

namespace File
{
    class JSON
    {
        public:
            static void save(QJsonDocument document, QString fileName);
            static QJsonDocument load(QString fileName);
    };
}
#endif // JSON_H
