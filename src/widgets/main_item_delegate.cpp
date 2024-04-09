#include "main_item_delegate.h"
#include <QPainter>

namespace tc
{
    MainItemDelegate::MainItemDelegate(QObject *pParent)
            : QStyledItemDelegate(pParent) {
    }

    MainItemDelegate::~MainItemDelegate() = default;

    void MainItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const {
        editor->setGeometry(option.rect);
    }
}