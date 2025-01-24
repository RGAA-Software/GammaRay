//
// Created by RGAA on 6/07/2024.
//

#ifndef GAMMARAYPC_FLOAT_ICON_H
#define GAMMARAYPC_FLOAT_ICON_H

#include "base_widget.h"

namespace tc
{

    class FloatIcon : public BaseWidget {
    public:
        explicit FloatIcon(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void SetIcons(const QString& normal_path, const QString& selected_path);
        void SwitchToNormalState();
        void SwitchToSelectedState();
    private:
        bool is_selected_ = false;
        QPixmap normal_pixmap_;
        QPixmap selected_pixmap_;
        bool enter_ = false;
        bool pressed_ = false;
    };

}

#endif //GAMMARAYPC_FLOAT_ICON_H
