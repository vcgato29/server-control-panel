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
// Local includes
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tray.h"
#include "configurationdialog.h"
#include "settings.h"

// Global includes
#include <QApplication>
#include <QMessageBox>
#include <QSharedMemory>
#include <QRegExp>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QDialogButtonBox>
#include <QCheckbox>

class QCloseEvent;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // disable Maximize functionality
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
    setFixedWidth(620);
    setFixedHeight(380);

    // overrides the window title defined in mainwindow.ui
    setWindowTitle(APP_NAME_AND_VERSION);

    settings.readSettings();
    checkAlreadyActiveDaemons();

    // inital state of status leds is disabled
    ui->label_Nginx_Status->setEnabled(false);
    ui->label_PHP_Status->setEnabled(false);
    ui->label_MariaDb_Status->setEnabled(false);
    ui->label_Memcached_Status->setEnabled(false);
    ui->label_MongoDb_Status->setEnabled(false);

    createActions();

    // The tray icon is an instance of the QSystemTrayIcon class.
    // To check whether a system tray is present on the user's desktop,
    // we call the static QSystemTrayIcon::isSystemTrayAvailable() function.
    if ( false == QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, APP_NAME, tr("You don't have a system tray."));
        QApplication::exit(1);
    }

    createTrayIcon();

    // fetch version numbers from the daemons and set label text accordingly
    ui->label_Nginx_Version->setText( getNginxVersion() );
    ui->label_PHP_Version->setText( getPHPVersion() );
    ui->label_MariaDb_Version->setText( getMariaVersion() );
    ui->label_MongoDB_Version->setText( getMongoVersion() );
    ui->label_Memcached_Version->setText( getMemcachedVersion() );

    // hardcode ports for v0.3.0
    // @todo these ports need to be read from wpnxm.ini
    ui->label_Nginx_Port->setText("80");
    ui->label_PHP_Port->setText("9100");
    ui->label_MariaDb_Port->setText("3306");
    ui->label_MongoDB_Port->setText("123");
    ui->label_Memcached_Port->setText("123");

    showPushButtonsOnlyForInstalledTools();
    enableToolsPushButtons(false);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete trayIcon;
}

void MainWindow::createTrayIcon()
{
    // instantiate and attach the tray icon to the system tray
    trayIcon = new Tray(qApp);

    // the following actions point to SLOTS in the trayIcon object
    // therefore connections must be made, after constructing trayIcon

    // handle clicks on the icon
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    // Connect Actions for Status Table - Column Status
    // if process state of a daemon changes, then change the label status in UI::MainWindow too
    connect(trayIcon, SIGNAL(signalSetLabelStatusActive(QString, bool)),
            this, SLOT(setLabelStatusActive(QString, bool)));

    // Connect Action for, if process state of NGINX and PHP changes,
    // then change the disabled/ enabled state of pushButtons too
    connect(trayIcon, SIGNAL(signalEnableToolsPushButtons(bool)),
            this, SLOT(enableToolsPushButtons(bool)));

    // Connect Actions for Status Table - Column Action (Start)
    connect(ui->pushButton_StartNginx, SIGNAL(clicked()), trayIcon, SLOT(startNginx()));
    connect(ui->pushButton_StartPHP, SIGNAL(clicked()), trayIcon, SLOT(startPhp()));
    connect(ui->pushButton_StartMariaDb, SIGNAL(clicked()), trayIcon, SLOT(startMariaDb()));
    connect(ui->pushButton_StartMongoDb, SIGNAL(clicked()), trayIcon, SLOT(startMongoDb()));
    connect(ui->pushButton_StartMemcached, SIGNAL(clicked()), trayIcon, SLOT(startMemcached()));

    // Connect Actions for Status Table - Column Action (Stop)
    connect(ui->pushButton_StopNginx, SIGNAL(clicked()), trayIcon, SLOT(stopNginx()));
    connect(ui->pushButton_StopPHP, SIGNAL(clicked()), trayIcon, SLOT(stopPhp()));
    connect(ui->pushButton_StopMariaDb, SIGNAL(clicked()), trayIcon, SLOT(stopMariaDb()));
    connect(ui->pushButton_StopMongoDb, SIGNAL(clicked()), trayIcon, SLOT(stopMongoDb()));
    connect(ui->pushButton_StopMemcached, SIGNAL(clicked()), trayIcon, SLOT(stopMemcached()));

     // Connect Actions for Status Table - AllDaemons Start, Stop
    connect(ui->pushButton_AllDaemons_Start, SIGNAL(clicked()), trayIcon, SLOT(startAllDaemons()));
    connect(ui->pushButton_AllDaemons_Stop, SIGNAL(clicked()), trayIcon, SLOT(stopAllDaemons()));

    // finally: show the tray icon
    trayIcon->show();
}

