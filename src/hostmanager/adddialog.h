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

#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
class QLineEdit;
//class QLabel;
//class QPushButton;

class HostsAddDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HostsAddDialog(QWidget *parent = 0);
    QString name();
    QString address();

    void edit(QString name, QString adress);

signals:

public slots:

private:
    QLineEdit *m_lineedit_Name;
    QLineEdit *m_lineedit_Address;
};

#endif // ADDDIALOG_H
