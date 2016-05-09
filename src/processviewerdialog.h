#ifndef ProcessViewerDialog_H
#define ProcessViewerDialog_H

#include "src/csv.h"

#include <QDialog>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QDebug>

namespace Ui {
    class ProcessViewerDialog;
}

class ProcessViewerDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ProcessViewerDialog(QWidget *parent = 0);
        ~ProcessViewerDialog();

        QTreeWidgetItem* addRoot(QString name, QString description);
        void addChild(QTreeWidgetItem *parent, QString name, QString description);

        QList<QStringList> execute(const QString &cmd);

    private:
        Ui::ProcessViewerDialog *ui;
};

#endif // ProcessViewerDialog_H
