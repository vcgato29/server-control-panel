#ifndef ACTIONCOLUMNITEMDELEGATE_H
#define ACTIONCOLUMNITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPushButton>
#include <QProgressBar>

#include <QPainter>
#include <QStylePainter>
#include <QApplication>

namespace Updater
{
    class ActionColumnItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        explicit ActionColumnItemDelegate(QObject *parent = 0);
        ~ActionColumnItemDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        static const int IsDownloadPushButtonRole  = Qt::UserRole + 101;
        static const int IsDownloadProgressBarRole = Qt::UserRole + 102;
        static const int IsInstallPushButtonRole   = Qt::UserRole + 103;

        bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    signals:
        void downloadButtonClicked(const QModelIndex &index);
        void installButtonClicked(const QModelIndex &index);

    private:
        void drawDownloadPushButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const ;
        void drawDownloadProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void drawInstallPushButton(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        QPushButton *btn;
        QProgressBar *bar;

        void setPushButtonStyle(QPushButton *btn) const;
        void setProgressBarStyle(QProgressBar *bar) const;
    };

}

#endif // ACTIONCOLUMNITEMDELEGATE_H
