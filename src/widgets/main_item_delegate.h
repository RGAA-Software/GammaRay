#pragma once

#include <QStyledItemDelegate>

namespace tc
{
    class MainItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

    public:
        explicit MainItemDelegate(QObject *pParent);

        ~MainItemDelegate() override;

        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
    };
}