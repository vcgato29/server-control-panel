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

// local includes
#include "configurationdialog.h"
#include "ui_configurationdialog.h"

#include <QStandardPaths>
#include <QFile>
#include <QFileDialog>

#include "windowsapi.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    ui->setupUi(this);

    this->settings = new Settings;
    readSettings();

    hideAutostartCheckboxesOfNotInstalledServers();

    toggleAutostartDaemonCheckboxes(ui->checkbox_autostartDaemons->isChecked());

    //populateOpenWithDropdown();

    connect(ui->checkbox_autostartDaemons, SIGNAL(clicked(bool)),
            this, SLOT(toggleAutostartDaemonCheckboxes(bool)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickedButtonBoxOk()));
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

void ConfigurationDialog::setServers(Servers *servers)
{
    this->servers = servers;
}

void ConfigurationDialog::readSettings()
{
   // Read Settings from INI and prefill config dialog items

   ui->checkbox_runOnStartUp->setChecked(settings->get("global/runonstartup", false).toBool());
   ui->checkbox_autostartDaemons->setChecked(settings->get("global/autostartdaemons", false).toBool());
   ui->checkbox_startMinimized->setChecked(settings->get("global/startminimized", false).toBool());

   ui->checkbox_autostart_PHP->setChecked(settings->get("autostart/php", true).toBool());
   ui->checkbox_autostart_Nginx->setChecked(settings->get("autostart/nginx", true).toBool());
   ui->checkbox_autostart_MariaDb->setChecked(settings->get("autostart/mariadb", true).toBool());
   ui->checkbox_autostart_MongoDb->setChecked(settings->get("autostart/mongodb", false).toBool());
   ui->checkbox_autostart_Memcached->setChecked(settings->get("autostart/memcached", false).toBool());
   ui->checkbox_autostart_Postgresql->setChecked(settings->get("autostart/postgresql", false).toBool());

   ui->checkbox_clearLogsOnStart->setChecked(settings->get("global/clearlogsonstart", false).toBool());
   ui->checkbox_stopDaemonsOnQuit->setChecked(settings->get("global/stopdaemonsonquit", false).toBool());

   ui->checkbox_onStartAllMinimize->setChecked(settings->get("global/onstartallminimize", false).toBool());
   ui->checkbox_onStartAllOpenWebinterface->setChecked(settings->get("global/onstartallopenwebinterface", false).toBool());

   ui->lineEdit_SelectedEditor->setText(settings->get("global/editor", QVariant(QString("notepad.exe")) ).toString());
}

void ConfigurationDialog::writeSettings()
{
    // we use a boolean to int type conversion, to convert the isChecked()
    // boolean return value to integer (0/1). i like that more then having true/false in INI.

    settings->set("global/runonstartup",      int(ui->checkbox_runOnStartUp->isChecked()));
    settings->set("global/startminimized",    int(ui->checkbox_startMinimized->isChecked()));
    settings->set("global/autostartdaemons",  int(ui->checkbox_autostartDaemons->isChecked()));

    settings->set("autostart/nginx",          int(ui->checkbox_autostart_Nginx->isChecked()));
    settings->set("autostart/php",            int(ui->checkbox_autostart_PHP->isChecked()));
    settings->set("autostart/mariadb",        int(ui->checkbox_autostart_MariaDb->isChecked()));
    settings->set("autostart/mongodb",        int(ui->checkbox_autostart_MongoDb->isChecked()));
    settings->set("autostart/memcached",      int(ui->checkbox_autostart_Memcached->isChecked()));
    settings->set("autostart/postgresql",     int(ui->checkbox_autostart_Postgresql->isChecked()));

    settings->set("global/clearlogsonstart",  int(ui->checkbox_clearLogsOnStart->isChecked()));
    settings->set("global/stopdaemonsonquit", int(ui->checkbox_stopDaemonsOnQuit->isChecked()));

    settings->set("global/onstartallminimize",          int(ui->checkbox_onStartAllMinimize->isChecked()));
    settings->set("global/onstartallopenwebinterface",  int(ui->checkbox_onStartAllOpenWebinterface->isChecked()));

    settings->set("global/editor",            QString(ui->lineEdit_SelectedEditor->text()));
}

void ConfigurationDialog::onClickedButtonBoxOk()
{
    writeSettings();
    toggleRunOnStartup();
}

