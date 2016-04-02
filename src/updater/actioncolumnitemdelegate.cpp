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
    if (index.data(IsDownloadPushButtonRole).canConvert<bool>())
    {
        drawDownloadPushButton(painter,option,index);
    }
    else if(index.data(IsDownloadProgressBarRole).canConvert<bool>())
    {
        drawDownloadProgressBar(painter,option,index);
    }
    else if(index.data(IsInstallPushButtonRole).canConvert<bool>())
    {
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
    bool buttonClicked = index.data(IsDownloadPushButtonRole).value<bool>();
    opt.state |= buttonClicked ? QStyle::State_Sunken : QStyle::State_Raised;

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
    bool buttonClicked = index.data(IsInstallPushButtonRole).value<bool>();
    opt.state |= buttonClicked ? QStyle::State_Sunken : QStyle::State_Raised;

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
    opt.progress = index.model()->data(index, IsDownloadProgressBarRole).toInt();
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
}

/**
 * Qt makes it really hard to add widgets and events to tableviews
 * The cell doesn't contain an editor, but this allows to bind to events...
 * so its a hack to intercept the mouseclick on the table cell.
 */
bool ActionColumnItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,const QStyleOptionViewItem &option,const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {

        if(index.data(IsDownloadPushButtonRole).canConvert<bool>()) {
            qDebug() << "Action Cell in Row " << index.row() << "has Download Role and Download Button clicked..";
            qDebug() << "DownloadURL"<< index.model()->data(index.model()->index(index.row(), UpdaterDialog::Columns::DownloadURL)).toString();
            model->setData(index, true, IsDownloadPushButtonRole);
            emit downloadButtonClicked(index);
            return true;
        }

        if(index.data(IsInstallPushButtonRole).canConvert<bool>()) {
            qDebug() << "Action Cell in Row " << index.row() << "has Install Role and Install Button clicked...";
            model->setData(index, true, IsInstallPushButtonRole);
            emit installButtonClicked(index);
            return true;
        }
    }

    if(event->type() == QEvent::MouseButtonRelease) {
        if(index.data(IsDownloadPushButtonRole).canConvert<bool>()) {
            model->setData(index, false, IsDownloadPushButtonRole);
        }
        if(index.data(IsInstallPushButtonRole).canConvert<bool>()) {
            model->setData(index, false, IsInstallPushButtonRole);
        }
    }

    return 0;
}

}
