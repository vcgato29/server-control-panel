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
#include <QDesktopWidget>
#include <QDate>

class QCloseEvent;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // The tray icon is an instance of the QSystemTrayIcon class.
    // To check whether a system tray is present on the user's desktop,
    // we call the static QSystemTrayIcon::isSystemTrayAvailable() function.
    if ( false == QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, APP_NAME, tr("You don't have a system tray."));
        QApplication::exit(1);
    }

    setDefaultSettings();

    // overrides the window title defined in mainwindow.ui
    setWindowTitle(APP_NAME_AND_VERSION);

    // start minimized to tray
    if(settings->get("global/startminimized").toBool()) {
        setWindowState( Qt::WindowMinimized );
    } else {
        // maximize and move window to the top
        setFocus();
        setWindowState( windowState() & ( ~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized ) );
    }

    // disable Maximize button functionality
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

    this->servers = new Servers();

    createTrayIcon();

    renderInstalledDaemons();

    checkAlreadyActiveDaemons();

    showPushButtonsOnlyForInstalledTools();

    // daemon autostart
    if(settings->get("global/autostartdaemons").toBool()) {
        qDebug() << "[Daemons] Autostart enabled";
        autostartDaemons();
    };

    createActions();

    if(ui->centralWidget->findChild<QLabel*>("label_Nginx_Status")->isEnabled() &&
       ui->centralWidget->findChild<QLabel*>("label_PHP_Status")->isEnabled()) {
      enableToolsPushButtons(true);
    } else {
      enableToolsPushButtons(false);
    }

    // set window size fixed
    //qDebug() << ui->ToolsGroupBox->geometry();
    //qDebug() << ui->ToolsGroupBox->frameGeometry();
    //qDebug() << height();
    setFixedSize(width(), height());
}

MainWindow::~MainWindow()
{
    // stop all daemons, when quitting the tray application
    if(settings->get("global/stopdaemonsonquit").toBool()) {
        qDebug() << "[Daemons] Stopping All Daemons on Quit...";
        stopAllDaemons();
    }

    delete ui;
    delete tray;
}

