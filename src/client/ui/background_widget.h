//
// Created by RGAA on 6/07/2024.
//

#ifndef GAMMARAYPC_BACKGROUND_WIDGET_H
#define GAMMARAYPC_BACKGROUND_WIDGET_H

#include "base_widget.h"

namespace tc
{

    class BackgroundWidget : public BaseWidget {
    public:
        explicit BackgroundWidget(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

        void SetColors(int normal_color, int enter_color, int pressed_color);
        void SetRadius(int radius);

    private:
        bool enter_ = false;
        bool pressed_ = false;
        int normal_color_ = 0xffffff;
        int enter_color_ = 0xf0f0f0;
        int pressed_color_ = 0xd9d9d9;
        int radius_ = 0;
    };

}

#endif //GAMMARAYPC_BACKGROUND_WIDGET_H
