#ifndef NGINXADDPOOLDIALOG_H
#define NGINXADDPOOLDIALOG_H

#include <QDialog>

namespace Configuration
{
    namespace Ui {
        class NginxAddPoolDialog;
    }

    class NginxAddPoolDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NginxAddPoolDialog(QWidget *parent = 0);
        ~NginxAddPoolDialog();

        QString pool();
        QString method();

        enum Column {
            Pool, Method
        };

    private:
        Configuration::Ui::NginxAddPoolDialog *ui;
    };

}
#endif // NGINXADDPOOLDIALOG_H