void MainWindow::createTrayIcon()
{
    // instantiate and attach the tray icon to the system tray
    tray = new Tray(qApp, servers, settings);
    tray->setIcon(QIcon(":/wpnxm"));

    // the following actions point to SLOTS in the tray object
    // therefore connections must be made, after instantiating Tray

    // handle clicks on the icon
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    // Connect Actions for Status Table - Column Status
    // if process state of a daemon changes, then change the label status in UI::MainWindow too
    connect(servers, SIGNAL(signalSetLabelStatusActive(QString, bool)),
            this, SLOT(setLabelStatusActive(QString, bool)));

    // Connect Action for, if process state of NGINX and PHP changes,
    // then change the disabled/ enabled state of pushButtons too
    connect(servers, SIGNAL(signalEnableToolsPushButtons(bool)),
            this, SLOT(enableToolsPushButtons(bool)));

    // finally: show the tray icon
    tray->show();
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
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApplication()));

    // Connect Actions for Status Table - Column Action (Start)
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_Nginx"), SIGNAL(clicked()), servers, SLOT(startNginx()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_PHP"), SIGNAL(clicked()), servers, SLOT(startPHP()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_MariaDb"), SIGNAL(clicked()), servers, SLOT(startMariaDb()));
    QPushButton *buttonStartMongoDb =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_MongoDb");
    if(buttonStartMongoDb != 0) { connect(buttonStartMongoDb, SIGNAL(clicked()), servers, SLOT(startMongoDb())); }
    QPushButton *buttonStartMemcached =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_Memcached");
    if(buttonStartMemcached != 0) { connect(buttonStartMemcached, SIGNAL(clicked()), servers, SLOT(startMemcached())); }
    QPushButton *buttonStartPostgreSQL =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_PostgreSQL");
    if(buttonStartPostgreSQL != 0) { connect(buttonStartPostgreSQL, SIGNAL(clicked()), servers, SLOT(startPostgreSQL())); }

    // Connect Actions for Status Table - Column Action (Stop)
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_Nginx"), SIGNAL(clicked()), servers, SLOT(stopNginx()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_PHP"), SIGNAL(clicked()), servers, SLOT(stopPHP()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_MariaDb"), SIGNAL(clicked()), servers, SLOT(stopMariaDb()));
    QPushButton *buttonStopMongoDb =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_MongoDb");
    if(buttonStopMongoDb != 0) { connect(buttonStopMongoDb, SIGNAL(clicked()), servers, SLOT(stopMongoDb())); }
    QPushButton *buttonStopMemcached =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_Memcached");
    if(buttonStopMemcached != 0) { connect(buttonStopMemcached, SIGNAL(clicked()), servers, SLOT(stopMemcached())); }
    QPushButton *buttonStopPostgreSQL =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_PostgreSQL");
    if(buttonStopPostgreSQL != 0) { connect(buttonStopPostgreSQL, SIGNAL(clicked()), servers, SLOT(stopPostgreSQL())); }

    // Connect Actions for Status Table - AllDaemons Start, Stop
    connect(ui->pushButton_AllDaemons_Start, SIGNAL(clicked()), this, SLOT(startAllDaemons()));
    connect(ui->pushButton_AllDaemons_Stop, SIGNAL(clicked()), this, SLOT(stopAllDaemons()));

    // PushButtons:: Website, Mailinglist, ReportBug, Donate
    connect(ui->pushButton_Website, SIGNAL(clicked()), this, SLOT(goToWebsite()));
    connect(ui->pushButton_GoogleGroup, SIGNAL(clicked()), this, SLOT(goToGoogleGroup()));
    connect(ui->pushButton_ReportBug, SIGNAL(clicked()), this, SLOT(goToReportIssue()));
    connect(ui->pushButton_Donate, SIGNAL(clicked()), this, SLOT(goToDonate()));

    // PushButtons: Configuration, Help, About, Close
    connect(ui->pushButton_Webinterface, SIGNAL(clicked()), this, SLOT(openProjectFolderInBrowser()));
    connect(ui->pushButton_Configuration, SIGNAL(clicked()), this, SLOT(openConfigurationDialog()));
    connect(ui->pushButton_Help, SIGNAL(clicked()), this, SLOT(openHelpDialog()));
    connect(ui->pushButton_About, SIGNAL(clicked()), this, SLOT(openAboutDialog()));
    // clicking Close, does not quit, but closes the window to tray
    connect(ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(hide()));

    // Actions - Tools
    connect(ui->pushButton_tools_phpinfo, SIGNAL(clicked()), this, SLOT(openToolPHPInfo()));
    connect(ui->pushButton_tools_phpmyadmin, SIGNAL(clicked()), this, SLOT(openToolPHPMyAdmin()));
    connect(ui->pushButton_tools_webgrind, SIGNAL(clicked()), this, SLOT(openToolWebgrind()));
    connect(ui->pushButton_tools_adminer, SIGNAL(clicked()), this, SLOT(openToolAdminer()));

    // Actions - Open Projects Folder
    connect(ui->pushButton_OpenProjects_browser, SIGNAL(clicked()), this, SLOT(openProjectFolderInBrowser()));
    connect(ui->pushButton_OpenProjects_Explorer, SIGNAL(clicked()), this, SLOT(openProjectFolderInExplorer()));

    // Actions - Status Table

    // Configuration via Webinterface
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_Nginx"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogNginx()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_PHP"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogPHP()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_MariaDb"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogMariaDb()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_MongoDb"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogMongoDb()));
    connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_PostgreSQL"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogPostgresql()));
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
                    QTimer::singleShot(0, this, SLOT(hide()));
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
    if (tray->isVisible()) {

        bool doNotAskAgainCloseToTray = settings->get("global/donotaskagainclosetotray").toBool();

        if(!doNotAskAgainCloseToTray)
        {
            QCheckBox *checkbox = new QCheckBox(tr("Do not show this message again."), this);
            checkbox->setChecked(doNotAskAgainCloseToTray);

            QMessageBox msgbox(this);
            msgbox.setWindowTitle(qApp->applicationName());
            msgbox.setIconPixmap(QMessageBox::standardIcon(QMessageBox::Information));
            msgbox.setText(
               tr("This program will keep running in the system tray.<br>"
                  "To terminate the program, choose <b>Quit</b> in the context menu of the system tray.")
            );
            msgbox.setCheckBox(checkbox);
            msgbox.exec();

            settings->set("global/donotaskagainclosetotray", int(msgbox.checkBox()->isChecked()));
        }

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

    // the following two buttons provide the same functionality
    // they open up the webinterface in the browser
    // change state of "Open Projects Folder" >> "Browser" button
    ui->pushButton_OpenProjects_browser->setEnabled(enabled);
    // change state of "Rightside Toolbar" >> "Webinterface" button
    ui->pushButton_Webinterface->setEnabled(enabled);

    // disable "webinterface" in TrayMenu, if PHP/Nginx is off
    QList<QAction *>actions = tray->contextMenu()->actions();
    const int listSize = actions.size();
    for (int i = 0; i < listSize; ++i) {
       if(actions.at(i)->iconText() == tr("Webinterface")) {
            actions.at(i)->setEnabled(enabled);
       }
    }

    // webinterface configuration is only available, when nginx and php are running
    // disable "pushButton_Configure_*"
    QList<QPushButton *> allConfigurePushButtonsButtons = ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_Configure_\\w"));

    // set all PushButtons enabled/disabled
    for(int i = 0; i < allConfigurePushButtonsButtons.size(); ++i)
    {
        allConfigurePushButtonsButtons[i]->setEnabled(enabled);
    }
}

void MainWindow::showPushButtonsOnlyForInstalledTools()
{
    // get all PushButtons from the Tools GroupBox of MainWindow::UI
    QList<QPushButton *> allPushButtonsButtons = ui->ToolsGroupBox->findChildren<QPushButton *>();

    // set all PushButtons invisible
    for(int i = 0; i < allPushButtonsButtons.size(); ++i) {
        allPushButtonsButtons[i]->setVisible(false);
    }

    // if tool directory exists, show pushButtons in the Tools Groupbox
    if(QDir(getProjectFolder() + "/tools/webinterface").exists()) { ui->pushButton_tools_phpinfo->setVisible(true);  }
    if(QDir(getProjectFolder() + "/tools/phpmyadmin").exists())   { ui->pushButton_tools_phpmyadmin->setVisible(true); }
    if(QDir(getProjectFolder() + "/tools/adminer").exists())      { ui->pushButton_tools_adminer->setVisible(true); }
    if(QDir(getProjectFolder() + "/tools/webgrind").exists())     { ui->pushButton_tools_webgrind->setVisible(true); }
}

