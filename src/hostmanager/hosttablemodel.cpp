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
#include "hosttablemodel.h"
#include "host.h"

// Global includes
#include <QList>

HostsTableModel::HostsTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

int HostsTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_listHosts.size();
}

int HostsTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 2;
}

QVariant HostsTableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_listHosts.size() || index.row() < 0)
        return QVariant();

    Host* host = m_listHosts.at(index.row());

    /*
    if (role == Qt::CheckStateRole){
        switch(index.column()){
        case COLUMN_ADDRESS:
            return host->isEnable()?Qt::Checked:Qt::Unchecked;
        }
    }*/
    if (role == Qt::DisplayRole) {
        switch(index.column()){
        case COLUMN_ADDRESS:
            return host->address();
        case COLUMN_NAME:
            return host->name();
        }
    }
    return QVariant();
}

QVariant HostsTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case COLUMN_ADDRESS:
                return "Address";
            case COLUMN_NAME:
                return "Name";
            default:
                return QVariant();
        }
    }

    return QVariant();
}

Qt::ItemFlags HostsTableModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool HostsTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();

        Host* host = m_listHosts.value(row);

        switch(index.column()){
        case COLUMN_ADDRESS:
            host->setAddress(value.toString());
            break;
        case COLUMN_NAME:
            host->setName(value.toString());
            break;
        default:
            return false;
        }

        emit(dataChanged(index, index));

        return true;
    }

    return false;
}

bool HostsTableModel::insertRows(int position, int rows, const QModelIndex &index) {
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; row++) {
        Host* host = new Host();
        m_listHosts.insert(position, host);
    }

    endInsertRows();
    return true;
}

bool HostsTableModel::removeRows(int position, int rows, const QModelIndex &index) {
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row=0; row < rows; ++row) {
        Host* host = m_listHosts.takeAt(position);
        delete host;
    }

    endRemoveRows();
    return true;
}

void HostsTableModel::setList(QList<Host*> listHosts){
    m_listHosts = listHosts;
    QModelIndex root = index(0,0);
    emit(dataChanged(root, index(rowCount(QModelIndex()), columnCount(QModelIndex()))));
}

QList<Host*> HostsTableModel::getList(){
    return m_listHosts;
}
