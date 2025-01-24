//
// Created by RGAA on 12/08/2024.
//

#ifndef GAMMARAYPC_KEY_STATE_PANEL_H
#define GAMMARAYPC_KEY_STATE_PANEL_H

#include "base_widget.h"
#include <memory>
#include <QPaintEvent>
#include <QPainter>

namespace tc
{

    class KeyItem : public QWidget {
    public:
        explicit KeyItem(const QString& txt, QWidget* parent = nullptr);
        void paintEvent(QPaintEvent *event) override;
        void UpdateText(const QString& txt);
        void UpdateState(bool pressed);
        void UpdateIndicator(bool pressed);
        bool IsPressed();

    private:
        int color_;
        QString txt_;
        bool pressed_ = false;
        bool indicator_ = false;
    };

    class KeyStatePanel : public BaseWidget {
    public:
        explicit KeyStatePanel(const std::shared_ptr<ClientContext>& ctx, QWidget* parent = nullptr);

    public:
        KeyItem* alt_item_ = nullptr;
        KeyItem* shift_item_ = nullptr;
        KeyItem* control_item_ = nullptr;
        KeyItem* win_item_ = nullptr;
        KeyItem* caps_lock_item_ = nullptr;
        KeyItem* num_lock_item_ = nullptr;

    };

}

#endif //GAMMARAYPC_KEY_STATE_PANEL_H