void MainWindow::setLabelStatusActive(QString label, bool enabled)
{
    label = label.toLower();
    if(label == "nginx")                        { ui->centralWidget->findChild<QLabel*>("label_Nginx_Status")->setEnabled(enabled); }
    if(label == "php" || label == "php-cgi")    { ui->centralWidget->findChild<QLabel*>("label_PHP_Status")->setEnabled(enabled); }
    if(label == "mariadb")                      { ui->centralWidget->findChild<QLabel*>("label_MariaDb_Status")->setEnabled(enabled); }
    if(label == "mongodb" || label == "mysqld") { ui->centralWidget->findChild<QLabel*>("label_MongoDb_Status")->setEnabled(enabled); }
    if(label == "memcached")                    { ui->centralWidget->findChild<QLabel*>("label_Memcached_Status")->setEnabled(enabled); }
    if(label == "postgresql" || label == "postgres") { ui->centralWidget->findChild<QLabel*>("label_PostgreSQL_Status")->setEnabled(enabled); }
}

void MainWindow::quitApplication()
{
    if(settings->get("global/stopdaemonsonquit").toBool()) {
        qDebug() << "[Daemons] Stopping on Quit...\n";
        stopAllDaemons();
    }
    qApp->quit();
}

QString MainWindow::getNginxVersion()
{
    // this happens only during testing
    if(!QFile().exists("./bin/nginx/nginx.exe")) {
        return "0.0.0";
    }

    QProcess processNginx;
    processNginx.setProcessChannelMode(QProcess::MergedChannels);
    processNginx.start("./bin/nginx/nginx.exe -v");

    if (!processNginx.waitForFinished()) {
        qDebug() << "[Nginx] Version failed:" << processNginx.errorString();
        return "";
    }

    QByteArray p_stdout = processNginx.readAll();

    // string for regexp testing
    //QString p_stdout = "nginx version: nginx/1.2.1";

    qDebug() << "[Nginx] Version: \n" << p_stdout;

    return parseVersionNumber(p_stdout);
}

QString MainWindow::getMariaVersion()
{
    // this happens only during testing
    if(!QFile().exists("./bin/mariadb/bin/mysqld.exe")) {
        return "0.0.0";
    }

    QProcess processMaria;
    processMaria.setProcessChannelMode(QProcess::MergedChannels);
    processMaria.start("./bin/mariadb/bin/mysqld.exe -V"); // upper-case V

    if (!processMaria.waitForFinished()) {
        qDebug() << "[MariaDb] Version failed:" << processMaria.errorString();
        return "";
    }

    QByteArray p_stdout = processMaria.readAll();

    // string for regexp testing
    //QString p_stdout = "mysql  Ver 15.1 Distrib 5.5.24-MariaDB, for Win32 (x86)";

    qDebug() << "[MariaDb] Version: \n" << p_stdout;

    return parseVersionNumber(p_stdout.mid(15));
}

QString MainWindow::getPHPVersion()
{
    // this happens only during testing
    if(!QFile().exists("./bin/php/php.exe")) {
        return "0.0.0";
    }

    QProcess processPhp;
    processPhp.setProcessChannelMode(QProcess::MergedChannels);
    processPhp.start("./bin/php/php.exe -v");

    if (!processPhp.waitForFinished()) {
        qDebug() << "[PHP] Version failed:" << processPhp.errorString();
        return "";
    }

    QByteArray p_stdout = processPhp.readAll();

    // remove everything before "PHP" (e.g. warnings from false php.ini settings, etc)
    p_stdout.remove(0, p_stdout.indexOf("PHP"));

    // string for regexp testing
    //QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";

    qDebug() << "[PHP] Version: \n" << p_stdout;

    return parseVersionNumber(p_stdout);
}

QString MainWindow::getMongoVersion()
{
    QProcess processMongoDB;
    processMongoDB.start("./bin/mongodb/bin/mongod.exe --version");

    if (!processMongoDB.waitForFinished()) {
        qDebug() << "[MongoDB] Version failed:" << processMongoDB.errorString();
        return "";
    }

    QByteArray p_stdout = processMongoDB.readLine();

    // string for regexp testing
    //QString p_stdout = "----";

    qDebug() << "[MongoDb] Version: \n" << p_stdout;

    return parseVersionNumber(p_stdout.mid(3)); //21
}

QString MainWindow::getPostgresqlVersion()
{
    QProcess process;
    process.start("./bin/pgsql/bin/pg_ctl.exe -V");

    if (!process.waitForFinished()) {
        qDebug() << "[PostgreSQL] Version failed:" << process.errorString();
        return "";
    }

    QByteArray p_stdout = process.readAll();

    qDebug() << "[PostgreSQL] Version: \n" << p_stdout;

    return parseVersionNumber(p_stdout.mid(2)); //10
}