void MainWindow::createActions()
{
    // title bar - minimize
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    // title bar - restore
    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    // title bar - close
    // Note that this action is intercepted by MainWindow::closeEvent()
    // Its modified from "quit" to "close to tray" with a msgbox
    // qApp is global pointer to QApplication
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    // PushButtons:: Website, Mailinglist, ReportBug, Donate
    connect(ui->pushButton_Website, SIGNAL(clicked()), this, SLOT(goToWebsite()));
    connect(ui->pushButton_GoogleGroup, SIGNAL(clicked()), this, SLOT(goToGoogleGroup()));
    connect(ui->pushButton_ReportBug, SIGNAL(clicked()), this, SLOT(goToReportIssue()));
    connect(ui->pushButton_Donate, SIGNAL(clicked()), this, SLOT(goToDonate()));

    // PushButtons: Configuration, Help, About, Close
    connect(ui->pushButton_Configuration, SIGNAL(clicked()), this, SLOT(openConfigurationDialog()));
    connect(ui->pushButton_Help, SIGNAL(clicked()), this, SLOT(openHelpDialog()));
    connect(ui->pushButton_About, SIGNAL(clicked()), this, SLOT(openAboutDialog()));

    // @todo the following action is not intercepted by the closeEvent()
    // connect(ui->pushButton_Close, SIGNAL(clicked()), qApp, SLOT(quit()));
    // workaround is to not quit, but hide the window
    connect(ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(hide()));

    // Actions - Tools
    connect(ui->pushButton_tools_phpinfo, SIGNAL(clicked()), this, SLOT(openToolPHPInfo()));
    connect(ui->pushButton_tools_phpmyadmin, SIGNAL(clicked()), this, SLOT(openToolPHPMyAdmin()));
    connect(ui->pushButton_tools_webgrind, SIGNAL(clicked()), this, SLOT(openToolWebgrind()));
    connect(ui->pushButton_tools_adminer, SIGNAL(clicked()), this, SLOT(openToolAdminer()));

    // Actions - Open Projects Folder
    connect(ui->pushButton_OpenProjects_browser, SIGNAL(clicked()), this, SLOT(openProjectFolderInBrowser()));
    connect(ui->pushButton_OpenProjects_Explorer, SIGNAL(clicked()), this, SLOT(openProjectFolderInExplorer()));

    // Actions - Status Table (Config)
    connect(ui->pushButton_ConfigureNginx, SIGNAL(clicked()), this, SLOT(openConfigurationDialogNginx()));
    connect(ui->pushButton_ConfigurePHP, SIGNAL(clicked()), this, SLOT(openConfigurationDialogPHP()));
    connect(ui->pushButton_ConfigureMariaDb, SIGNAL(clicked()), this, SLOT(openConfigurationDialogMariaDb()));

    // Actions - Status Table (Logs)
    connect(ui->pushButton_ShowLog_NginxAccess, SIGNAL(clicked()), this, SLOT(openLogNginxAccess()));
    connect(ui->pushButton_ShowLog_NginxError, SIGNAL(clicked()), this, SLOT(openLogNginxError()));
    connect(ui->pushButton_ShowLog_PHP, SIGNAL(clicked()), this, SLOT(openLogPHP()));
    connect(ui->pushButton_ShowLog_MongoDb, SIGNAL(clicked()), this, SLOT(openLogMongoDb()));
    connect(ui->pushButton_ShowLog_MariaDb, SIGNAL(clicked()), this, SLOT(openLogMariaDb()));

}

void MainWindow::changeEvent(QEvent *event)
{
    if(0 != event)
    {
        switch (event->type())
        {
            case QEvent::WindowStateChange:
            {
                // minimize to tray (do not minimize to taskbar)
                if (this->windowState() & Qt::WindowMinimized)
                {
                    // @todo provide configuration options to let the user decide on this
                    //if (Preferences::instance().minimizeToTray())
                    //{
                        QTimer::singleShot(0, this, SLOT(hide()));
                    //}
                }

                break;
            }
            default:
                break;
        }
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, APP_NAME,
             tr("The program will keep running in the system tray.<br>"
                "To terminate the program, choose <b>Quit</b> in the context menu of the system tray."));

        // hide mainwindow
        hide();

        // do not propagate the event to the base class
        event->ignore();       
    }
    event->accept();
}

