//
// Created by RGAA on 3/07/2024.
//

#ifndef GAMMARAYPC_FLOAT_CONTROLLER_H
#define GAMMARAYPC_FLOAT_CONTROLLER_H

#include "base_widget.h"

namespace tc
{
    class ClientContext;

    class FloatController : public BaseWidget {
    public:
        explicit FloatController(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;

        void SetOnClickListener(std::function<void()>&& l) {
            click_listener_ = l;
        }
        void SetOnMoveListener(std::function<void()>&& l) {
            move_listener_ = l;
        }

        bool HasMoved() const;

    private:
        QPixmap pixmap_;
        bool enter_ = false;
        bool pressed_ = false;
        QPoint drag_position_;
        std::function<void()> click_listener_;
        std::function<void()> move_listener_;
        bool has_moved_ = false;
    };
}

#endif //GAMMARAYPC_FLOAT_CONTROLLER_H