QString MainWindow::getMemcachedVersion()
{
    QProcess processMemcached;
    processMemcached.start("./bin/memcached/memcached.exe -h");

    if (!processMemcached.waitForFinished()) {
        qDebug() << "[Memcached] Version failed:" << processMemcached.errorString();
        return "";
    }

    QByteArray p_stdout = processMemcached.readLine();

    qDebug() << "[Memcached] Version: \n" << p_stdout;

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

//*
//* Action slots
//*
void MainWindow::startAllDaemons()
{
    servers->startNginx();
    servers->startPHP();
    servers->startMariaDb();
    servers->startMongoDb();
    servers->startMemcached();
    servers->startPostgreSQL();
}

void MainWindow::stopAllDaemons()
{
    servers->stopMariaDb();
    servers->stopPHP();
    servers->stopNginx();
    servers->stopMongoDb();
    servers->stopMemcached();
    servers->stopPostgreSQL();
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
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=phpinfo"));
}

void MainWindow::openToolPHPMyAdmin()
{
    QDesktopServices::openUrl(QUrl("http://localhost/tools/phpmyadmin/"));
}

void MainWindow::openToolWebgrind()
{
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webgrind/"));
}

void MainWindow::openToolAdminer()
{
    QDesktopServices::openUrl(QUrl("http://localhost/tools/adminer/adminer.php"));
}

void MainWindow::openWebinterface()
{
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/"));
}

void MainWindow::openProjectFolderInBrowser()
{
    QDesktopServices::openUrl(QUrl("http://localhost"));
}

void MainWindow::openProjectFolderInExplorer()
{
    if(QDir(getProjectFolder()).exists()) {
        // exec explorer with path to projects
        QDesktopServices::openUrl(QUrl("file:///" + getProjectFolder(), QUrl::TolerantMode));
    } else {
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
    cfgDlg.setServers(servers->getListOfServerNamesInstalled());
    cfgDlg.setWindowTitle("WPN-XM Server Control Panel - Configuration");
    cfgDlg.exec();
}

void MainWindow::openConfigurationDialogNginx()
{
    // Open Configuration Dialog - Tab for Nginx
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#nginx"));
}

void MainWindow::openConfigurationDialogPHP()
{
    // Open Configuration Dialog - Tab for PHP
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#php"));
}

void MainWindow::openConfigurationDialogMariaDb()
{
    // Open Configuration Dialog - Tab for MariaDb
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#mariadb"));
}

void MainWindow::openConfigurationDialogMongoDb()
{
    // Open Configuration Dialog - Tab for MongoDb
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#mongodb"));
}

void MainWindow::openConfigurationDialogPostgresql()
{
    // Open Configuration Dialog - Tab for MongoDb
    QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#postgresql"));
}

void MainWindow::openConfigurationInEditor()
{
    QPushButton *button = (QPushButton *)sender();
    QString obj = button->objectName();

    // extract server name from pushbutton name (pushButton_ConfigurationEditor_server => server)
    QStringList pieces = obj.split("_");
    QString serverName = pieces.value( pieces.length() - 1);

    // fetch config file for server from the ini
    QString cfgfile = QDir(settings->get(serverName + "/config").toString()).absolutePath();

    if(!QFile().exists(cfgfile)) {
        QMessageBox::warning(this, tr("Warning"), tr("Config file not found: \n") + cfgfile, QMessageBox::Yes);
    } else {
       QDesktopServices::setUrlHandler("file", this, "execEditor");

       // if no UrlHandler is set, this executes the OS-dependend scheme handler
       QDesktopServices::openUrl(QUrl::fromLocalFile(cfgfile));

       QDesktopServices::unsetUrlHandler("file");
    }
}

void MainWindow::openLog()
{
    // we have a incoming SIGNAL object to this SLOT
    // we use the object name, e.g. pushButton_ShowLog_Nginx or pushButton_ShowErrorLog_Nginx
    // to map the log file

    QPushButton *button = (QPushButton *)sender();
    QString obj = button->objectName();

    QString logs = QDir(settings->get("paths/logs").toString()).absolutePath();
    QString logfile = "";

    if(obj == "pushButton_ShowLog_Nginx")        { logfile = logs + "/access.log";}
    if(obj == "pushButton_ShowErrorLog_Nginx")   { logfile = logs + "/error.log";}
    if(obj == "pushButton_ShowErrorLog_PHP")     { logfile = logs + "/php_error.log";}
    if(obj == "pushButton_ShowErrorLog_MariaDb") { logfile = logs + "/mariadb_error.log";}
    if(obj == "pushButton_ShowLog_MongoDb")      { logfile = logs + "/mongodb.log";}
    if(obj == "pushButton_ShowLog_PostgreSQL")   { logfile = logs + "/postgresql.log";}

    if(!QFile().exists(logfile)) {
        QMessageBox::warning(this, tr("Warning"), tr("Log file not found: \n") + logfile, QMessageBox::Yes);
    } else {
       QDesktopServices::setUrlHandler("file", this, "execEditor");

       // if no UrlHandler is set, this executes the OS-dependend scheme handler
       QDesktopServices::openUrl(QUrl::fromLocalFile(logfile));

       QDesktopServices::unsetUrlHandler("file");
    }
}

void MainWindow::execEditor(QUrl logfile)
{
    QProcess *process = new QProcess(this);
    QString program = settings->get("global/editor").toString();
    qDebug() << logfile.toLocalFile();
    process->start(program, QStringList() << logfile.toLocalFile());
}

void MainWindow::openHelpDialog()
{
    QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/wiki/Using-the-Server-Control-Panel"));
}

void MainWindow::openAboutDialog()
{
    QString year = QDate::currentDate().toString("yyyy");

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
            "<tr><td align=center><b>Author(s)</b></td><td>Jens-André Koch (C) 2011 - ").append(year).append(", <br>Yann Le Moigne (C) 2010.<br></td></tr>"
            "<tr><td align=center><b>Github</b></td><td>WPN-XM is developed on Github.<br><a href=\"https://github.com/WPN-XM/WPN-XM/\">https://github.com/WPN-XM/WPN-XM/</a><br></td></tr>"
            "<tr><td align=center><b>Icons</b></td><td>We are using Yusukue Kamiyamane's Fugue Icon Set.<br><a href=\"http://p.yusukekamiyamane.com/\">http://p.yusukekamiyamane.com/</a><br></td></tr>"
            "<tr><td align=center><b>+1?</b></td><td>If you like using WPN-XM, consider supporting it:<br><a href=\"http://wpn-xm.org/#donate\">http://wpn-xm.org/#donate</a><br></td></tr>"
            "</td></tr></table>"
            "<br><br>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.<br>"
    ));
    about.setParent(this);
    about.setAutoFillBackground(true);
    about.exec();
}

