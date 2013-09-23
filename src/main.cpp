/*
    WPN-XM Server Control Panel

    WPN-XM SCP is a tool to manage Nginx, PHP and MariaDb daemons under windows.
    It's a fork of Easy WEMP originally written by Yann Le Moigne and (c) 2010.
    WPN-XM SCP is written by Jens-Andre Koch and (c) 2011 - onwards.

    This file is part of WPN-XM Serverpack for Windows.

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

// global QT includes
#include <QtWidgets>
#include <QtCore>
#include <QCoreApplication>

// main method
int main(int argc, char * argv[])
{
    Q_INIT_RESOURCE(resources);

    // if CLI args are found, the application reacts as a console application
    if (argc > 1) { // first arg is the executable itself
        QCoreApplication app(argc, argv, false);
        handleCommandLineArguments(app.arguments());
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
    app.setWindowIcon(QIcon(":/wpnxm"));

    // do not leave application, until Quit is clicked in the tray menu
    app.setQuitOnLastWindowClosed(false);

    MainWindow mainWindow;
    mainWindow.show();

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

void handleCommandLineArguments(QStringList args)
{
    QString service;
    QString command;

    qDebug() << args;

    /*if (QCoreApplication::arguments().count() < 2) {
        printf("Error. Please provide valid parameters...\n");
        printHelpText();
    } else */
    if ((QCoreApplication::arguments().at(1) == "-h") || (QCoreApplication::arguments().at(1) == "--help")) {
        printHelpText();
    } else if ((QCoreApplication::arguments().at(1) == "-s") || (QCoreApplication::arguments().at(1) == "--service")) {

        // handle "scp.exe --service nginx start"
        if (QCoreApplication::arguments().count() < 3) {
            printf("Error. No service specfied...");
            printHelpText();
        } else if (QCoreApplication::arguments().count() == 3) {
            service = QCoreApplication::arguments().at(3);

        } else if (QCoreApplication::arguments().count() < 4) {
            printf("Error. No command specfied...");
            printHelpText();
        } else if (QCoreApplication::arguments().count() == 4) {
            command = QCoreApplication::arguments().at(4);

        } else if (QCoreApplication::arguments().count() > 5) {
            printf("Error. Too many arguments for -s (--service) <daemon> <command>.");
            printHelpText();
        }

    } else {
        printf("Error. Please provide valid argument...\n");
        printHelpText();
    }

    // definition of known CLI arguments as regular expressions
    QRegExp rxArgStart("--start\\s(\\S+)");
    QRegExp rxArgStop("--stop\\s(\\S+)");

    // loop over string list and check if we have some matches
    for (int i = 1; i < args.size(); ++i) {

        if (rxArgStart.indexIn(args.at(i)) != -1 ) {
            qDebug() << i << ":" << args.at(i) << "Starting " << rxArgStart.capturedTexts();
        }
        else if (rxArgStop.indexIn(args.at(i)) != -1 ) {
            qDebug() << i << ":" << args.at(i) << "Stopping " << rxArgStop.capturedTexts();
        }
        else {
            qDebug() << "Uknown arg:" << args.at(i);
        }

    }
}

void printHelpText()
{
      QString usage = "\n"
    "Usage: " + QCoreApplication::arguments().at(0) + " [option]\n"
    "\n"
    "Options:\n"
      "-v or --version                      Prints the version \n"
      "-h or --help                         Prints this help message \n"
      "-s or --service <daemon> <command>   Executes <command> on <daemon> \n"
         "    [<daemon>]: The name of a daemon, e.g. nginx, mariadb, memcached, mongodb \n"
         "    [<command>]: The command to execute, e.g. start, stop, restart \n"
         "    Ports specified in \"wpnxm.ini\" will be used. \n"
    "\n";

   qDebug() << "Unkown:" << usage;
   //printf(usage);
   exit(-1);
}
