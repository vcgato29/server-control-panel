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
#include "cli.h"

// global QT includes
#include <QtWidgets>
#include <QtCore>

// main method
int main(int argc, char * argv[])
{
    Q_INIT_RESOURCE(resources);

    // if CLI args are found, the application reacts as a console application
    if (argc > 1) { // first arg is the executable itself
        QApplication app(argc, argv);
        QApplication::setApplicationName(APP_NAME);
        QApplication::setApplicationVersion(APP_VERSION);
        CLI *cli = new CLI;
        cli->handleCommandLineArguments();
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
