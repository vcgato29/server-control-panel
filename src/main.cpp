/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a GUI tool for managing server daemons under Windows.
    It's a fork of Easy WEMP written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Server Stack for Windows.

    WPN-XM SCP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    WPN-XM SCP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WPN-XM SCP. If not, see <http://www.gnu.org/licenses/>.
*/

// local WPN-XM SCP includes
#include "main.h"
#include "version.h"
#include "mainwindow.h"
#include "settings.h"
#include "splashscreen.h"

// for CLI text colors, see inlined Color() method
#include "windows.h"

// global QT includes
#include <QtWidgets>
#include <QtCore>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

// main method
int main(int argc, char * argv[])
{
    Q_INIT_RESOURCE(resources);

    // if CLI args are found, the application reacts as a console application
    if (argc > 1) { // first arg is the executable itself
        QApplication app(argc, argv);
        QApplication::setApplicationName(APP_NAME);
        QApplication::setApplicationVersion(APP_VERSION);
        handleCommandLineArguments();
        return app.exec();
    }

    // else run as a normal QtGUI application
    QApplication app(argc, argv);

    // Single Instance Check
    exitIfAlreadyRunning();

    // Application Meta Data
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName("Jens-André Koch");
    app.setOrganizationDomain("http://wpn-xm.org/");
    app.setWindowIcon(QIcon(":/wpnxm.ico"));

    // splash on
    #ifndef QT_DEBUG
    SplashScreen splash(QPixmap(), Qt::WindowStaysOnTopHint);
    splash.show();
    #endif

    // do not leave application, until Quit is clicked in the tray menu
    app.setQuitOnLastWindowClosed(false);

    MainWindow mainWindow;
    mainWindow.show();

    // splash off
    #ifndef QT_DEBUG
    splash.finish(&mainWindow);
    #endif

    // enter the Qt Event loop here
    return app.exec();
}

/*
 * Single Instance Check
 * Although some people tend to solve this problem by using QtSingleApplication,
 * this approach uses a GUID stored into shared memory.
 */
void exitIfAlreadyRunning()
{
      // Set GUID for WPN-XM Server Control Panel to memory
      // It needs to be "static", because the QSharedMemory instance gets destroyed
      // at the end of the function and so does the shared memory segment.
      static QSharedMemory shared("004d54f6-7d00-4478-b612-f242f081b023");

      // already running
      if( !shared.create( 512, QSharedMemory::ReadWrite) )
      {
          QMessageBox msgBox;
          msgBox.setWindowTitle(APP_NAME);
          msgBox.setText( QObject::tr("WPN-XM is already running.") );
          msgBox.setIcon( QMessageBox::Critical );
          msgBox.exec();

          exit(0);
      }
}

static HANDLE hConsole;
static WORD oldConsoleAttributes = 0;

WORD GetConsoleTextAttribute(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(hConsole, &info);
    return info.wAttributes;
}

static const char *qWinColoredMsg(int prefix, int color, const char *msg)
{
    if(hConsole == NULL) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if (!hConsole) {
        return msg;
    }

    oldConsoleAttributes = GetConsoleTextAttribute(hConsole);

    WORD attr = (oldConsoleAttributes & 0x0f0);

    if (prefix)      attr |= FOREGROUND_INTENSITY;
    if (color == 31) attr |= FOREGROUND_RED | FOREGROUND_INTENSITY;  // red
    if (color == 32) attr |= FOREGROUND_GREEN;                       // green
    if (color == 33) attr |= FOREGROUND_GREEN | FOREGROUND_RED;      // yellow
    if (color == 34) attr |= FOREGROUND_BLUE;                        // blue
    if (color == 35) attr |= FOREGROUND_BLUE | FOREGROUND_RED;       // purple
    if (color == 36) attr |= FOREGROUND_BLUE | FOREGROUND_GREEN;     // cyan
    if (color == 37) attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // white
    if (color == 39) attr |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; // reset

    SetConsoleTextAttribute(hConsole, attr);
    printf(msg);

    SetConsoleTextAttribute(hConsole, oldConsoleAttributes);
    return "";
}

