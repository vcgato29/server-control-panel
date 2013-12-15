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
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
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


