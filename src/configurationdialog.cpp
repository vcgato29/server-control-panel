// local includes
#include "configurationdialog.h"
#include "ui_configurationdialog.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    ui->setupUi(this);
}

ConfigurationDialog::~ConfigurationDialog()
{
    delete ui;
}

bool ConfigurationDialog::runOnStartUp() const
{
    return (m_chkRunStartUp->checkState() == Qt::Checked);
}

void ConfigurationDialog::setRunOnStartUp(bool run)
{
    m_chkRunStartUp->setChecked(run);
}