void MainWindow::autostartDaemons()
{
    qDebug() << "[Daemons] Autostart...";
    if(settings->get("autostart/nginx").toBool()) servers->startNginx();
    if(settings->get("autostart/php").toBool()) servers->startPHP();
    if(settings->get("autostart/mariadb").toBool()) servers->startMariaDb();
    if(settings->get("autostart/mongodb").toBool()) servers->startMongoDb();
    if(settings->get("autostart/memcached").toBool()) servers->startMemcached();
    if(settings->get("autostart/postgresql").toBool()) servers->startPostgreSQL();
}

void MainWindow::checkPorts()
{
    // check for ports, which are already in use
    // based on "netstat -abno | grep "80\|8080\|443""
    // port and service name identification

    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("cmd", QStringList() << "/c netstat -abno");
    process.waitForFinished();

    QByteArray servicesByteArray = process.readAll();
    QStringList strLines = QString(servicesByteArray).split("\n", QString::SkipEmptyParts);

    qDebug() << "Port check netstat -abno needs higher privileges" << strLines;
}

void MainWindow::checkAlreadyActiveDaemons()
{
    qDebug() << "[Processes Running] Check for already running processes.";

    //checkPorts();

    // Check active processes and report, if processes are already running.
    // We do this to avoid collisions.
    // A modal dialog with checkboxes for running processes is shown.
    // The user might then select the processes to shutdown or continue.

    // fetch processes via tasklist stdout
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
    process.start("cmd", QStringList() << "/c tasklist.exe");
    process.waitForFinished();

    // processList contains the tasklist output
    QByteArray processList = process.readAll();
    //qDebug() << "Read" << processList.length() << "bytes";
    //qDebug() << processList;

    // define processes to look for
    QStringList processesToSearch;
    processesToSearch << "nginx"
                      << "apache"
                      << "memcached"
                      << "mysqld"
                      << "php-cgi"
                      << "mongod"
                      << "postgres";

    // init a list for found processes
    QStringList processesFoundList;

    // foreach processesToSearch take a look in the processList
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
                                       "To proceed without shutting processes down, click Continue.<br>"));

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
        dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);

        // Set signal and slot for "Buttons"
        connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
        connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

        // show modal window
        int dialogCode = dlg->exec();

        // fire modal window event loop and wait for button clicks
        // if shutdown was clicked (accept), execute shutdowns
        // if continue was clicked (reject), do nothing and proceed to mainwindow
        if(dialogCode == QDialog::Accepted)
        {
            // fetch all checkboxes
            QList<QCheckBox *> allCheckBoxes = dlg->findChildren<QCheckBox *>();

            // iterate checkbox values
            c = allCheckBoxes.size();
            for(int i = 0; i < c; ++i) {
               QCheckBox *cb = allCheckBoxes.at(i);
               if(cb->isChecked())
               {
                   qDebug() << "Process Shutdown:" << cb->text();

                   // handle the PostgreSQL PID file deletion, too
                   if(cb->text() == "postgres") {
                       QString file = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
                       if(QFile().exists(file)) {
                           QFile().remove(file);
                       }
                   }

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

        // if continue was clicked (reject), do update status indicators in mainwindow and tray
        if(dialogCode == QDialog::Rejected)
        {
            int c = processesFoundList.size();
            for(int i = 0; i < c; ++i) {
                QString procname = processesFoundList.at(i);

                QString servername = this->servers->getCamelCasedServerName(procname).toLocal8Bit().constData();

                Server *server = this->servers->getServer(servername.toLocal8Bit().constData());

                qDebug() << "[Processes Running] The process" << procname << " has the Server" << server->name;

                if(server->name != "Not Installed") {
                    // set indicator in main window
                    setLabelStatusActive(servername, true);

                    // set indicator in tray menu
                    server->trayMenu->setIcon(QIcon(":/status_run"));
                }
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

void MainWindow::setDefaultSettings()
{
    // if the INI is not existing yet, set defaults, they will be written to file
    // if the INI exists, do not set the defaults but read them from file
    if(false == QFile(settings->file()).exists()) {

        // The MainWindow position. Default value is screen center.
        //QPoint center = QApplication::desktop()->screenGeometry().center();
        //settings->set("mainwindow/position", center);

        //QMap<QString, QVariant> languages;
        //languages[str::sLanguageEnglishTitle] = str::sLanguageEnglishKey;
        //languages[str::sLanguageRussianTitle] = str::sLanguageRussianKey;
        //m_defaultManager.addProperty(str::sDefLanguages, languages, languages);

        settings->set("global/runonstartup",             0);
        settings->set("global/autostartdaemons",         0);
        settings->set("global/startminimized",           0);
        settings->set("global/stopdaemonsonquit",        1);
        settings->set("global/clearlogsonstart",         0);
        settings->set("global/donotaskagainclosetotray", 0);
        settings->set("global/editor",                   "notepad.exe");
        //settings->set("global/showballooninfos",         0);

        settings->set("paths/logs",             "./logs");
        settings->set("paths/php",              "./bin/php");
        settings->set("paths/nginx",            "./bin/nginx");
        settings->set("paths/mariadb",          "./bin/mariadb/bin");
        settings->set("paths/mongodb",          "./bin/mongodb/bin");
        settings->set("paths/memcached",        "./bin/memcached");
        settings->set("paths/postgresql",       "./bin/pgsql/bin");

        settings->set("autostart/nginx",        1);
        settings->set("autostart/php",          1);
        settings->set("autostart/mariadb",      1);
        settings->set("autostart/mongodb",      0);
        settings->set("autostart/memcached",    0);
        settings->set("autostart/postgresql",   0);

        settings->set("php/config",             "./bin/php/php.ini");
        settings->set("php/fastcgi-host",       "localhost");
        settings->set("php/fastcgi-port",       9100);

        settings->set("nginx/config",           "./bin/nginx/conf/nginx.conf");
        settings->set("nginx/sites",            "./www");
        settings->set("nginx/port",             80);

        settings->set("mariadb/config",         "./bin/mariadb/my.ini");
        settings->set("mariadb/port",           3306);
        settings->set("mariadb/password",       "");

        settings->set("memcached/port",         11211);

        settings->set("mongodb/config",         "./bin/mongodb/mongodb.conf");
        settings->set("mongodb/port",           27015);

        settings->set("postgresql/config",      "./bin/pgsql/data/postgresql.conf");
        settings->set("postgresql/port",        5432);

        //settings->set("updater/mode",         "manual");
        //settings->set("updater/interval",     "1w");

        qDebug() << "[Settings] Loaded Defaults...\n";
    }
}

void MainWindow::renderInstalledDaemons()
{
    QFont font1;
    font1.setBold(true);
    font1.setWeight(75);
    font1.setPixelSize(11);

    QFont fontNotBold = font1;
    fontNotBold.setBold(false);

    QGroupBox *DaemonStatusGroupBox = new QGroupBox(ui->centralWidget);
    DaemonStatusGroupBox->setObjectName(QStringLiteral("DaemonStatusGroupBox"));
    DaemonStatusGroupBox->setEnabled(true);
    DaemonStatusGroupBox->setGeometry(QRect(10, 70, 471, 121));
    DaemonStatusGroupBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    DaemonStatusGroupBox->setMinimumSize(QSize(0, 121));
    DaemonStatusGroupBox->setBaseSize(QSize(471, 100));
    DaemonStatusGroupBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    DaemonStatusGroupBox->setFlat(false);

    QGridLayout *DaemonsGridLayout = new QGridLayout(DaemonStatusGroupBox);
    DaemonsGridLayout->setSpacing(10);
    DaemonsGridLayout->setObjectName(QStringLiteral("DaemonsGridLayout"));
    DaemonsGridLayout->setSizeConstraint(QLayout::SetMinimumSize);

    QLabel* label_Status = new QLabel();
    label_Status->setText(QApplication::translate("MainWindow", "Status", 0));
    label_Status->setAlignment(Qt::AlignCenter);
    label_Status->setFont(font1);
    label_Status->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Status, 1, 0);

    QLabel* label_Port = new QLabel();
    label_Port->setText(QApplication::translate("MainWindow", "Port", 0));
    label_Port->setAlignment(Qt::AlignCenter);
    label_Port->setFont(font1);
    label_Port->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Port, 1, 1);

    QLabel* label_Daemon = new QLabel();
    label_Daemon->setText(QApplication::translate("MainWindow", "Daemon", 0));
    label_Daemon->setAlignment(Qt::AlignCenter);
    label_Daemon->setFont(font1);
    label_Daemon->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Daemon, 1, 2);

    QLabel* label_Version = new QLabel();
    label_Version->setText(QApplication::translate("MainWindow", "Version", 0));
    label_Version->setAlignment(Qt::AlignCenter);
    label_Version->setFont(font1);
    label_Version->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Version, 1, 3);

    QLabel* label_Config = new QLabel();
    label_Config->setText(QApplication::translate("MainWindow", "Config", 0));
    label_Config->setAlignment(Qt::AlignCenter);
    label_Config->setFont(font1);
    label_Config->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Config, 1, 4, 1, 2); // two columns (gear and gear-pencil)

    QLabel* label_Logs = new QLabel();
    label_Logs->setText(QApplication::translate("MainWindow", "Logs", 0));
    label_Logs->setAlignment(Qt::AlignCenter);
    label_Logs->setFont(font1);
    label_Logs->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Logs, 1, 6, 1, 2); // two columns (log and log-warning)

    QLabel* label_Actions = new QLabel();
    label_Actions->setText(QApplication::translate("MainWindow", "Actions", 0));
    label_Actions->setAlignment(Qt::AlignCenter);
    label_Actions->setFont(font1);
    label_Actions->setEnabled(false);
    DaemonsGridLayout->addWidget(label_Actions, 1, 8, 1, 2); // two columns

    QIcon iconConfig;
    iconConfig.addFile(QStringLiteral(":/gear.png"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon iconConfigEdit;
    iconConfigEdit.addFile(QStringLiteral(":/gear--pencil.png"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon iconLog;
    iconLog.addFile(QStringLiteral(":/report.png"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon iconErrorLog;
    iconErrorLog.addFile(QStringLiteral(":/report--exclamation.png"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon iconStop;
    iconStop.addFile(QStringLiteral(":/action_stop"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon iconStart;
    iconStart.addFile(QStringLiteral(":/action_run"), QSize(), QIcon::Normal, QIcon::Off);

    int rowCounter = 2;

    foreach(Server *server, servers->servers()) {

        // The DaemonsGrid has the following columns:
        //
        // Status | Port | Daemon | Version | Config | Logs (2) | Actions (2)

        // Status
        QLabel* labelStatus = new QLabel();
        labelStatus->setObjectName(QString("label_" + server->name + "_Status"));
        labelStatus->setPixmap(QPixmap(QString::fromUtf8(":/status_run_big")));
        labelStatus->setAlignment(Qt::AlignCenter);
        labelStatus->setEnabled(false); // inital state of status leds is disabled
        DaemonsGridLayout->addWidget(labelStatus, rowCounter, 0);

        // Port
        QLabel* labelPort = new QLabel();
        labelPort->setObjectName(QString("label_"+server->name+"_Port"));
        labelPort->setText( settings->get(server->lowercaseName+"/port").toString() );
        labelPort->setAlignment(Qt::AlignCenter);
        labelPort->setFont(fontNotBold);
        DaemonsGridLayout->addWidget(labelPort, rowCounter, 1);

        // Daemon
        QLabel* labelDaemon = new QLabel();
        labelDaemon->setObjectName(QString("label_" + server->name + "_Name"));
        labelDaemon->setAlignment(Qt::AlignCenter);
        labelDaemon->setText(QApplication::translate("MainWindow",
            "<span style=\" font-family:'MS Shell Dlg 2'; font-size: 14px; font-weight: bold;\">" + server->name.toLocal8Bit() + "</span><", 0));
        DaemonsGridLayout->addWidget(labelDaemon, rowCounter, 2);

        // Version
        QLabel* labelVersion = new QLabel();
        labelVersion->setObjectName(QString("label_" + server->name + "_Version"));
        labelVersion->setAlignment(Qt::AlignCenter);
        labelVersion->setText(getVersion(server->lowercaseName));
        labelVersion->setFont(fontNotBold);
        DaemonsGridLayout->addWidget(labelVersion, rowCounter, 3);

        // Config

        if(server->name != "Memcached") {

            // Configuration via Webinterface
            QPushButton* pushButton_Configure = new QPushButton();
            pushButton_Configure->setObjectName(QString("pushButton_Configure_"+ server->name +""));
            pushButton_Configure->setIcon(iconConfig);
            pushButton_Configure->setFlat(true);
            pushButton_Configure->setToolTip(QApplication::translate(
                "MainWindow", "Open the Webinterface Configuration Tab for "+ server->name.toLocal8Bit() +" ", 0)
            );
            DaemonsGridLayout->addWidget(pushButton_Configure, rowCounter, 4);

            // Configuration via Editor
            QPushButton* pushButton_ConfigureEdit = new QPushButton();
            pushButton_ConfigureEdit->setObjectName(QString("pushButton_ConfigurationEditor_"+ server->name +""));
            pushButton_ConfigureEdit->setIcon(iconConfigEdit);
            pushButton_ConfigureEdit->setFlat(true);
            pushButton_ConfigureEdit->setToolTip(QApplication::translate(
                "MainWindow", "Open "+ server->name.toLocal8Bit() +" Configuration File with Editor", 0)
            );
            DaemonsGridLayout->addWidget(pushButton_ConfigureEdit, rowCounter, 5);

            connect(pushButton_ConfigureEdit, SIGNAL(clicked()), this, SLOT(openConfigurationInEditor()));
        }

        // Logs
        foreach (QString logfile, server->logFiles)
        {
            if(!logfile.isEmpty()) {
                // normal log
                if(!logfile.contains("error"))
                {
                    QPushButton* pushButton_ShowLog = new QPushButton();
                    pushButton_ShowLog->setObjectName(QString("pushButton_ShowLog_" + server->name + ""));
                    pushButton_ShowLog->setIcon(iconLog);
                    pushButton_ShowLog->setFlat(true);
                    pushButton_ShowLog->setToolTip(QApplication::translate(
                        "MainWindow", "Open "+ server->name.toLocal8Bit() +" Log", 0)
                    );
                    DaemonsGridLayout->addWidget(pushButton_ShowLog, rowCounter, 6);

                    connect(pushButton_ShowLog, SIGNAL(clicked()), this, SLOT(openLog()));
                }

                // error log
                if(logfile.contains("error"))
                {
                    QPushButton* pushButton_ShowErrorLog = new QPushButton();
                    pushButton_ShowErrorLog->setObjectName(QString("pushButton_ShowErrorLog_"+ server->name +""));
                    pushButton_ShowErrorLog->setIcon(iconErrorLog);
                    pushButton_ShowErrorLog->setFlat(true);
                    pushButton_ShowErrorLog->setToolTip(QApplication::translate(
                        "MainWindow", "Open "+ server->name.toLocal8Bit() +" Error Log", 0)
                    );
                    DaemonsGridLayout->addWidget(pushButton_ShowErrorLog, rowCounter, 7);

                    connect(pushButton_ShowErrorLog, SIGNAL(clicked()), this, SLOT(openLog()));
                }
            }
        }

        // Actions
        QPushButton* pushButton_Stop = new QPushButton();
        pushButton_Stop->setObjectName(QString("pushButton_Stop_"+ server->name +""));

        pushButton_Stop->setIcon(iconStop);
        pushButton_Stop->setFlat(true);
        pushButton_Stop->setToolTip(QApplication::translate(
            "MainWindow", "Stop "+ server->name.toLocal8Bit() +"", 0)
        );
        DaemonsGridLayout->addWidget(pushButton_Stop, rowCounter, 8);

        QPushButton* pushButton_Start = new QPushButton();
        pushButton_Start->setObjectName(QString("pushButton_Start_"+ server->name +""));
        pushButton_Start->setIcon(iconStart);
        pushButton_Start->setFlat(true);
        pushButton_Start->setToolTip(QApplication::translate(
            "MainWindow", "Start "+ server->name.toLocal8Bit() +"", 0)
        );
        DaemonsGridLayout->addWidget(pushButton_Start, rowCounter, 9);

        rowCounter++;
    }

    /**
     * DaemonsGridLayout Size depends on installed Components.
     * So the BottomWidget has to move down (y + height of DaemonsGridLayout + margin)
     * The RightSideWidget moves up, if there are only 3-4 elements,
     * the "Webinterface" PushButton will be on par with the Labels.
     * If there are more then 4 elements, the "webinterface" PushButton
     * is on par with the first daemon.
     */

    QRect DaemonsBox = DaemonStatusGroupBox->frameGeometry();
    QSize DaemonsSize = DaemonStatusGroupBox->sizeHint();

    // (top left corner y) + (dynamic height y, based on rows for daemons)
    int bottomWidgetY = DaemonsBox.y() + DaemonsSize.height();

    QRect BottomWidgetGeo = ui->BottomWidget->geometry();

    if(BottomWidgetGeo.y() > bottomWidgetY) {
        // 3 or 4 elements - move things from the bottom up
        ui->BottomWidget->move(QPoint(BottomWidgetGeo.x(), bottomWidgetY + 10));
        ui->ToolsGroupBox->move(QPoint(ui->ToolsGroupBox->x(), ui->ToolsGroupBox->y()));
        QRect RightWidgetGeo = ui->RightSideWidget->geometry();
        ui->RightSideWidget->move(QPoint(RightWidgetGeo.x(), RightWidgetGeo.y() - 20));
        this->resize(QSize(this->geometry().width(), bottomWidgetY + BottomWidgetGeo.height() + 20));
    } else {
        // more then 4 elements - auto-expand
        ui->BottomWidget->move(QPoint(BottomWidgetGeo.x(), bottomWidgetY - 10));
        this->resize(QSize(this->geometry().width(), bottomWidgetY + BottomWidgetGeo.height()));
    }
}

QString MainWindow::getVersion(QString server)
{
    QString s = server.toLower();
    if(s == "nginx")     { return getNginxVersion(); }
    if(s == "memcached") { return getMemcachedVersion(); }
    if(s == "mongodb")   { return getMongoVersion(); }
    if(s == "mariadb")   { return getMariaVersion(); }
    if(s == "php")       { return getPHPVersion(); }
    if(s == "postgresql"){ return getPostgresqlVersion(); }

    return "The function for fetching the version for " + s + "is not implemented, yet.";
}

void MainWindow::updateVersion(QString server) {
    QString version = getVersion(server);
    QLabel* label = qApp->activeWindow()->findChild<QLabel *>("label_" + server + "_Version");
    if(label != 0) {
         label->setText(version);
    }
}

QJsonDocument MainWindow::loadJson(QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::ReadOnly);
    return QJsonDocument().fromJson(jsonFile.readAll());
}

void MainWindow::saveJson(QJsonDocument document, QString fileName) {
    QFile jsonFile(fileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(document.toJson());
}