void MainWindow::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        //case QSystemTrayIcon::Trigger:
        //case QSystemTrayIcon::MiddleClick:

        // Double click toggles dialog display
        case QSystemTrayIcon::DoubleClick:
            if( isVisible() )
                // clicking the tray icon, when the main window is shown, hides it
                setVisible(false);
            else {
                // clicking the tray icon, when the main window is hidden, shows the main window
                show();
                setFocus();
                setWindowState( windowState() & ( ~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized ) );
            }
            break;
        default:
            break;
    }
}

void MainWindow::enableToolsPushButtons(bool enabled)
{
    // get all PushButtons from the Tools GroupBox of MainWindow::UI
    QList<QPushButton *> allPushButtonsButtons = ui->ToolsGroupBox->findChildren<QPushButton *>();

    // set all PushButtons enabled/disabled
    for(int i = 0; i < allPushButtonsButtons.size(); ++i)
    {
        allPushButtonsButtons[i]->setEnabled(enabled);
    }

    // change state of "Open Projects Folder" >> "Browser" button
    ui->OpenProjectFolderGroupBox
      ->findChild<QPushButton*>("pushButton_OpenProjects_browser")
      ->setEnabled(enabled);
}

void MainWindow::showPushButtonsOnlyForInstalledTools()
{
    // get all PushButtons from the Tools GroupBox of MainWindow::UI
    QList<QPushButton *> allPushButtonsButtons = ui->ToolsGroupBox->findChildren<QPushButton *>();

    // set all PushButtons invisible
    for(int i = 0; i < allPushButtonsButtons.size(); ++i)
    {
       allPushButtonsButtons[i]->setVisible(false);
    }

    // if tool directory exists, show pushButton

    if(QDir(getProjectFolder() + "/webinterface").exists())
    {
        ui->pushButton_tools_phpinfo->setVisible(true);
    }

    if(QDir(getProjectFolder() + "/phpmyadmin").exists())
    {
        ui->pushButton_tools_phpmyadmin->setVisible(true);
    }

    if(QDir(getProjectFolder() + "/adminer").exists())
    {
        ui->pushButton_tools_adminer->setVisible(true);
    }

    if(QDir(getProjectFolder() + "/webgrind").exists())
    {
        ui->pushButton_tools_webgrind->setVisible(true);
    }
}

void MainWindow::setLabelStatusActive(QString label, bool enabled)
{
    if(label == "nginx") { ui->label_Nginx_Status->setEnabled(enabled); }
    if(label == "php") { ui->label_PHP_Status->setEnabled(enabled); }
    if(label == "mariadb"){ ui->label_MariaDb_Status->setEnabled(enabled); }
    if(label == "mongodb") { ui->label_MongoDb_Status->setEnabled(enabled); }
    if(label == "memcached") { ui->label_Memcached_Status->setEnabled(enabled); }
}

QString MainWindow::getNginxVersion()
{
    QProcess processNginx;
    processNginx.setProcessChannelMode(QProcess::MergedChannels);
    qDebug() << "./bin/nginx/nginx.exe -v";
    processNginx.start("./bin/nginx/nginx.exe -v");

    if (!processNginx.waitForFinished())
        qDebug() << "Nginx Version failed:" << processNginx.errorString();

    QByteArray p_stdout = processNginx.readAll();

    // string for regexp testing
    //QString p_stdout = "nginx version: nginx/1.2.1";

    qDebug() << "Nginx Version: " << p_stdout;

    return parseVersionNumber(p_stdout);
}

QString MainWindow::getMariaVersion()
{
    QProcess processMaria;
    processMaria.setProcessChannelMode(QProcess::MergedChannels);
    processMaria.start("./bin/mariadb/bin/mysqld.exe -V"); // upper-case V

    if (!processMaria.waitForFinished())
        qDebug() << "MariaDb Version failed:" << processMaria.errorString();

    QByteArray p_stdout = processMaria.readAll();

    // test
    //QString p_stdout = "mysql  Ver 15.1 Distrib 5.5.24-MariaDB, for Win32 (x86)";

    qDebug() << "MariaDb Version: " << p_stdout;

    return parseVersionNumber(p_stdout.mid(15));
}

