#include "json.h"

namespace File
{
    QJsonDocument JSON::load(QString fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        return QJsonDocument::fromJson(file.readAll());
    }

    void JSON::save(QJsonDocument document, QString fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(document.toJson());
        file.close();
    }
}
