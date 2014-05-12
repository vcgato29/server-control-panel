#ifndef CLI_H
#define CLI_H

// for CLI text colors, see inlined Color() method
#include "windows.h"
#include "servers.h"
#include "version.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QDate>

class CLI
{
public:
    CLI();
    void handleCommandLineArguments();
    void printHelpText(QString errorMessage = QString());
    void execDaemons(const QString &command, QCommandLineOption &clioption, QStringList args, QCommandLineParser &parser);
    void colorTest();
    void colorPrint(QString msg, QString colorName = "gray");
};

#endif // CLI_H
