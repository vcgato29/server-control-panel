#include "updaterdialog.h"
#include "ui_updaterdialog.h"

#include <QDebug>
#include <QNetworkProxy>

namespace Updater
{

    UpdaterDialog::UpdaterDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Updater::Ui::UpdaterDialog)
    {
        ui->setupUi(this);

        // disable resizing
        setSizeGripEnabled(false);
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        //downloadManager   = new Downloader::DownloadManager();
        softwareRegistry  = new SoftwareRegistry::Manager();

        setJsonToServerStackTable(softwareRegistry->getStackSoftwareRegistry());
    }

    UpdaterDialog::~UpdaterDialog()
    {
        delete ui;
    }

    void UpdaterDialog::setJsonToServerStackTable(QJsonObject json)
    {
        /**
         * The Model
         */
        model = new QStandardItemModel(0,4,this);

        /**
         * @brief jsonObject has the following item structure:
         *
         *   {...},
         *   {
         *     "wpnxmscp": {
         *       "latest": {
         *           "url": "https://github.com/WPN-XM/../file.zip",
         *           "version": "0.8.4"
         *       },
         *       "name": "WPN-XM Server Control Panel x86",
         *       "website": "http://wpn-xm.org/"
         *   },
         *   {...}
         */

        /**
         * populate Model with JSON data
         */
        for (QJsonObject:: Iterator iter = json.begin(); iter != json.end(); ++iter)
        {
            QList<QStandardItem*> rowItems;

            // The "key" is the registry name of the software component.
            //QString *registrySoftwareName = QString(iter.key());

            // The "value" is the data of the software component
            QJsonObject software = iter.value().toObject();

            // Map for Latest URL and Version
            QVariantMap latestVersionMap = software["latest"].toObject().toVariantMap();
            //qDebug() << latestVersion["url"].toString() << latestVersion["version"].toString();

            // Table Columns

            // Software Name
            QStandardItem *softwareName = new QStandardItem( "      " + software["name"].toString() );
            rowItems.append(softwareName);

            // Website Link
            QStandardItem *websiteURL = new QStandardItem( software["website"].toString() );
            rowItems.append(websiteURL);

            // Installed Version (= Your current version)
            QString installedVersionString = "1.2.3"; // @todo detect currently installed versions
            QStandardItem *installedVersion = new QStandardItem( installedVersionString );
            installedVersion->setTextAlignment(Qt::AlignCenter);
            rowItems.append(installedVersion);

            // Latest Version
            QStandardItem *latestVersion = new QStandardItem( latestVersionMap["version"].toString() );
            latestVersion->setTextAlignment(Qt::AlignCenter);
            rowItems.append(latestVersion);

            // Download URL for Latest Version
            QStandardItem *latestVersionURL = new QStandardItem( latestVersionMap["url"].toString() );
            rowItems.append(latestVersionURL);

            // Action

            /**
             * How to render widgets in a table cell when using QTableView?
             *
             * Setting a QPushButton directly to the model is not possible at this point.
             * But its possible to set the model to the tableView and then use rowCount()
             * and draw the buttons using setIndexWidet() in a second iteration.
             *
             * Boils down to: When using a
             * - tableWidget: ugly sorting, no-model, but full cell control with all widgets
             * - tableView: either nice sorting, less cell control, no widgets, but a proper model
             *
             * ----------
             * Finally, i came up with using an ItemDelegate to render the widgets in the action column.
             * The rendering is done by the delegate's paint function.
             * To render multiple widgets (Download Button, Download ProgressBar, Install Button)
             * i've added custom UserRoles (Qt::ItemDataRole) (DownloadButtonRole, etc.).
             * These UserRoles are set as data to the model and tell the view what to render.
             * This allows to easily update the model and get the matching action buttons.
             *
             * Data:
             * - Buttons: hide || show || show-clicked
             * - ProgressBar: hide || <int> progress
             */
            QStandardItem *action = new QStandardItem("ActionCell");
            action->setData("show", ActionColumnItemDelegate::DownloadPushButtonRole);
            action->setData("hide", ActionColumnItemDelegate::DownloadProgressBarRole);
            action->setData("hide", ActionColumnItemDelegate::InstallPushButtonRole);
            rowItems.append(action);

            model->appendRow(rowItems);            
        }

        /**
         * Set Header Labels for Table
         */
        QStringList headerLabels;
        // Table               1              (hidden)           2               3             (hidden)        4
        headerLabels<<"Software Component"<<"WebsiteURL"<<"Your Version"<<"Latest Version"<<"DownloadURL"<<"Actions";
        model->setHorizontalHeaderLabels(headerLabels);

        /**
         * Setup SortingProxy for the Model
         */
        myFilterProxyModel = new QSortFilterProxyModel(this);
        myFilterProxyModel->setSourceModel(model);
        // sorting is case-insensitive
        myFilterProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
        // filtering is case-insensitive and using the software name column
        myFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        myFilterProxyModel->setFilterKeyColumn(Columns::SoftwareComponent);

        /**
         * Set "sort/filter-proxied" Model to View
         */
        ui->tableView_1->setModel(myFilterProxyModel);

        /**
         * Set Item Delegates for "SoftwareComponent" and "Action" Columns
         */
        softwareDelegate = new Updater::SoftwareColumnItemDelegate;
        ui->tableView_1->setItemDelegateForColumn(Columns::SoftwareComponent, softwareDelegate);

        actionDelegate = new Updater::ActionColumnItemDelegate;
        ui->tableView_1->setItemDelegateForColumn(Columns::Action, actionDelegate);

        connect(actionDelegate, SIGNAL(downloadButtonClicked(QModelIndex)), this, SLOT(doDownload(QModelIndex)));
        connect(actionDelegate, SIGNAL(installButtonClicked(QModelIndex)), this, SLOT(doInstall(QModelIndex)));

        /**
         * Configure view
         */
        // enable mouse tracking to be able to bind the mouseover/hover event
        ui->tableView_1->setMouseTracking(true);
        // disable resizing of the columns
        ui->tableView_1->horizontalHeader()->setSectionResizeMode(Columns::SoftwareComponent, QHeaderView::Stretch);
        ui->tableView_1->horizontalHeader()->setSectionResizeMode(Columns::LatestVersion, QHeaderView::Fixed);
        ui->tableView_1->horizontalHeader()->setSectionResizeMode(Columns::YourVersion, QHeaderView::Fixed);
        // hide columns (1 and 3; both URLs)
        ui->tableView_1->setColumnHidden(Columns::WebsiteURL, true);
        ui->tableView_1->setColumnHidden(Columns::DownloadURL, true);
        // settings
        ui->tableView_1->setAutoScroll(true);
        ui->tableView_1->setAlternatingRowColors(true);
        // sort
        ui->tableView_1->setSortingEnabled(true);
        ui->tableView_1->sortByColumn(Columns::SoftwareComponent, Qt::AscendingOrder);
        // sizes
        ui->tableView_1->verticalHeader()->setDefaultSectionSize(28);
        ui->tableView_1->horizontalHeader()->setDefaultSectionSize(28);
        //ui->tableView_1->resizeRowsToContents();
        ui->tableView_1->resizeColumnsToContents();
        ui->tableView_1->setColumnWidth(Columns::SoftwareComponent, 150);
        ui->tableView_1->setColumnWidth(Columns::LatestVersion, 80);
        ui->tableView_1->setColumnWidth(Columns::YourVersion, 80);
        ui->tableView_1->setColumnWidth(Columns::Action, 200);
    }

    void UpdaterDialog::doDownload(const QModelIndex &index)
    {
        QUrl downloadURL = getDownloadUrl(index);
        if (!validateURL(downloadURL)) {
            return;
        }
        qDebug() << "doDownload()" << downloadURL;

        //QString targetFolder = QDir::toNativeSeparators(QApplication::applicationDirPath() + "/data/downloads");
        //QString targetName   = "test.zip";

        /**
         * Download Request
         */

        // setup Proxy
        // use system proxy (by default) "--use-proxy=on"
        //QNetworkProxyFactory::setUseSystemConfiguration(true);

        // setup Network Request
        QNetworkRequest request(downloadURL);
        QString appVersion(qApp->applicationName()+qApp->applicationVersion());
        request.setRawHeader("User-Agent", QByteArray(appVersion.toStdString().c_str()));
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

        downloadManager.setQueueMode(Downloader::DownloadManager::Parallel);
        downloadManager.get(request);

        qDebug() << "FilesDownloadedCounter" << downloadManager.FilesDownloadedCounter;
        qDebug() << "FilesToDownloadCounter" << downloadManager.FilesToDownloadCounter;

        // fetch the bubbled up download progress
        connect(&downloadManager, SIGNAL(signalProgress(QMap<QString, QVariant>)),
                this, SLOT(updateDownloadProgress(QMap<QString, QVariant>)));

        currentIndexRow = index.row();

        QMetaObject::invokeMethod(&downloadManager, "checkForAllDone", Qt::QueuedConnection);
    }

    QUrl UpdaterDialog::getDownloadUrl(const QModelIndex &index)
    {
        QModelIndex indexURL = index.model()->index(index.row(), Columns::DownloadURL, QModelIndex());
        return QUrl(ui->tableView_1->model()->data(indexURL).toString());
    }

    void UpdaterDialog::doInstall(const QModelIndex &index)
    {
        Q_UNUSED(index);
    }

    void UpdaterDialog::updateDownloadProgress(QMap<QString, QVariant> progress)
    {
        qDebug() << "UpdaterDialog::updateDownloadProgress";

        QModelIndex actionIndex = ui->tableView_1->model()->index(currentIndexRow, Columns::Action);

        // hide DownloadButton
        if(actionIndex.data(ActionColumnItemDelegate::DownloadPushButtonRole).toString() != "hide") {
            ui->tableView_1->model()->setData(actionIndex, "hide", ActionColumnItemDelegate::DownloadPushButtonRole);
        }

        // update the "progress" data in the model
        QVariant progressVariant = QVariant::fromValue(progress);
        ui->tableView_1->model()->setData(actionIndex, progressVariant, ActionColumnItemDelegate::DownloadProgressBarRole);

        // "hide" progressBar when we reach 100% and "show" Install Button
        if(actionIndex.data(ActionColumnItemDelegate::DownloadProgressBarRole).toMap()["percentage"] == "100 %") {
            ui->tableView_1->model()->setData(actionIndex, "hide", ActionColumnItemDelegate::DownloadProgressBarRole);
            ui->tableView_1->model()->setData(actionIndex, "show", ActionColumnItemDelegate::InstallPushButtonRole);
        }

        /*qDebug() << "Download Button Data" << actionIndex.data(ActionColumnItemDelegate::DownloadPushButtonRole);
        qDebug() << "ProgressBar Data" << actionIndex.data(ActionColumnItemDelegate::DownloadProgressBarRole);
        qDebug() << "Install Button Data" << actionIndex.data(ActionColumnItemDelegate::InstallPushButtonRole);*/

        ui->tableView_1->model()->dataChanged(actionIndex, actionIndex);
        ui->tableView_1->repaint();
    }

    void UpdaterDialog::downloadsFinished()
    {
        qDebug() << "UpdaterDialog::downloadsFinished \n Triggering post-download tasks";
    }

    void UpdaterDialog::on_searchLineEdit_textChanged(const QString &arg1)
    {
       //myFilterProxyModel->setFilterRegExp(QRegExp(arg1, Qt::CaseInsensitive, QRegExp::FixedString));
       myFilterProxyModel->setFilterFixedString(arg1);
    }

    bool UpdaterDialog::validateURL(QUrl url)
    {
        if (!url.isValid() || url.isEmpty() || url.host().isEmpty()) {
            qDebug() << "URL invalid:" << url;
            return false;
        }
        return true;
    }

}