QString MainWindow::getPHPVersion()
{
    QProcess processPhp;
    processPhp.setProcessChannelMode(QProcess::MergedChannels);
    processPhp.start("./bin/php/php.exe -v");

    if (!processPhp.waitForFinished())
        qDebug() << "PHP Version failed:" << processPhp.errorString();

    QByteArray p_stdout = processPhp.readAll();

    // test
    //QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";

    qDebug() << "PHP Version: " << p_stdout;

    return parseVersionNumber(p_stdout);
}

QString MainWindow::getMongoVersion()
{
    QProcess processMongoDB;
    processMongoDB.start("./bin/mongodb/bin/mongod --version");

    if (!processMongoDB.waitForFinished())
        qDebug() << "MongoDB Version failed:" << processMongoDB.errorString();

    QByteArray p_stdout = processMongoDB.readAll();

    //QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";

    qDebug() << "MongoDb Version: " << p_stdout;

    return parseVersionNumber(p_stdout.mid(3)); //21
}

QString MainWindow::getMemcachedVersion()
{
    QProcess processMemcached;
    processMemcached.start("./bin/memcached/memcached.exe -h");

    if (!processMemcached.waitForFinished())
        qDebug() << "Memcached Version failed:" << processMemcached.errorString();

    QByteArray p_stdout = processMemcached.readAll();

    /*QString p_stdout = processMemcached->readAllStandardOutput();*/
    //QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";

    qDebug() << "Memcached Version: " << p_stdout;

    return parseVersionNumber(p_stdout.mid(2)); //10
}

QString MainWindow::parseVersionNumber(QString stringWithVersion)
{
    //qDebug() << stringWithVersion;

    // This RegExp matches version numbers: (\d+\.)?(\d+\.)?(\d+\.)?(\*|\d+)
    // This is the same, but escaped:
    QRegExp regex("(\\d+\\.)?(\\d+\\.)?(\\d+\\.)?(\\*|\\d+)");

    // match
    regex.indexIn(stringWithVersion);

    //qDebug() << regex.cap(0);
    QString cap = regex.cap(0);
    return cap;

// Leave this for debugging reasons
//    int pos = 0;
//    while((pos = regex.indexIn(stringWithVersion, pos)) != -1)
//    {
//        qDebug() << "Match at pos " << pos
//                 << " with length " << regex.matchedLength()
//                 << ", captured = " << regex.capturedTexts().at(0).toLatin1().data()
//                 << ".\n";
//        pos += regex.matchedLength();
//    }
}

void MainWindow::goToWebsite()
{
    QDesktopServices::openUrl(QUrl("http://wpn-xm.org/"));
}

void MainWindow::goToGoogleGroup()
{
    QDesktopServices::openUrl(QUrl("http://groups.google.com/group/wpn-xm/"));
}

void MainWindow::goToReportIssue()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/"));
}

void MainWindow::goToDonate()
{
    QDesktopServices::openUrl(QUrl("http://wpn-xm.org/#donate"));
}

void MainWindow::openToolPHPInfo()
{
    QDesktopServices::openUrl(QUrl("http://localhost/webinterface/index.php?page=phpinfo"));
}

void MainWindow::openToolPHPMyAdmin()
{
    QDesktopServices::openUrl(QUrl("http://localhost/phpmyadmin/"));
}

void MainWindow::openToolWebgrind()
{
    QDesktopServices::openUrl(QUrl("http://localhost/webgrind/"));
}

void MainWindow::openToolAdminer()
{
    QDesktopServices::openUrl(QUrl("http://localhost/adminer/"));
}

void MainWindow::openWebinterface()
{
    QDesktopServices::openUrl(QUrl("http://localhost/webinterface/"));
}

void MainWindow::openProjectFolderInBrowser()
{
    // @todo open only, when Nginx and PHP are running...
    QDesktopServices::openUrl(QUrl("http://localhost"));
}

void MainWindow::openProjectFolderInExplorer()
{
    if(QDir(getProjectFolder()).exists())
    {
        // exec explorer with path to projects
        QDesktopServices::openUrl(QUrl("file:///" + getProjectFolder(), QUrl::TolerantMode));
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("The projects folder was not found."));
    }
}

QString MainWindow::getProjectFolder() const
{
    return QDir::toNativeSeparators(QApplication::applicationDirPath() + "/www");
}

void MainWindow::openConfigurationDialog()
{
    ConfigurationDialog cfgDlg;
    cfgDlg.setWindowTitle("WPN-XM Server Control Panel - Configuration");
    cfgDlg.exec();
}

