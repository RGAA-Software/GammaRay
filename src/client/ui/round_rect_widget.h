//
// Created by RGAA on 2023/8/30.
//

#ifndef SAILFISH_SERVER_ROUNDRECTWIDGET_H
#define SAILFISH_SERVER_ROUNDRECTWIDGET_H

#include <QWidget>

namespace tc
{

    class RoundRectWidget : public QWidget {
    public:

        RoundRectWidget(int color, int radius, QWidget *parent = nullptr);

        void paintEvent(QPaintEvent *event) override;

    private:

        int bg_color_ = 0;
        int radius_ = 0;

    };

}

#endif //SAILFISH_SERVER_ROUNDRECTWIDGET_H
