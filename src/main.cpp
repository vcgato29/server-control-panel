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
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(APP_NAME);
        QCoreApplication::setApplicationVersion(APP_VERSION);
        handleCommandLineArguments(app);
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
    app.setWindowIcon(QIcon("wpnxm.ico"));

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

void handleCommandLineArguments(QCoreApplication &app)
{
    // lets support multiple cli options, each with different options
    // this should handle, e.g. "wpn-xm.exe --service nginx start"

    QCommandLineParser parser;

    // -h, --help, -?
    QCommandLineOption helpOption(QStringList() << "h" << "help" << "?", "Prints this help message.");
    parser.addOption(helpOption);

    // -v, --version
    QCommandLineOption versionOption(QStringList() << "v" << "version", "Display the version.");
    parser.addOption(versionOption);

    // -s, --service
    QCommandLineOption serviceOption(QStringList() << "s" << "service", "Execute a service.", "[daemon] [command]");
    parser.addOption(serviceOption);

    // call parse() to find out the positional arguments.
    parser.parse(QCoreApplication::arguments());
    const QStringList args = parser.positionalArguments();
    //qDebug() << args;
    const QString command = args.isEmpty() ? QString() : args.first();
    //qDebug() << command;

    // at this point we already have "--service <daemon>", but not <command>//
    if (parser.isSet(serviceOption)) {

        // daemon is the service value
        QString daemon = parser.value(serviceOption);

        if(daemon.isEmpty()) {
            printHelpText(QString("Error: no <daemon> specified."));
        }

        QStringList availableDaemons = QStringList() << "nginx" << "mariadb" << "mongodb";
        if(!availableDaemons.contains(daemon)) {
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

        executeDaemonCommand(daemon, command);
    }

    if(parser.isSet(versionOption)) {
        printf("WPN-XM Server Stack " APP_VERSION " - Command Line Interface \n");
        app.exit(0);
    }

    if(parser.isSet(helpOption)) {
        printHelpText();
    }

    //if(parser.unknownOptionNames().count() > 1) {
        printHelpText(QString("Error: Unknown option."));
    //}
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

void executeDaemonCommand(const QString &daemon, const QString &command)
{
    // instantiate and attach the tray icon to the system tray
    Servers *servers = new Servers();

    if(daemon == "nginx") {
        if(command == "start") { servers->startNginx(); }
        if(command == "stop") { }
        if(command == "restart") { }
    }

    if(daemon == "mariadb") {
        if(command == "start") { }
        if(command == "stop") { }
        if(command == "restart") { }
    }

    if(daemon == "php") {
        if(command == "start") { }
        if(command == "stop") { }
        if(command == "restart") { }
    }

    if(daemon == "mongodb") {
        if(command == "start") { }
        if(command == "stop") { }
        if(command == "restart") { }
    }

    if(daemon == "memcached") {
        if(command == "start") { }
        if(command == "stop") { }
        if(command == "restart") { }
    }

    printHelpText(
       QString("Command not handled, yet! (daemon = %1) (command = %2) \n")
          .arg(daemon.toLocal8Bit().constData(), command.toLocal8Bit().constData())
    );
}
