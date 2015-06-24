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
#include "hostmanagerdialog.h"
#include "hosttablemodel.h"
#include "adddialog.h"
#include "host.h"

// Global includes
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QToolBar>
#include <QApplication>

HostsManagerDialog::HostsManagerDialog(QWidget *parent) :
    QDialog(parent)
{
    // remove question mark from the title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QToolBar* toolbar = new QToolBar(this);
    toolbar->addAction("Add", this, SLOT(addEntry()));
    toolbar->addAction("Edit", this, SLOT(editEntry()));
    toolbar->addAction("Delete", this, SLOT(removeEntry()));

    QPushButton* btnOk = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_VistaShield), "OK", this);
    QPushButton* btnCancel = new QPushButton("Cancel", this);


    HostsTableModel* tableModel = new HostsTableModel(this);
    tableModel->setList(Host::GetHosts());

    m_tableView = new QTableView(this);
    m_tableView->setModel(tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setMinimumWidth(300);

    QGridLayout *gLayout = new QGridLayout;
    gLayout->addWidget(toolbar, 0, 0);
    gLayout->addWidget(m_tableView, 1, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(btnOk);
    buttonLayout->addWidget(btnCancel);

    gLayout->addLayout(buttonLayout, 2, 0, Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gLayout);
    setLayout(mainLayout);

    connect(btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    //connect(tableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SIGNAL(selectionChanged(QItemSelection)));

    setWindowTitle(tr("WPX-XM Server Control Panel - Host File Manager"));
    setFixedWidth(400);
}

HostsManagerDialog::~HostsManagerDialog(){
    HostsTableModel *model = static_cast<HostsTableModel*>(m_tableView->model());
    qDeleteAll(model->getList());
}

void HostsManagerDialog::addEntry() {
    HostsAddDialog aDialog;

    if (aDialog.exec()) {
        QString name = aDialog.name();
        QString address = aDialog.address();

        HostsTableModel *model = static_cast<HostsTableModel*>(m_tableView->model());
        QList<Host*> list = model->getList();

        //do the add
        Host host(name, address);
        if (!list.contains(&host)) {
            model->insertRows(0, 1, QModelIndex());

            QModelIndex index = model->index(0, HostsTableModel::COLUMN_NAME, QModelIndex());
            model->setData(index, name, Qt::EditRole);
            index = model->index(0, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
            model->setData(index, address, Qt::EditRole);
        } else {
            QMessageBox::information(this, tr("Duplicate Entry"), tr("The host mapping already exists."));
        }
    }
}

void HostsManagerDialog::editEntry()
 {
     HostsTableModel *model = static_cast<HostsTableModel*>(m_tableView->model());
     QItemSelectionModel *selectionModel = m_tableView->selectionModel();

     QModelIndexList indexes = selectionModel->selectedRows();
     QModelIndex index;
     int row = -1;
     QString name;
     QString address;

     if(!indexes.empty()){
         foreach (index, indexes) {
             row = index.row();

             QModelIndex indexName = model->index(row, HostsTableModel::COLUMN_NAME, QModelIndex());
             QVariant varName = model->data(indexName, Qt::DisplayRole);
             name = varName.toString();

             QModelIndex indexAddress = model->index(row, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
             QVariant varAddr = model->data(indexAddress, Qt::DisplayRole);
             address = varAddr.toString();
         }

         HostsAddDialog aDialog;
         aDialog.edit(name, address);

         if (aDialog.exec()) {
              QString newAddress = aDialog.address();
              if (newAddress != address) {
                  QModelIndex i = model->index(row, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
                  model->setData(i, newAddress, Qt::EditRole);
              }
         }
     }
}

void HostsManagerDialog::removeEntry(){
    HostsTableModel *model = static_cast<HostsTableModel*>(m_tableView->model());
    QItemSelectionModel *selectionModel = m_tableView->selectionModel();

    QModelIndexList indexes = selectionModel->selectedRows();
    QModelIndex index;

    foreach (index, indexes) {
        model->removeRows(index.row(), 1, QModelIndex());
    }
}

void HostsManagerDialog::accept(){
    HostsTableModel *model = static_cast<HostsTableModel*>(m_tableView->model());
    Host::SetHosts(model->getList());
    QDialog::accept();
}
