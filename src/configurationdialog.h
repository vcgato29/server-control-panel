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

#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

// global includes
#include <QDialog>
#include "settings.h"

// forward declarations
QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

namespace Ui {
class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ConfigurationDialog(QWidget* parent = 0);
  ~ConfigurationDialog();

  void setRunOnStartUp(bool run = true);
  bool runOnStartUp() const;

  void setAutostartDaemons(bool run = true);
  bool runAutostartDaemons() const;

  void setClearLogsOnStart(bool run = true);
  bool runClearLogsOnStart() const;

  void setStopDaemonsOnQuit(bool run = true);
  bool stopDaemonsOnQuit() const;

  void fileOpen();

 private slots:
  void toggleAutostartDaemonCheckboxes(bool run = true);
  void onClickedButtonBoxOk();

  void on_toolButton_SelectEditor_clicked();
  void on_toolButton_ResetEditor_clicked();

private:
  Ui::ConfigurationDialog* ui;

  Settings* settings;

  QCheckBox* checkbox_runOnStartUp;
  QCheckBox* checkbox_autostartDaemons;
  QCheckBox* checkbox_clearLogsOnStart;
  QCheckBox* checkbox_stopDaemonsOnQuit;

  QCheckBox* checkbox_autostartMariaDb;
  QCheckBox* checkbox_autostartMongoDb;
  QCheckBox* checkbox_autostartPHP;
  QCheckBox* checkbox_autostartNginx;
  QCheckBox* checkbox_autostartMemcached;
  QCheckBox* checkbox_autostartPostgreSQL;

  void readSettings();
  void writeSettings();
  void toggleRunOnStartup();
};

#endif  // CONFIGURATIONDIALOG_H