void MainWindow::openConfigurationDialogNginx()
{
    // Open Configuration Dialog - Tab for Nginx
}

void MainWindow::openConfigurationDialogPHP()
{
    // Open Configuration Dialog - Tab for PHP
}

void MainWindow::openConfigurationDialogMariaDb()
{
    // Open Configuration Dialog - Tab for MariaDb
}

void MainWindow::openConfigurationDialogMongoDb()
{
    // Open Configuration Dialog - Tab for MongoDb
}

void MainWindow::openLogNginxAccess()
{
    if(QFile().exists(qApp->applicationDirPath() + "/logs/access.log") == false) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found!"), QMessageBox::Yes);
    }

    QDesktopServices::openUrl(QUrl("file:///" + qApp->applicationDirPath() + "/logs/access.log", QUrl::TolerantMode));
}

void MainWindow::openLogNginxError()
{
    if(QFile().exists(qApp->applicationDirPath() + "/logs/error.log") == false) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found!"), QMessageBox::Yes);
    }

    QDesktopServices::openUrl(QUrl("file:///" + qApp->applicationDirPath() + "/logs/error.log", QUrl::TolerantMode));
}

void MainWindow::openLogPHP()
{
    if(QFile().exists(qApp->applicationDirPath() + "/logs/php_error.log") == false) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found!"), QMessageBox::Yes);
    }

    QDesktopServices::openUrl(QUrl("file:///" + qApp->applicationDirPath() + "/logs/php_error.log", QUrl::TolerantMode));
}

void MainWindow::openLogMariaDb()
{
    if(QFile().exists(qApp->applicationDirPath() + "/logs/mariadb_error.log") == false) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found!"), QMessageBox::Yes);
    }

    QDesktopServices::openUrl(QUrl("file:///" + qApp->applicationDirPath() + "/logs/mariadb_error.log", QUrl::TolerantMode));
}

void MainWindow::openLogMongoDb()
{
    if(QFile().exists(qApp->applicationDirPath() + "/logs/mongodb.log") == false) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found!"), QMessageBox::Yes);
    }

    QDesktopServices::openUrl(QUrl("file:///" + qApp->applicationDirPath() + "/logs/mongodb.log", QUrl::TolerantMode));
}

void MainWindow::openHelpDialog()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/wiki/Using-the-Server-Control-Panel"));
}

void MainWindow::openAboutDialog()
{
    QMessageBox about(this);
    about.setWindowTitle(tr("About"));
    about.setText(
        tr(
            "<table border=0>"
            "<tr><td colspan=2><img src=\":/wpnxm-logo-dark-transparent\"></img>&nbsp;"
            "<span style=\"display: inline-block; vertical-align: super; top: -20px; font-weight: bold; font-size: 16px;\">v" APP_VERSION "</span>"
            "</td></tr>"
            "<tr><td colspan=2>&nbsp;&nbsp;</td></tr>"
            "<tr><td align=center><b>Website</b></td><td><a href=\"http://wpn-xm.org/\">http://wpn-xm.org/</a><br></td></tr>"
            "<tr><td align=center><b>License</b></td><td>GNU/GPL version 3, or any later version.<br></td></tr>"
            "<tr><td align=center><b>Author(s)</b></td><td>Jens-André Koch (C) 2011 - onwards, <br>Yann Le Moigne (C) 2010.<br></td></tr>"
            "<tr><td align=center><b>Github</b></td><td>WPN-XM is developed on Github.<br><a href=\"https://github.com/WPN-XM/WPN-XM/\">https://github.com/WPN-XM/WPN-XM/</a><br></td></tr>"
            "<tr><td align=center><b>Icons</b></td><td>We are using Yusukue Kamiyamane's Fugue Icon Set.<br><a href=\"http://p.yusukekamiyamane.com/\">http://p.yusukekamiyamane.com/</a><br></td></tr>"
            "<tr><td align=center><b>+1?</b></td><td>If you like using WPN-XM, consider supporting it:<br><a href=\"http://wpn-xm.org/donate.html\">http://wpn-xm.org/#donate</a><br></td></tr>"
            "</td></tr></table>"
            "<br><br>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.<br>"
    ));
    about.setParent(this);
    about.setAutoFillBackground(true);
    about.exec();
}

