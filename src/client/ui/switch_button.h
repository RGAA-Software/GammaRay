//
// Created by RGAA on 2023-09-01.
//

#ifndef SAILFISH_CLIENT_PC_SWITCHBUTTON_H
#define SAILFISH_CLIENT_PC_SWITCHBUTTON_H

#include <QWidget>
#include <QPainter>
#include "switch_button.h"
#include "app_color_theme.h"

namespace tc
{

    class SwitchButton : public QWidget {
    public:
        explicit SwitchButton(QWidget *parent = nullptr);
        ~SwitchButton() override;

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void SetStatus(bool enabled);
        void SetClickCallback(std::function<void(bool)> &&cbk) { click_cbk_ = cbk; }

    private:
        void ExecAnimation(bool selected);

    private:
        int border_color = 0x999999;
        int border_width = 1;
        int normal_bg_color_ = 0xffffff;
        int selected_bg_color_ = 0xffffff;
        int normal_thumb_color = 0xaaaaaa;
        int selected_thumb_color = AppColorTheme::kAppMenuItemBgHoverColor;

        bool selected = false;

        int left_point_ = 0;
        int right_point_ = 0;
        int thumb_point_ = 0;

        std::function<void(bool)> click_cbk_;

        bool need_repair_ = false;

    };

}

#endif //SAILFISH_CLIENT_PC_SWITCHBUTTON_H
