//
// Created by RGAA on 2023-09-02.
//

#ifndef SAILFISH_CLIENT_PC_MUTILDISPLAYMODE_H
#define SAILFISH_CLIENT_PC_MUTILDISPLAYMODE_H

#include <QWidget>
#include <QPainter>
#include <functional>

#include "Settings.h"
#include "settings.h"

namespace tc
{

    class MultiDisplayModeWidget : public QWidget {
    public:
        explicit MultiDisplayModeWidget(MultiDisplayMode mode, QWidget* parent = nullptr);
        ~MultiDisplayModeWidget();

        void paintEvent(QPaintEvent *event) override;
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

        void SetSelected(bool selected);
        void SetOnClickCallback(std::function<void()>&& cbk) { click_cbk_ = std::move(cbk); }

    private:

        MultiDisplayMode display_mode_;

        bool enter_ = false;
        bool selected_ = false;
        std::function<void()> click_cbk_;

    };

}

#endif //SAILFISH_CLIENT_PC_MUTILDISPLAYMODE_H
