// local includes
#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "QDebug"

#include <QStandardPaths>
#include <QFile>

// this is needed for "createShellLink"
#include <windows.h>
#include <objbase.h>
#include <objidl.h>
#include <shlobj.h> // type defintion for IShellLink
#include <shlwapi.h>

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

/*
 * http://msdn.microsoft.com/en-us/library/windows/desktop/bb776891%28v=vs.85%29.aspx
 */
IShellLink* CreateShellLink(QString target_app_path, QString app_args, QString description,
                            QString icon_path, int icon_index, QString working_dir,
                            QString linkShortcut)
{
    IShellLink* shell_link = NULL;

    HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
                                  reinterpret_cast<void**> (&(shell_link)));

    if(SUCCEEDED(hres)) {

        IPersistFile* persistFile = NULL;

        shell_link->SetPath(target_app_path.toStdWString().c_str());
        shell_link->SetArguments(app_args.toStdWString().c_str());
        shell_link->SetIconLocation(icon_path.toStdWString().c_str(), icon_index);
        shell_link->SetDescription(description.toStdWString().c_str());
        shell_link->SetWorkingDirectory(working_dir.toStdWString().c_str());

        // Query IShellLink for the IPersistFile interface,
        // used for saving the shortcut in persistent storage.
        hres = shell_link->QueryInterface(IID_IPersistFile, reinterpret_cast<void**> (&(persistFile)));

        if (SUCCEEDED(hres)) {

            // Save the link by calling IPersistFile::Save.
            hres = persistFile->Save((LPCOLESTR)linkShortcut.toStdWString().c_str(), STGM_WRITE);

            // Release the pointer to the IPersistFile interface.
            persistFile->Release();
        }

        // Release the pointer to the IShellLink interface.
        shell_link->Release();
    }

    return shell_link;
}

void ConfigurationDialog::toggleRunOnStartup()
{
    QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

     if(ui->checkbox_runOnStartUp->isChecked() == true) {

       /*
         Add WPN-XM SCP shortcut to the Windows Autostart folder.
         In Windows terminology shortcuts are "shell links".
        */

        CreateShellLink(
            qApp->applicationFilePath(),    // target app
            "",                             // arguments
             "WPN-XM Server Control Panel", // desc
            qApp->applicationFilePath(),    // icon_path
            0,                              // icon idx
            qApp->applicationDirPath(),     // working dir
            startupDir + "\\WPN-XM Server Control Panel.lnk" // filepath of shortcut
        );

     } else {
        // remove link
        QFile::remove(startupDir+"\\WPN-XM Server Control Panel.lnk");
     }

}
