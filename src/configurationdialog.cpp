// local includes
#include "configurationdialog.h"
#include "ui_configurationdialog.h"

ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigurationDialog)
{
    ui->setupUi(this);

    // create a 2*2 table
    QStandardItemModel* table_model = new QStandardItemModel(2, 2);
    for (int row = 0; row < 2; ++row) {
        for (int column = 0; column < 2; ++column) {
            QStandardItem *item = new QStandardItem((QString())); // you should set your data here (in this case as a string)
            table_model.setItem(row, column, item);
        }
    }

    // bind MODEL (table_model) to VIEW (tableview)
    QTableView table;
    table.setModel(table_model);
    table.show();

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

