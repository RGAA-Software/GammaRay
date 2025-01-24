//
// Created by RGAA on 2023/8/19.
//

#ifndef SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H
#define SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include <QPaintEvent>
#include <QPixmap>
#include "db/stream_item.h"

namespace tc
{

    class StreamItemWidget : public QWidget {
    public:

        explicit StreamItemWidget(const StreamItem& item,  int bg_color, QWidget* parent = nullptr);
        ~StreamItemWidget() override;

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;

    private:
        StreamItem item_;
        int bg_color_ = 0;
        bool enter_ = false;

        QPixmap icon_;
        QBitmap mask_;
        int radius_ = 13;

    };

}

#endif //SAILFISH_CLIENT_PC_STREAMITEMWIDGET_H
