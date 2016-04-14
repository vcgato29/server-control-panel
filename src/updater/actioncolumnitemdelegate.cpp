#include "actioncolumnitemdelegate.h"
#include "updaterdialog.h"

#include <QDebug>

namespace Updater
{

ActionColumnItemDelegate::ActionColumnItemDelegate(QObject *parent)
{
    Q_UNUSED(parent);

    btn = new QPushButton;
    btn->setVisible(false);
    setPushButtonStyle(btn);

    bar = new QProgressBar;
    bar->setVisible(false);
    setProgressBarStyle(bar);
}

ActionColumnItemDelegate::~ActionColumnItemDelegate()
{
    delete btn;
    delete bar;
}

void ActionColumnItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{    
    if (index.data(DownloadPushButtonRole).toString() != "hide")
    {
        //qDebug() << "DownloadButton";
        drawDownloadPushButton(painter,option,index);
    }
    else if(index.data(DownloadProgressBarRole).toString() != "hide" && index.data(DownloadPushButtonRole).toString() == "hide")
    {
        //qDebug() << "ProgressBar";
        drawDownloadProgressBar(painter,option,index);
    }
    else if(index.data(InstallPushButtonRole).toString() != "hide" && index.data(DownloadProgressBarRole).toString() == "hide")
    {
        //qDebug() << "InstallButton";
        drawInstallPushButton(painter,option,index);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

void ActionColumnItemDelegate::drawDownloadPushButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton opt;
    opt.initFrom(btn);
    opt.rect = option.rect.adjusted(2,2,-2,-2);
    opt.text = "Download";
    opt.state = QStyle::State_Enabled;
    opt.features |= QStyleOptionButton::DefaultButton;

    // change style of button, when clicked. based on boolean value in the model. see setData() in editorEvent().
    opt.state |= (index.data(DownloadPushButtonRole).toString() == "show-clicked") ?
                 QStyle::State_Sunken : QStyle::State_Raised;

    // hover on MouseOver
    if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect.adjusted(+1,+1,-1,-1), Qt::darkGray);
    }

    btn->style()->drawControl(QStyle::CE_PushButton,&opt,painter,btn);
}

void ActionColumnItemDelegate::drawInstallPushButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton opt;
    opt.initFrom(btn);
    opt.rect = option.rect.adjusted(2,2,-2,-2);
    opt.text = "Install";
    opt.state = QStyle::State_Enabled;
    opt.features |= QStyleOptionButton::DefaultButton;

    // change style of button, when clicked. based on boolean value in the model. see setData() in editorEvent().
    opt.state |= (index.data(InstallPushButtonRole).toString() == "show-clicked") ?
                 QStyle::State_Sunken : QStyle::State_Raised;

    // hover on MouseOver
    if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect.adjusted(+1,+1,-1,-1), Qt::darkGray);
    }

    btn->style()->drawControl(QStyle::CE_PushButton,&opt,painter,btn);
}

void ActionColumnItemDelegate::drawDownloadProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionProgressBarV2 opt;
    opt.initFrom(bar);
    opt.rect = option.rect;
    opt.rect.adjust(5,5,-5,-5);
    opt.textVisible = true;
    opt.state = QStyle::State_Enabled | QStyle::State_Active;

    // progress    
    opt.minimum = 0;
    opt.maximum = 100;
    opt.progress = index.model()->data(index, DownloadProgressBarRole).toInt();
    opt.text = QString("%1%").arg(opt.progress);

    bar->style()->drawControl(QStyle::CE_ProgressBar,&opt,painter,bar);
}

void ActionColumnItemDelegate::setPushButtonStyle(QPushButton *btn) const
{
    QString style = "QPushButton { border: 1px solid #CCC; background-color: #EEE; text-align: center;}";
        style += "QPushButton:pressed {background-color: #d1d7da; border-width: 1px; border-color: #666;}";
        style += "QPushButton:hover:!pressed { border: 1px solid red; }";
        style += "QPushButton:hover { background-color:yellow; }";

    btn->setStyleSheet(style);
}

void ActionColumnItemDelegate::setProgressBarStyle(QProgressBar *bar) const
{    
    QString style = "QProgressBar {border: 1px solid #CCC; background-color: #EEE; text-align: center;}";
        style += "QProgressBar::chunk {background-color: #d1d7da; margin: 0.5px;}";

    bar->setStyleSheet(style);

    //bar->setStyleSheet(QString("QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 grey, stop: 1 grey);}")+QString("QProgressBar::horizontal {border: 1px solid gray; border-radius: 3px; background: yellow; padding: 0px; text-align: left; margin-right: 4ex;}"));
}

/**
 * Qt makes it really hard to add widgets and events to tableviews
 * The cell doesn't contain an editor, but this allows to bind to events...
 * so its a hack to intercept the mouseclick on the table cell.
 */
bool ActionColumnItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,const QStyleOptionViewItem &option,const QModelIndex &index)
{
    Q_UNUSED(option);

    if(event->type() != QEvent::MouseButtonRelease && event->type() != QEvent::MouseButtonPress ) {
       return false;
    }

    if (event->type() == QEvent::MouseButtonPress) {

        if(index.data(DownloadPushButtonRole).toString() != "hide") {
            qDebug() << "Action Cell in Row " << index.row() << "has Download Role and Download Button clicked..";
            //QModelIndex urlIndex = index.model()->index(index.row(), UpdaterDialog::Columns::DownloadURL);
            //qDebug() << "DownloadURL"<< index.model()->data(urlIndex).toString();
            model->setData(index, "show-clicked", DownloadPushButtonRole);
            emit downloadButtonClicked(index);
            model->setData(index, 0, DownloadProgressBarRole);
            emit model->dataChanged(index, index);
            /*qDebug() << index.data(DownloadPushButtonRole);
            qDebug() << index.data(DownloadProgressBarRole);
            qDebug() << index.data(InstallPushButtonRole);*/
            return true;
        }
        if(index.data(InstallPushButtonRole).toString() != "hide") {
            qDebug() << "Action Cell in Row " << index.row() << "has Install Role and Install Button clicked...";
            model->setData(index, 1, InstallPushButtonRole);
            emit installButtonClicked(index);
            return true;
        }
    }

    if(event->type() == QEvent::MouseButtonRelease) {

        if(index.data(DownloadPushButtonRole).toString() == "show-clicked") {
            model->setData(index, "hide", DownloadPushButtonRole);
            return true;
        }
        if(index.data(InstallPushButtonRole).toString() == "shown-clicked") {
            model->setData(index, "hide", InstallPushButtonRole);
            return true;
        }
    }

    return false;
}

}
