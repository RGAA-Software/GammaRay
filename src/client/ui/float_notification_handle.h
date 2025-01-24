//
// Created by RGAA on 6/07/2024.
//

#ifndef GAMMARAYPC_FLOAT_NOTIFICATION_HANDLE_H
#define GAMMARAYPC_FLOAT_NOTIFICATION_HANDLE_H

#include "base_widget.h"

namespace tc
{

    class FloatNotificationHandle : public BaseWidget {
    public:
        explicit FloatNotificationHandle(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void SetPixmap(const QString& path);
        void SetOnClickListener(OnClickListener&& l);
        void SetExpand(bool exp);

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        QPixmap pixmap_;
        bool enter_ = false;
        bool pressed_ = false;
        bool expand_ = false;
        OnClickListener  click_listener_;
    };

}

#endif //GAMMARAYPC_FLOAT_NOTIFICATION_HANDLE_H
