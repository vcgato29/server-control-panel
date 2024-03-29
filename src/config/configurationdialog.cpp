#include "configurationdialog.h"
#include "ui_configurationdialog.h"

#include "nginxaddpooldialog.h"
#include "nginxaddserverdialog.h"
#include "../json.h"

namespace Configuration
{
    ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ConfigurationDialog)
    {
        ui->setupUi(this);

        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        this->settings = new Settings::SettingsManager;
        readSettings();

        // setup autostart section
        hideAutostartCheckboxesOfNotInstalledServers();
        toggleAutostartDaemonCheckboxes(ui->checkbox_autostartDaemons->isChecked());
        connect(ui->checkbox_autostartDaemons, SIGNAL(clicked(bool)),
                this, SLOT(toggleAutostartDaemonCheckboxes(bool)));

        // load initial data for pages
        loadNginxUpstreams();

        connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickedButtonBoxOk()));

        ui->configMenuTreeWidget->expandAll();
    }

    ConfigurationDialog::~ConfigurationDialog()
    {
        delete ui;
    }

    /**
     * Search for items in the "Configuration Menu" TreeWidget
     *
     * @brief ConfigurationDialog::on_configMenuSearchLineEdit_textChanged
     * @param query
     */
    void ConfigurationDialog::on_configMenuSearchLineEdit_textChanged(const QString &query)
    {
        ui->configMenuTreeWidget->expandAll();

        // Iterate over all child items : filter items with "contains" query
        QTreeWidgetItemIterator iterator(ui->configMenuTreeWidget, QTreeWidgetItemIterator::All);
        while(*iterator)
        {
            QTreeWidgetItem *item = *iterator;
            if(item && item->text(0).contains(query, Qt::CaseInsensitive)) {
                item->setHidden(false);
            } else {
                // Problem: the matched child is visibile, but parent is hidden, because no match.
                // so, lets hide only items without childs.
                // any not matching parent will stay visible.. until next iteration, see below.
                if(item->childCount() == 0) {
                    item->setHidden(true);
                }
            }
            ++iterator;
        }

        // Iterate over items with childs : hide, if they do not have a matching (visible) child (see above).
        QTreeWidgetItemIterator parentIterator(ui->configMenuTreeWidget, QTreeWidgetItemIterator::HasChildren);
        while(*parentIterator)
        {
            QTreeWidgetItem *item = *parentIterator;
            // count the number of hidden childs
            int childs = item->childCount();
            int hiddenChilds = 0;
            for (int i = 0; i < childs; ++i) {
                if(item->child(i)->isHidden()) {
                    ++hiddenChilds;
                }
            }
            // finally: if all childs are hidden, hide the parent (*item), too
            if(hiddenChilds == childs) {
                item->setHidden(true);
            }
            ++parentIterator;
        }
    }

    void ConfigurationDialog::setServers(Servers::Servers *servers)
    {
        this->servers = servers;
    }

    void ConfigurationDialog::readSettings()
    {
       // Read Settings from INI and prefill config dialog items

       ui->checkbox_runOnStartUp->setChecked(settings->get("global/runonstartup", false).toBool());
       ui->checkbox_autostartDaemons->setChecked(settings->get("global/autostartdaemons", false).toBool());
       ui->checkbox_startMinimized->setChecked(settings->get("global/startminimized", false).toBool());

       ui->checkbox_autostart_PHP->setChecked(settings->get("autostart/php", true).toBool());
       ui->checkbox_autostart_Nginx->setChecked(settings->get("autostart/nginx", true).toBool());
       ui->checkbox_autostart_MariaDb->setChecked(settings->get("autostart/mariadb", true).toBool());
       ui->checkbox_autostart_MongoDb->setChecked(settings->get("autostart/mongodb", false).toBool());
       ui->checkbox_autostart_Memcached->setChecked(settings->get("autostart/memcached", false).toBool());
       ui->checkbox_autostart_Postgresql->setChecked(settings->get("autostart/postgresql", false).toBool());
       ui->checkbox_autostart_Redis->setChecked(settings->get("autostart/redis", false).toBool());

       ui->checkbox_clearLogsOnStart->setChecked(settings->get("global/clearlogsonstart", false).toBool());
       ui->checkbox_stopDaemonsOnQuit->setChecked(settings->get("global/stopdaemonsonquit", false).toBool());

       ui->checkbox_onStartAllMinimize->setChecked(settings->get("global/onstartallminimize", false).toBool());
       ui->checkbox_onStartAllOpenWebinterface->setChecked(settings->get("global/onstartallopenwebinterface", false).toBool());

       ui->lineEdit_SelectedEditor->setText(settings->get("global/editor", QVariant(QString("notepad.exe")) ).toString());

       // Configuration > Updater > Self Updater
       ui->checkBox_SelfUpdater_RunOnStartUp->setChecked(settings->get("selfupdater/runonstartup", false).toBool());
       ui->checkBox_SelfUpdater_AutoUpdate->setChecked(settings->get("selfupdater/autoupdate", false).toBool());
       ui->checkBox_SelfUpdater_AutoRestart->setChecked(settings->get("selfupdater/autorestart", false).toBool());

       // Configuration > Components > Memcached
       ui->lineEdit_memcached_tcpport->setText(settings->get("memcached/tcpport", QVariant(QString("11211"))).toString() );
       ui->lineEdit_memcached_udpport->setText(settings->get("memcached/udpport", QVariant(QString("0"))).toString() );
       ui->lineEdit_memcached_threads->setText(settings->get("memcached/threads", QVariant(QString("2"))).toString() );
       ui->lineEdit_memcached_maxconnections->setText(settings->get("memcached/maxconnections", QVariant(QString("2048"))).toString() );
       ui->lineEdit_memcached_maxmemory->setText(settings->get("memcached/maxmemory", QVariant(QString("2048"))).toString() );
    }

    void ConfigurationDialog::writeSettings()
    {
        // we convert the type "boolean" from isChecked() to "int".
        // because i like having a simple 0/1 in the INI file, instead of true/false.

        /**
         * Page "Server Control Panel" - Tab "Configuration"
         */
        settings->set("global/runonstartup",      int(ui->checkbox_runOnStartUp->isChecked()));
        settings->set("global/startminimized",    int(ui->checkbox_startMinimized->isChecked()));
        settings->set("global/autostartdaemons",  int(ui->checkbox_autostartDaemons->isChecked()));

        settings->set("global/clearlogsonstart",  int(ui->checkbox_clearLogsOnStart->isChecked()));
        settings->set("global/stopdaemonsonquit", int(ui->checkbox_stopDaemonsOnQuit->isChecked()));

        settings->set("global/onstartallminimize",         int(ui->checkbox_onStartAllMinimize->isChecked()));
        settings->set("global/onstartallopenwebinterface", int(ui->checkbox_onStartAllOpenWebinterface->isChecked()));

        settings->set("global/editor",            QString(ui->lineEdit_SelectedEditor->text()));

        // Configuration > Updater > Self Updater
        settings->set("selfupdater/runonstartup", int(ui->checkBox_SelfUpdater_RunOnStartUp->isChecked()));
        settings->set("selfupdater/autoupdate",   int(ui->checkBox_SelfUpdater_AutoUpdate->isChecked()));
        settings->set("selfupdater/autorestart",  int(ui->checkBox_SelfUpdater_AutoRestart->isChecked()));

        // Autostart Servers with the Server Control Panel
        settings->set("autostart/nginx",          int(ui->checkbox_autostart_Nginx->isChecked()));
        settings->set("autostart/php",            int(ui->checkbox_autostart_PHP->isChecked()));
        settings->set("autostart/mariadb",        int(ui->checkbox_autostart_MariaDb->isChecked()));
        settings->set("autostart/mongodb",        int(ui->checkbox_autostart_MongoDb->isChecked()));
        settings->set("autostart/memcached",      int(ui->checkbox_autostart_Memcached->isChecked()));
        settings->set("autostart/postgresql",     int(ui->checkbox_autostart_Postgresql->isChecked()));
        settings->set("autostart/redis",          int(ui->checkbox_autostart_Redis->isChecked()));

        // Configuration > Components > PHP

        // Configuration > Components > Nginx

        // Configuration > Components > XDebug

        // Configuration > Components > MariaDB

        // Configuration > Components > MongoDB

        // Configuration > Components > Memcached
        settings->set("memcached/tcpport",        QString(ui->lineEdit_memcached_tcpport->text()));
        settings->set("memcached/udpport",        QString(ui->lineEdit_memcached_udpport->text()));
        settings->set("memcached/threads",        QString(ui->lineEdit_memcached_threads->text()));
        settings->set("memcached/maxconnections", QString(ui->lineEdit_memcached_maxconnections->text()));
        settings->set("memcached/maxmemory",      QString(ui->lineEdit_memcached_maxmemory->text()));

        /**
         * Page "Nginx" - Tab "Upstream"
         */
        saveSettings_Nginx_Upstream();
    }

    void ConfigurationDialog::saveSettings_Nginx_Upstream()
    {
        QJsonObject upstreams;
        upstreams.insert("pools", serialize_toJSON_Nginx_Upstream_PoolsTable(ui->tableWidget_pools));

        // write JSON file
        QJsonDocument jsonDoc;
        jsonDoc.setObject(upstreams);
        File::JSON::save(jsonDoc, "./bin/wpnxm-scp/nginx-upstreams.json");

        // update Nginx upstream config files
        writeNginxUpstreamConfigs(jsonDoc);
    }

    void ConfigurationDialog::writeNginxUpstreamConfigs(QJsonDocument jsonDoc)
    {        
        createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs();

        // build servers string by iterating over all pools

        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools (key)
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // the "value" object has the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            QString poolName        = jsonPool["name"].toString();
            QString method          = jsonPool["method"].toString();
            QJsonObject jsonServers = jsonPool["servers"].toObject();

            // build "servers" block for later insertion into the upstream template string
            QString servers;

            // iterate over all servers
            for (int i = 0; i < jsonServers.count(); ++i) {
                // get values for this server
                QJsonObject s = jsonServers.value(QString::number(i)).toObject();

                // use values to build server string
                QString server = QString("    server %1:%2 weight=%3 max_fails=%4 fail_timeout=%5;\n")
                            .arg(s["address"].toString(),
                                s["port"].toString(), s["weight"].toString(),
                                s["maxfails"].toString(),s["failtimeout"].toString()
                            );

                servers.append(server);
            }

            QString upstream(
                "#\n"
                "# Automatically generated Nginx Upstream definition.\n"
                "# Do not edit manually!\n"
                "\n"
                "upstream "+poolName+" {\n"
                "    "+method+";\n"
                "\n"
                +servers+
                "}\n"
            );

            QString filename("./bin/nginx/conf/upstreams/"+poolName+".conf");

            QFile file(filename);
            if (file.open(QIODevice::ReadWrite | QFile::Truncate)) {
                QTextStream stream(&file);
                stream << upstream << endl;
            }
            file.close();

            qDebug() << "[Nginx Upstream Config] Saved: " << filename;
        }
    }

    void ConfigurationDialog::createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs()
    {
        QDir dir("./bin/nginx/conf/upstreams");

        // create Nginx Conf Upstream Folder If Not Exists
        if(!dir.exists()) {
            dir.mkpath(".");
        }

        // delete old upstream configs
        dir.setNameFilters(QStringList() << "*.conf");
        dir.setFilter(QDir::Files);
        foreach(QString dirFile, dir.entryList()) {
            dir.remove(dirFile);
        }
    }

    QJsonValue ConfigurationDialog::serialize_toJSON_Nginx_Upstream_PoolsTable(QTableWidget *pools)
    {
        QJsonObject jsonPools; // 1..n jsonPool's
        QJsonObject jsonPool;  // pool key/value pairs

        int rows = pools->rowCount();

        for (int i = 0; i < rows; ++i) {

            QString poolName = pools->item(i, NginxAddPoolDialog::Column::Pool)->text();
            QString method = pools->item(i, NginxAddPoolDialog::Column::Method)->text();

            jsonPool.insert("name", poolName);
            jsonPool.insert("method", method);

            // serialize the currently displayed server table
            if(ui->tableWidget_servers->property("servers_of_pool_name") == poolName) {
                qDebug() << "Serializing the currently displayed";
                qDebug() << "Servers Table of Pool" << ui->tableWidget_servers->property("servers_of_pool_name") << poolName;

                jsonPool.insert("servers", serialize_toJSON_Nginx_Upstream_ServerTable(ui->tableWidget_servers));
            } else {
                qDebug() << "Loading table data from file -- Servers of Pool" << poolName;

                // and re-use json data from file for the non-displayed ones
                QJsonObject poolFromJsonFile = getNginxUpstreamPoolByName(poolName);
                jsonPool.insert("servers", poolFromJsonFile["servers"]);
            }

            jsonPools.insert(QString::number(i), QJsonValue(jsonPool));
        }

        return QJsonValue(jsonPools);
    }

    QJsonValue ConfigurationDialog::serialize_toJSON_Nginx_Upstream_ServerTable(QTableWidget *servers)
    {
        QJsonObject jsonServers;     // 1..n jsonServer's
        QJsonObject jsonServer;      // server key/value pairs

        int rows = servers->rowCount();

        for (int i = 0; i < rows; ++i) {

            jsonServer.insert("address",     servers->item(i, 0/*NginxAddServerDialog::Column::Address*/)->text());
            jsonServer.insert("port",        servers->item(i, 1/*NginxAddServerDialog::Column::Port*/)->text());
            jsonServer.insert("weight",      servers->item(i, 2/*NginxAddServerDialog::Column::Weight*/)->text());
            jsonServer.insert("maxfails",    servers->item(i, 3/*NginxAddServerDialog::Column::MaxFails*/)->text());
            jsonServer.insert("failtimeout", servers->item(i, 4/*NginxAddServerDialog::Column::FailTimeout*/)->text());
            jsonServer.insert("phpchildren", servers->item(i, 5/*NginxAddServerDialog::Column::PHPChildren*/)->text());

            jsonServers.insert(QString::number(i), QJsonValue(jsonServer));
        }

        return QJsonValue(jsonServers);
    }

    void ConfigurationDialog::onClickedButtonBoxOk()
    {
        writeSettings();
        toggleRunOnStartup();
    }

    bool ConfigurationDialog::runOnStartUp() const
    {
        return (ui->checkbox_runOnStartUp->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setRunOnStartUp(bool run)
    {
        ui->checkbox_runOnStartUp->setChecked(run);
    }

    bool ConfigurationDialog::runAutostartDaemons() const
    {
        return (ui->checkbox_autostartDaemons->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setAutostartDaemons(bool run)
    {
        ui->checkbox_autostartDaemons->setChecked(run);
    }

    void ConfigurationDialog::toggleAutostartDaemonCheckboxes(bool run)
    {
        // Note: layout doesn't "inject" itself in the parent-child tree, so findChildren() doesn't work

        // left box
        for (int i = 0; i < ui->autostartDaemonsFormLayout->count(); ++i)
        {
          ui->autostartDaemonsFormLayout->itemAt(i)->widget()->setEnabled(run);
        }

        // right box
        for (int i = 0; i < ui->autostartDaemonsFormLayout2->count(); ++i)
        {
          ui->autostartDaemonsFormLayout2->itemAt(i)->widget()->setEnabled(run);
        }
    }

    void ConfigurationDialog::hideAutostartCheckboxesOfNotInstalledServers()
    {
        QStringList installed = this->servers->getListOfServerNamesInstalled();

        QList<QCheckBox *> boxes = ui->tabWidget->findChildren<QCheckBox *>(QRegExp("checkbox_autostart_\\w"));

        for(int i = 0; i < boxes.size(); ++i) {
           QCheckBox *box = boxes.at(i);

           // return last part of "checkbox_autostart_*"
           QString name = box->objectName().section("_", -1).toLower();
           QString labelName = this->servers->getCamelCasedServerName(name) + "Label";
           QLabel *label = ui->tabWidget->findChild<QLabel *>(labelName);

           if(installed.contains(name) == true) {
               qDebug() << "[" + name + "] Autostart Checkbox visible.";
               box->setVisible(true);
               //label->setVisible(true);
           } else {
               qDebug() << "[" + name + "] Autostart Checkbox hidden.";
               box->setVisible(false);
               label->setVisible(false);
           }
        }
    }

    bool ConfigurationDialog::runClearLogsOnStart() const
    {
        return (ui->checkbox_clearLogsOnStart->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setClearLogsOnStart(bool run)
    {
        ui->checkbox_clearLogsOnStart->setChecked(run);
    }

    bool ConfigurationDialog::stopDaemonsOnQuit() const
    {
        return (ui->checkbox_stopDaemonsOnQuit->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setStopDaemonsOnQuit(bool run)
    {
        ui->checkbox_stopDaemonsOnQuit->setChecked(run);
    }

    void ConfigurationDialog::toggleRunOnStartup()
    {
        // Windows %APPDATA% = Roaming ... Programs\Startup
        QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

        if(runOnStartUp()) {
            // Add WPN-XM SCP shortcut to the Windows Autostart folder.
            // In Windows terminology "shortcuts" are "shell links".
            WindowsAPI::CreateShellLink(
                qApp->applicationFilePath(),"","WPN-XM Server Control Panel", // app, args, desc
                qApp->applicationFilePath(),0, // icon path and idx
                qApp->applicationDirPath(), // working dir
                startupDir + "\\WPN-XM Server Control Panel.lnk" // filepath of shortcut
            );
        } else {
            // remove link
            QFile::remove(startupDir+"\\WPN-XM Server Control Panel.lnk");
        }
    }

    void ConfigurationDialog::fileOpen()
    {
        QString file = QFileDialog::getOpenFileName(this, tr("Select Editor..."),
                getenv("PROGRAMFILES"), tr("Executables (*.exe);;All Files (*)"));

        file = QDir::toNativeSeparators(file);

        ui->lineEdit_SelectedEditor->setText(file);
    }

    void ConfigurationDialog::on_toolButton_SelectEditor_clicked()
    {
        ConfigurationDialog::fileOpen();
    }

    void ConfigurationDialog::on_toolButton_ResetEditor_clicked()
    {
        ui->lineEdit_SelectedEditor->setText("notepad.exe");
    }

    void ConfigurationDialog::on_configMenuTreeWidget_clicked(const QModelIndex &index)
    {
        // a click on a menu item switches to the matching page in the stacked widget
        QString menuitem = ui->configMenuTreeWidget->model()->data(index).toString().toLower().remove(" ");
        QWidget *w = ui->stackedWidget->findChild<QWidget *>(menuitem);
        if(w != 0)
            ui->stackedWidget->setCurrentWidget(w);
        else
            qDebug() << "[Config Menu] There is no page " << menuitem << " in the stack widget.";
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddPool_clicked()
    {
        int result;

        NginxAddPoolDialog *dialog = new NginxAddPoolDialog();
        dialog->setWindowTitle("Nginx - Add Pool");

        ui->tableWidget_pools->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_pools->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if(result == QDialog::Accepted) {
            int row = ui->tableWidget_pools->rowCount();
            ui->tableWidget_pools->insertRow(row);
            ui->tableWidget_pools->setItem(row, NginxAddPoolDialog::Column::Pool,   new QTableWidgetItem(dialog->pool()));
            ui->tableWidget_pools->setItem(row, NginxAddPoolDialog::Column::Method, new QTableWidgetItem(dialog->method()));
        }

        delete dialog;
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddServer_clicked()
    {
        int result;

        NginxAddServerDialog *dialog = new NginxAddServerDialog();
        dialog->setWindowTitle("Nginx - Add Server");

        ui->tableWidget_servers->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_servers->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if(result == QDialog::Accepted) {
            int row = ui->tableWidget_servers->rowCount();
            ui->tableWidget_servers->insertRow(row);
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Address,     new QTableWidgetItem(dialog->address()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Port,        new QTableWidgetItem(dialog->port()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Weight,      new QTableWidgetItem(dialog->weight()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::MaxFails,    new QTableWidgetItem(dialog->maxfails()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Timeout,     new QTableWidgetItem(dialog->timeout()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::PHPChildren, new QTableWidgetItem(dialog->phpchildren()));
        }

        delete dialog;
    }

    void ConfigurationDialog::loadNginxUpstreams()
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_pools->setRowCount(0);
        ui->tableWidget_servers->setRowCount(0);

        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // The "value" are the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // --- Fill Pools Table ---

            // insert new row
            int insertRow = ui->tableWidget_pools->rowCount();
            ui->tableWidget_pools->insertRow(insertRow);

            // insert column values
            ui->tableWidget_pools->setItem(insertRow,NginxAddPoolDialog::Column::Pool,
                                           new QTableWidgetItem(jsonPool["name"].toString()));
            ui->tableWidget_pools->setItem(insertRow,NginxAddPoolDialog::Column::Method,
                                           new QTableWidgetItem(jsonPool["method"].toString()));
        }

        // --- Fill Servers Table ---

        // get the first pool, then the "server" key
        QJsonObject jsonPoolFirst = jsonPools.value(QString::number(0)).toObject();

        updateServersTable(jsonPoolFirst);
    }


    void ConfigurationDialog::on_tableWidget_pools_itemSelectionChanged()
    {
        // there is a selection, but its not a row selection
        if(ui->tableWidget_pools->selectionModel()->selectedRows(0).size() <= 0) {
            return;
        }

        // get "pool" from selection
        QString selectedPoolName = ui->tableWidget_pools->selectionModel()->selectedRows().first().data().toString();

        // there is a selection, but the selection is already the currently displayed table view
        if (ui->tableWidget_servers->property("servers_of_pool_name") == selectedPoolName) {
            return;
        }

        // get the pool and update servers table
        QJsonObject jsonPool = getNginxUpstreamPoolByName(selectedPoolName);
        updateServersTable(jsonPool);
    }

    void ConfigurationDialog::updateServersTable(QJsonObject jsonPool)
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_servers->setRowCount(0);

        // set new "pool name" as table property (table view identifier)
        ui->tableWidget_servers->setProperty("servers_of_pool_name", jsonPool["name"].toString());

        // key "servers"
        QJsonObject jsonServers = jsonPool["servers"].toObject();

        for (int i = 0; i < jsonServers.count(); ++i) {

            // values for a "server"
            QJsonObject values = jsonServers.value(QString::number(i)).toObject();

            // insert new row
            int insertRow = ui->tableWidget_servers->rowCount();
            ui->tableWidget_servers->insertRow(insertRow);

            // insert column values
            ui->tableWidget_servers->setItem(
                        insertRow,0/*NginxAddServerDialog::Column::Address*/,     new QTableWidgetItem(values["address"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,1/*NginxAddServerDialog::Column::Port*/,        new QTableWidgetItem(values["port"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,2/*NginxAddServerDialog::Column::Weight*/,      new QTableWidgetItem(values["weight"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,3/*NginxAddServerDialog::Column::MaxFails*/,    new QTableWidgetItem(values["maxfails"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,4/*NginxAddServerDialog::Column::FailTimeout*/, new QTableWidgetItem(values["failtimeout"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,5/*NginxAddServerDialog::Column::PHPChildren*/, new QTableWidgetItem(values["phpchildren"].toString()));
        }
    }

    QJsonObject ConfigurationDialog::getNginxUpstreamPoolByName(QString requestedPoolName)
    {
        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // "value" is key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // key "name" = poolName
            if(jsonPool["name"].toString() == requestedPoolName) {
                return jsonPool;
            }
        }

        return QJsonObject();
    }

}