void MainWindow::checkAlreadyActiveDaemons()
{
    // Check list of active processes for
    // apache, nginx, mysql, php-cgi, memcached
    // and report, if processes are already running.
    // We do this to avoid collisions.

    // Provide a modal dialog with checkboxes for all running processes
    // The user might then select the processes to Leave Running or Shutdown.

    // a) fetch processes via tasklist stdout
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("cmd", QStringList() << "/c tasklist.exe");
    process.waitForFinished();

    // processList contains the tasklist output
    QByteArray processList = process.readAll();
    //qDebug() << "Read" << processList.length() << "bytes";
    //qDebug() << processList;

    // b) define processes to look for
    QStringList processesToSearch;
    processesToSearch << "nginx"
                      << "apache"
                      << "memcached"
                      << "mysqld"
                      << "php-cgi"
                      << "mongod"
                      << "pg_ctl"; // postgresql

    // c) init a list for found processes
    QStringList processesFoundList;

    // d) foreach processesToSearch take a look in the processList
    for (int i = 0; i < processesToSearch.size(); ++i)
    {
        //qDebug() << "Searching for process: " << processesToSearch.at(i).toLocal8Bit().constData() << endl;

        if(processList.contains( processesToSearch.at(i).toLatin1().constData() ))
        {
            // process found
            processesFoundList << processesToSearch.at(i).toLatin1().constData();
        }
    }

    //qDebug() << "Already running Processes found : " << processesFoundList;

    // only show the "process shutdown" dialog, when there are processes to shutdown
    if(false == processesFoundList.isEmpty())
    {
        QLabel *labelA = new QLabel(tr("The following processes are already running:"));
        QGroupBox *groupBox = new QGroupBox(tr("Running Processes"));
        QVBoxLayout *vbox = new QVBoxLayout;

        // iterate over proccesFoundList and draw a "process shutdown" checkbox for each one
        int c = processesFoundList.size();
        for(int i = 0; i < c; ++i) {
           // create checkbox
           QCheckBox *checkbox = new QCheckBox(processesFoundList.at(i));
           checkbox->setChecked(true);
           checkbox->setCheckable(true);
           // add checkbox to view
           vbox->addWidget(checkbox);
        }

        groupBox->setLayout(vbox);

        QLabel *labelB = new QLabel(tr("Please select the processes you wish to shutdown.<br><br>"
                                       "Click Shutdown to shut the selected processes down and continue using the server control panel.<br>"
                                       "To proceed without shuting processes down, click Continue.<br>"));

        QPushButton *ShutdownButton = new QPushButton(tr("Shutdown"));
        QPushButton *ContinueButton = new QPushButton(tr("Continue"));
        ShutdownButton->setDefault(true);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
        buttonBox->addButton(ShutdownButton, QDialogButtonBox::ActionRole);
        buttonBox->addButton(ContinueButton, QDialogButtonBox::ActionRole);

        // e) build dialog to inform user about running processes
        QGridLayout *grid = new QGridLayout;
        grid->addWidget(labelA);
        grid->addWidget(groupBox);
        grid->addWidget(labelB);
        grid->addWidget(buttonBox);

        QDialog *dlg = new QDialog(this);
        dlg->setWindowModality(Qt::WindowModal);
        dlg->setLayout(grid);
        dlg->resize(250, 100);
        dlg->setWindowTitle(tr(APP_NAME));

        // Set signal and slot for "Buttons"
        connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
        connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

        // fire modal window event loop and wait for button clicks
        // if shutdown was clicked (accept), execute shutdowns
        // if continue was clicked (reject), do nothing and proceed to mainwindow
        if(dlg->exec() == QDialog::Accepted)
        {
            // fetch all checkboxes
            QList<QCheckBox *> allCheckBoxes = dlg->findChildren<QCheckBox *>();

            // iterate checkbox values
            c = allCheckBoxes.size();
            for(int i = 0; i < c; ++i) {
               QCheckBox *cb = allCheckBoxes.at(i);
               if(cb->isChecked())
               {
                   //qDebug() << "Shutting down :" << cb->text();

                   QProcess::startDetached("cmd.exe",
                    // taskkill parameters:
                    // /f = force shutdown, /t = structure shutdown, /im = the name of the process
                    // nginx and mariadb need a forced shutdown !
                    QStringList() << "/c" << "taskkill /f /t /im " + cb->text() + ".exe"
                   );
               }
               delete cb;
            }
        }        
        delete vbox;
        delete labelA;
        delete labelB;
        delete ShutdownButton;
        delete ContinueButton;
        delete buttonBox;
        delete groupBox;
        delete grid;
        delete dlg;
    }
}
