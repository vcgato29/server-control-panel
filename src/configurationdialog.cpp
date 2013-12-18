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

#include "windowsapi.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    ui->setupUi(this);


    connect(ui->checkbox_autostartDaemons, SIGNAL(clicked(bool)),
            this, SLOT(toggleAutostartDaemonCheckboxes(bool)));

    this->settings = new Settings;
    readSettings();

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickedButtonBoxOk()));
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

void ConfigurationDialog::readSettings()
{
   // Read Settings from INI and prefill config dialog items

   ui->checkbox_runOnStartUp->setChecked(settings->get("global/runonstartup", false).toBool());
   ui->checkbox_autostartDaemons->setChecked(settings->get("global/autostartdaemons", false).toBool());

   ui->checkbox_autostartPHP->setChecked(settings->get("autostart/php", true).toBool());
   ui->checkbox_autostartNginx->setChecked(settings->get("autostart/nginx", true).toBool());
   ui->checkbox_autostartMariaDb->setChecked(settings->get("autostart/mariadb", true).toBool());
   ui->checkbox_autostartMongoDb->setChecked(settings->get("autostart/mongodb", false).toBool());
   ui->checkbox_autostartMemcached->setChecked(settings->get("autostart/memcached", false).toBool());

   ui->checkbox_clearLogsOnStart->setChecked(settings->get("global/clearlogsonstart", false).toBool());

   ui->checkbox_stopDaemonsOnQuit->setChecked(settings->get("global/stopdaemonsonquit", false).toBool());
}

void ConfigurationDialog::writeSettings()
{
    // we use a boolean to int type conversion, to convert the isChecked()
    // boolean return value to integer (0/1). i like that more then having true/false in INI.

    settings->set("global/runonstartup",      int(ui->checkbox_runOnStartUp->isChecked()));
    settings->set("global/autostartdaemons",  int(ui->checkbox_autostartDaemons->isChecked()));

    settings->set("autostart/nginx",          int(ui->checkbox_autostartNginx->isChecked()));
    settings->set("autostart/php",            int(ui->checkbox_autostartPHP->isChecked()));
    settings->set("autostart/mariadb",        int(ui->checkbox_autostartMariaDb->isChecked()));
    settings->set("autostart/mongodb",        int(ui->checkbox_autostartMongoDb->isChecked()));
    settings->set("autostart/memcached",      int(ui->checkbox_autostartMemcached->isChecked()));

    settings->set("global/clearlogsonstart",  int(ui->checkbox_clearLogsOnStart->isChecked()));
    settings->set("global/stopdaemonsonquit", int(ui->checkbox_stopDaemonsOnQuit->isChecked()));
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

    for (int i = 0; i < ui->autostartDaemonsFormLayout->count(); ++i)
    {
      ui->autostartDaemonsFormLayout->itemAt(i)->widget()->setEnabled(run);
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

void ConfigurationDialog::toggleRunOnStartup()
{
    QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

     if(ui->checkbox_runOnStartUp->isChecked() == true) {
        // Add WPN-XM SCP shortcut to the Windows Autostart folder.
        // In Windows terminology shortcuts are "shell links".
        //using WindowsAPI::CreateShellLink;
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
