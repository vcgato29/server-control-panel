#ifndef HOSTMANAGERDIALOG_H
#define HOSTMANAGERDIALOG_H

#include "hosttablemodel.h"
#include "adddialog.h"
#include "host.h"

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
#include <QDialog>

namespace HostsFileManager
{
    class HostsManagerDialog : public QDialog
    {
        Q_OBJECT

        public:
            explicit HostsManagerDialog(QWidget *parent = 0);
            ~HostsManagerDialog();
        signals:
            void selectionChanged (const QItemSelection &selected);
        public slots:
            void addEntry();
            void editEntry();
            void removeEntry();
            void accept();
        private:
            QTableView* tableView;
    };

}

#endif // HOSTMANAGERDIALOG_H