void handleCommandLineArguments()
{
    // lets support multiple cli options, each with different options
    // this should handle, e.g. "wpn-xm.exe --service nginx start"

    QCommandLineParser parser;

    /**
     * Definition of Command Line Arguments
     */

    // -h, --help, -?
    QCommandLineOption helpOption(QStringList() << "h" << "help" << "?", "Prints this help message.");
    parser.addOption(helpOption);

    // -v, --version
    QCommandLineOption versionOption(QStringList() << "v" << "version", "Display the version.");
    parser.addOption(versionOption);

    // -s, --service
    QCommandLineOption serviceOption(QStringList() << "s" << "service", "Install/Uninstall daemon as service.", "[daemon] [command]");
    parser.addOption(serviceOption);

    // -d, --daemon
    QCommandLineOption daemonOption(QStringList() << "d" << "daemon", "Execute a command on daemon.", "[daemon] [command]");
    parser.addOption(daemonOption);

    // --start
    QCommandLineOption startOption("start", "Starts a daemon.", "[daemon/s]");
    parser.addOption(startOption);

    // --stop
    QCommandLineOption stopOption("stop", "Stops a daemon.", "[daemon/s]");
    parser.addOption(stopOption);

    // --restart
    QCommandLineOption restartOption("stop", "Restarts a daemon.", "[daemon/s]");
    parser.addOption(restartOption);

    /**
     * Handling of Command Line Arguments
     */

    // find out the positional arguments.
    parser.parse(QCoreApplication::arguments());
    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();
    qDebug() << "Arguments: " << args << " - " << "Command: " << command;

    // -h, --help, -?
    if(parser.isSet(helpOption)) {
        printHelpText();
    }

    // -v, --version
    if(parser.isSet(versionOption)) {
        printf("WPN-XM Server Stack " APP_VERSION " - Command Line Interface \n");
        exit(0);
    }

    // -s, --service <daemon> <command>, where <command> is on|off
    if (parser.isSet(serviceOption)) {

    }

    // -d, --daemon <daemon> <command>, where <command> is start|stop|restart
    if (parser.isSet(daemonOption)) {

        // at this point we already have "--daemon <daemon>", but not <command>//

        // daemon is the value
        QString daemon = parser.value(daemonOption);

        if(daemon.isEmpty()) {
            printHelpText(QString("Error: no <daemon> specified."));
        }

        Servers *servers = new Servers();

        // check if daemon is whitelisted
        if(!servers->getListOfServerNames().contains(daemon)) {
            printHelpText(
                QString("Error: \"%1\" is not a valid <daemon>.")
                    .arg(daemon.toLocal8Bit().constData())
            );
        }

        if(command.isEmpty()) {
            printHelpText(QString("Error: no <command> specified."));
        }

        QStringList availableCommands = QStringList() << "start" << "stop" << "restart";
        if(!availableCommands.contains(command)) {
            printHelpText(
                QString("Error: \"%1\" is not a valid <command>.")
                    .arg(command.toLocal8Bit().constData())
            );
        }

        QString methodName = command + servers->fixName(daemon);

        if(QMetaObject::invokeMethod(servers, methodName.toLocal8Bit().constData()))
        {
           exit(0);
        }

        printHelpText(
           QString("Command not handled, yet! (daemon = %1) (command = %2) \n")
              .arg(daemon.toLocal8Bit().constData(), command.toLocal8Bit().constData())
        );
        exit(0);
    }

    // --start
    if (parser.isSet(startOption)) {
        execDaemons("start", startOption, args, parser);
    }

    // --stop
    if (parser.isSet(stopOption)) {
        execDaemons("stop", stopOption, args, parser);
    }

    // --restart
    if (parser.isSet(restartOption)) {
        execDaemons("stop", restartOption, args, parser);
    }

    //if(parser.unknownOptionNames().count() > 1) {
        printHelpText(QString("Error: Unknown option."));
    //}
}

/**
 * @brief execDaemons - executes "command" on multiple daemons
 * @param command "start", "stop", "restart"
 * @param clioption
 * @param args
 */
void execDaemons(const QString &command, QCommandLineOption &clioption, QStringList args, QCommandLineParser &parser)
{
    // the value of the key "--start|--stop|--restart" is the first daemon
    QString daemon = parser.value(clioption);

    if(daemon.isEmpty()) {
        printf("%s", qWinColoredMsg(1, 31, "test"));

        //cliColor(YELLOW);
        printHelpText(QString("Error: no <daemon> specified."));

        qWinColoredMsg(1, 33, "xxx");
    }

    // ok, add first daemon to the list
    QStringList daemons;
    daemons << daemon;

    // add the others args
    if(!args.isEmpty()) {
        daemons << args;
    }

    Servers *servers = new Servers();

    for (int i = 0; i < daemons.size(); ++i)
    {
        QString daemon = daemons.at(i);

        // check if daemon is whitelisted
        if(!servers->getListOfServerNames().contains(daemon)) {
            printHelpText(
                QString("Error: \"%1\" is not a valid <daemon>.")
                    .arg(daemon.toLocal8Bit().constData())
            );
        }

        QString methodName = command + servers->fixName(daemon);

        QMetaObject::invokeMethod(servers, methodName.toLocal8Bit().constData());
    }

    exit(0);
}

void printHelpText(const QString &errorMessage)
{
    QString helpText = "\n"
        "WPN-XM Server Stack " APP_VERSION " - Command Line Interface \n"
        "\n"
        "Usage: " + QCoreApplication::arguments().at(0) + " [option] \n"
        "\n"
        "Example: " + QCoreApplication::arguments().at(0) + " --service nginx start \n"
        "\n"
        "Options: \n"
        " -v, --version                        Prints the version. \n"
        " -h, --help                           Prints this help message. \n"
        " -s, --service <daemon> <command>     Executes <command> on <daemon>. \n"
        "\n"
        "Arguments: \n"
        " <daemon>:  The name of a daemon, e.g. nginx, mariadb, memcached, mongodb. \n"
        " <command>: The command to execute, e.g. start, stop, restart. \n"
        "\n"
        "Info :\n"
        " Ports specified in \"wpn-xm.ini\" will be used. \n"
        "\n";

   printf("%s\n%s\n",
          errorMessage.toLocal8Bit().constData(),
          helpText.toLocal8Bit().constData()
   );

   exit(0);
}
