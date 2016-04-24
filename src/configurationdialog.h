#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <QDialog>
#include <QStandardPaths>
#include <QFile>
#include <QFileDialog>

#include "settings.h"
#include "windowsapi.h"
#include "servers.h"

namespace Configuration
{
    namespace Ui {
        class ConfigurationDialog;
    }

    class ConfigurationDialog : public QDialog
    {
        Q_OBJECT

        public:
            explicit ConfigurationDialog(QWidget *parent = 0);

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

            void setServers(Servers::Servers *servers);
            void hideAutostartCheckboxesOfNotInstalledServers();

        private slots:
            void toggleAutostartDaemonCheckboxes(bool run = true);
            void onClickedButtonBoxOk();

            void on_toolButton_SelectEditor_clicked();
            void on_toolButton_ResetEditor_clicked();

        private:
            Ui::ConfigurationDialog *ui;

            Settings::SettingsManager *settings;
            Servers::Servers          *servers;

            QCheckBox *checkbox_runOnStartUp;
            QCheckBox *checkbox_autostartDaemons;
            QCheckBox *checkbox_clearLogsOnStart;
            QCheckBox *checkbox_stopDaemonsOnQuit;

            QCheckBox *checkbox_autostart_MariaDb;
            QCheckBox *checkbox_autostart_MongoDb;
            QCheckBox *checkbox_autostart_PHP;
            QCheckBox *checkbox_autostart_Nginx;
            QCheckBox *checkbox_autostart_Memcached;
            QCheckBox *checkbox_autostart_PostgreSQL;
            QCheckBox *checkbox_autostart_Redis;

            void readSettings();
            void writeSettings();
            void toggleRunOnStartup();
    };

}

#endif  // CONFIGURATIONDIALOG_H