bool ConfigurationDialog::runOnStartUp() const
{
    return (ui->checkbox_runOnStartUp->checkState() == Qt::Checked);
}

void ConfigurationDialog::setRunOnStartUp(bool run)
{
    ui->checkbox_runOnStartUp->setChecked(run);
}

bool ConfigurationDialog::runAutostartDaemons() const
{
    return (ui->checkbox_autostartDaemons->checkState() == Qt::Checked);
}

void ConfigurationDialog::setAutostartDaemons(bool run)
{
    ui->checkbox_autostartDaemons->setChecked(run);
}

void ConfigurationDialog::toggleAutostartDaemonCheckboxes(bool run)
{
    // Note: layout doesn't "inject" itself in the parent-child tree, so findChildren() doesn't work

    // left box
    for (int i = 0; i < ui->autostartDaemonsFormLayout->count(); ++i)
    {
      ui->autostartDaemonsFormLayout->itemAt(i)->widget()->setEnabled(run);
    }

    // right box
    for (int i = 0; i < ui->autostartDaemonsFormLayout2->count(); ++i)
    {
      ui->autostartDaemonsFormLayout2->itemAt(i)->widget()->setEnabled(run);
    }
}

void ConfigurationDialog::hideAutostartCheckboxesOfNotInstalledServers()
{
    QStringList installed = this->servers->getListOfServerNamesInstalled();

    QList<QCheckBox *> boxes = ui->tabWidget->findChildren<QCheckBox *>(QRegExp("checkbox_autostart_\\w"));

    for(int i = 0; i < boxes.size(); ++i) {
       QCheckBox *box = boxes.at(i);

       // return last part of "checkbox_autostart_*"
       QString name = box->objectName().section("_", -1).toLower();
       QString labelName = this->servers->getCamelCasedServerName(name) + "Label";
       QLabel *label = ui->tabWidget->findChild<QLabel *>(labelName);

       if(installed.contains(name) == true) {
           qDebug() << "[" + name + "] Autostart Checkbox and Label visible.";
           box->setVisible(true);
           //label->setVisible(true);
       } else {
           qDebug() << "[" + name + "] Autostart Checkbox and Label hidden.";
           box->setVisible(false);
           label->setVisible(false);
       }
    }
}

bool ConfigurationDialog::runClearLogsOnStart() const
{
    return (ui->checkbox_clearLogsOnStart->checkState() == Qt::Checked);
}

void ConfigurationDialog::setClearLogsOnStart(bool run)
{
    ui->checkbox_clearLogsOnStart->setChecked(run);
}

bool ConfigurationDialog::stopDaemonsOnQuit() const
{
    return (ui->checkbox_stopDaemonsOnQuit->checkState() == Qt::Checked);
}

void ConfigurationDialog::setStopDaemonsOnQuit(bool run)
{
    ui->checkbox_stopDaemonsOnQuit->setChecked(run);
}

void ConfigurationDialog::toggleRunOnStartup()
{
    // Windows %APPDATA% = Roaming ... Programs\Startup
    QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

    if(ui->checkbox_runOnStartUp->isChecked() == true) {
        // Add WPN-XM SCP shortcut to the Windows Autostart folder.
        // In Windows terminology "shortcuts" are "shell links".
        WindowsAPI::CreateShellLink(
            qApp->applicationFilePath(),"","WPN-XM Server Control Panel", // app, args, desc
            qApp->applicationFilePath(),0, // icon path and idx
            qApp->applicationDirPath(), // working dir
            startupDir + "\\WPN-XM Server Control Panel.lnk" // filepath of shortcut
        );
    } else {
        // remove link
        QFile::remove(startupDir+"\\WPN-XM Server Control Panel.lnk");
    }
}

void ConfigurationDialog::fileOpen()
{
    QString programFilesPath(getenv("PROGRAMFILES"));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Editor..."),
            programFilesPath, tr("Executables (*.exe);;All Files (*)"));

    fileName = QDir::toNativeSeparators(fileName); // not needed, but better looking

    ui->lineEdit_SelectedEditor->setText(fileName);
}

void ConfigurationDialog::on_toolButton_SelectEditor_clicked()
{
    ConfigurationDialog::fileOpen();
}

void ConfigurationDialog::on_toolButton_ResetEditor_clicked()
{
    ui->lineEdit_SelectedEditor->setText("notepad.exe");
}
