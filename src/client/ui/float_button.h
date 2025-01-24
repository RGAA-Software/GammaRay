//
// Created by RGAA on 2023-08-22.
//

#ifndef SAILFISH_CLIENT_PC_FLOATBUTTON_H
#define SAILFISH_CLIENT_PC_FLOATBUTTON_H

#include <QWidget>
#include <QPixmap>

#include <functional>

namespace tc
{

    class FloatButton : public QWidget {
    public:

        explicit FloatButton(QWidget *parent = nullptr);

        ~FloatButton();

        void paintEvent(QPaintEvent *event) override;

        void enterEvent(QEnterEvent *event) override;

        void leaveEvent(QEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

        void SetOnClickCallback(std::function<void()> &&cbk) {
            click_cbk_ = std::move(cbk);
        }

        void ShowWithAnim();

        void HideWithAnim(std::function<void()> &&finished_task = nullptr);

        void Show();

        void Hide();

    private:

        bool enter_ = false;
        bool pressed_ = false;

        QPixmap expand_pixmap_;

        std::function<void()> click_cbk_;

        float transparency_ = 0.0f;

    };

}

#endif //SAILFISH_CLIENT_PC_FLOATBUTTON_H
