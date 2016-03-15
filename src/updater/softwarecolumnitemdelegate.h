#ifndef SOFTWARECOLUMNITEMDELEGATE_H
#define SOFTWARECOLUMNITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QLabel>
#include <QPainter>
#include <QTextDocument>
#include <QDesktopServices>
#include <QMouseEvent>

namespace Updater
{

    class SoftwareColumnItemDelegate : public QStyledItemDelegate
    {
         Q_OBJECT
    public:
        explicit SoftwareColumnItemDelegate(QObject *parent = 0);
        ~SoftwareColumnItemDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    private:
        QLabel *label;

    public slots:
        void onClickedOpenURL(const QModelIndex &index);
    };

}

#endif // SOFTWARECOLUMNITEMDELEGATE_H
