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
#include "mainwindow.h"

// global QT includes
#include <QtGui>

// main method
int main(int argc, char * argv[])
{
    Q_INIT_RESOURCE(Resources);

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
