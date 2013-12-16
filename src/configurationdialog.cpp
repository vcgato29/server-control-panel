// local includes
#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "QDebug"

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
}

bool ConfigurationDialog::runOnStartUp() const
{
    return (checkbox_runOnStartUp->checkState() == Qt::Checked);
}

void ConfigurationDialog::setRunOnStartUp(bool run)
{
    checkbox_runOnStartUp->setChecked(run);
}

bool ConfigurationDialog::runAutostartDaemons() const
{
    return (checkbox_autostartDaemons->checkState() == Qt::Checked);
}

void ConfigurationDialog::setAutostartDaemons(bool run)
{
    checkbox_autostartDaemons->setChecked(run);
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
    return (checkbox_clearLogsOnStart->checkState() == Qt::Checked);
}

void ConfigurationDialog::setClearLogsOnStart(bool run)
{
    checkbox_clearLogsOnStart->setChecked(run);
}
